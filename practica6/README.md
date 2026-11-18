# Práctica 6 — Diez operaciones con OpenMP

Programa en C con un menú de consola que resuelve las diez operaciones del
enunciado de la práctica. Cada operación está implementada en su propia función
y usa OpenMP para repartir el trabajo entre hilos.

> **Estado:** las diez operaciones están implementadas y probadas.
> Este documento funciona a la vez como informe de la práctica y como material
> de estudio del código: para cada operación se explica **qué se pedía**,
> **cómo se resolvió**, **por qué la versión paralela es correcta** y **qué
> conviene tener claro para defenderla**.

## Índice

1. [Enunciado original](#enunciado-original)
2. [Archivos del proyecto](#archivos-del-proyecto)
3. [Requisitos](#requisitos)
4. [Compilación y ejecución](#compilación-y-ejecución)
5. [Estructura del programa](#estructura-del-programa)
6. [Mapa de lectura del código](#mapa-de-lectura-del-código)
7. [Las diez operaciones](#las-diez-operaciones)
   - [Operación 1 — Espacios en blanco en una cadena](#operación-1--espacios-en-blanco-en-una-cadena)
   - [Operación 2 — Palabras y caracteres de un archivo](#operación-2--palabras-y-caracteres-de-un-archivo)
   - [Operación 3 — Suma de matrices cuadradas](#operación-3--suma-de-matrices-cuadradas)
   - [Operación 4 — ¿Dos cadenas son iguales?](#operación-4--dos-cadenas-son-iguales)
   - [Operación 5 — Ordenar enteros](#operación-5--ordenar-enteros)
   - [Operación 6 — ¿La palabra está en el texto?](#operación-6--la-palabra-está-en-el-texto)
   - [Operación 7 — Intersección de conjuntos](#operación-7--intersección-de-conjuntos)
   - [Operación 8 — Raíces reales de una cuadrática](#operación-8--raíces-reales-de-una-cuadrática)
   - [Operación 9 — Apariciones de una palabra](#operación-9--apariciones-de-una-palabra)
   - [Operación 10 — Sumatoria de n enteros](#operación-10--sumatoria-de-n-enteros)
8. [Resumen de directivas OpenMP usadas](#resumen-de-directivas-openmp-usadas)
9. [Glosario OpenMP](#glosario-openmp)
10. [Complejidad y Ley de Amdahl](#complejidad-y-ley-de-amdahl)
11. [Cómo medir el rendimiento](#cómo-medir-el-rendimiento)
12. [Trampas y errores comunes](#trampas-y-errores-comunes)
13. [Pruebas realizadas](#pruebas-realizadas)
14. [Preguntas de repaso](#preguntas-de-repaso)
15. [Checklist: sé explicar esto](#checklist-sé-explicar-esto)
16. [Limitaciones y posibles mejoras](#limitaciones-y-posibles-mejoras)

---

## Enunciado original

> En equipo, escribir un programa, utilizando OpenMP en C/C++, que permita
> realizar las siguientes operaciones:
>
> 1. Contar el número de espacios en blanco que tiene una cadena.
> 2. Contar el número de palabras (caracteres consecutivos sin espacio) y el
>    número de caracteres que tiene un archivo de texto.
> 3. Sumar dos matrices cuadradas de números enteros.
> 4. Revisar si dos cadenas son iguales.
> 5. Ordenar un conjunto de números enteros.
> 6. Revisar si una palabra se encuentra en un texto.
> 7. Obtener la intersección de dos conjuntos de enteros.
> 8. Calcular las raíces reales de una ecuación cuadrática.
> 9. Revisar cuántas veces aparece una palabra en un texto.
> 10. Calcular la sumatoria de n números enteros.

Aunque varias operaciones son de grano muy fino (paralelizarlas no las hace más
rápidas), el objetivo de la práctica es **aplicar y entender los distintos
mecanismos de OpenMP**: reparto de iteraciones (`for`), reducciones,
descomposición funcional (`sections`) y sincronización (`critical`).

---

## Archivos del proyecto

| Archivo | Descripción |
|---|---|
| `programa.c` | Todo el programa: menú, utilidades, las diez operaciones y sus directivas OpenMP. |
| `Makefile` | Compilación con `gcc -std=c11`, OpenMP y `-lm`. |
| `README.md` | Este informe/guía de estudio. |

---

## Requisitos

- `gcc` con soporte de OpenMP (`libgomp`). Comprobar con:
  ```bash
  echo | gcc -fopenmp -dM -E - | grep _OPENMP
  # Debe imprimir una línea con #define _OPENMP <versión>;
  # con gcc 16 es: #define _OPENMP 202111
  ```
- Biblioteca matemática `-lm` (se usa `sqrt` en la operación 8).
- `make` (opcional, hay un `Makefile`).

---

## Compilación y ejecución

```bash
cd practica6
make          # gcc -std=c11 -O2 -Wall -Wextra -fopenmp programa.c -o programa -lm
./programa    # o: make run
```

Compilación directa, sin `Makefile`:

```bash
gcc -std=c11 -O2 -Wall -Wextra -fopenmp programa.c -o programa -lm
```

Limpiar el ejecutable:

```bash
make clean
```

Control del número de hilos (por defecto OpenMP usa todos los núcleos lógicos):

```bash
OMP_NUM_THREADS=4 ./programa
```

Al arrancar, el programa entra en un bucle: muestra el menú, lee una opción,
ejecuta la operación y vuelve al menú. La opción `0` termina. Cada operación se
encapsula en una función `static void opN_*` y se invoca desde un `switch`.

---

## Estructura del programa

### `main` y el menú

`main` (líneas 140–172) es un bucle infinito con este esqueleto:

```c
for (;;) {
    imprimir_menu();
    if (scanf("%d", &opcion) != 1) { /* entrada inválida */ }
    limpiar_entrada();              /* descarta el '\n' restante */

    if (opcion == 0) break;

    switch (opcion) {
        case 1:  op1_espacios();            break;
        case 2:  op2_palabras_caracteres(); break;
        /* ... hasta case 10 ... */
        default: printf("Opcion no valida.\n"); break;
    }
}
```

Detalles importantes:

- `limpiar_entrada()` consume el salto de línea que deja `scanf`, para que las
  lecturas posteriores con `fgetc`/`getchar` no lean una línea vacía.
- Cada `case` delega en una función; añadir o quitar operaciones no toca el
  flujo del menú.
- La opción `0` rompe el bucle y termina el programa.

### Utilidades compartidas

| Función | Líneas | Para qué sirve |
|---|---|---|
| `imprimir_menu()` | 28–43 | Imprime el menú de las diez operaciones. |
| `limpiar_entrada()` | 45–48 | Descarta caracteres hasta el fin de línea o `EOF`. |
| `leer_linea_dinamica(FILE *)` | 50–77 | Lee una línea completa de longitud arbitraria; redimensiona el búfer con `realloc`. |
| `leer_archivo_completo(ruta, &n)` | 79–113 | Carga un archivo entero en memoria (modo `"rb"`) y devuelve el tamaño en bytes. |
| `normalizar_palabra(inicio, len)` | 115–137 | Quita signos de puntuación al inicio y al final y pasa todo a minúsculas. |

**Por qué existen estas utilidades.** El enunciado no limita el tamaño de las
cadenas ni de los archivos. Usar `scanf("%s")` o un `char[100]` habría truncado
la entrada o desbordado el búfer. Estas funciones permiten trabajar con
cadenas y archivos de cualquier longitud, que es lo que hace interesante
paralelizar el recorrido.

### Cómo se leen los datos

- Las cadenas de texto se leen con `leer_linea_dinamica`, por lo que pueden
  contener espacios y tener cualquier longitud (sin el `\n` final).
- Los enteros se leen con `scanf` y se validan (`n <= 0`, fallo de lectura).
- Las operaciones 3, 5, 7 y 10 permiten **escribir los datos por teclado** o
  **generarlos aleatoriamente**, para poder probar con tamaños grandes sin
  teclear miles de números.
- Los rangos aleatorios son `[-100, 100]` (ops. 3, 5 y 10) y `[-25, 25]`
  (op. 7).

---

## Mapa de lectura del código

| # | Función | Líneas | Directiva principal |
|---|---|---|---|
| 1 | `op1_espacios` | 174–195 | `parallel for reduction(+:espacios)` |
| 2 | `op2_palabras_caracteres` | 197–238 | `parallel for reduction(+:palabras, caracteres)` |
| 3 | `op3_sumar_matrices` | 240–357 | `parallel for collapse(2)` |
| 4 | `op4_cadenas_iguales` | 359–401 | `parallel for reduction(+:diferencias)` |
| 5 | `op5_ordenar` | 403–490 | `parallel for` (una región por fase) |
| 6 | `op6_palabra_en_texto` | 492–618 | `parallel for reduction(+:encontrada)` |
| 7 | `op7_interseccion` | 620–768 | `parallel` + `omp for` + `omp critical` |
| 8 | `op8_raices_cuadraticas` | 770–822 | `parallel sections` (dos bloques) |
| 9 | `op9_contar_palabra` | 824–945 | `parallel for reduction(+:apariciones)` |
| 10 | `op10_sumatoria` | 947–1019 | `parallel for reduction(+:suma) schedule(static)` |

---

## Las diez operaciones

Cada ficha sigue la misma plantilla para poder compararlas: enunciado, idea,
decisiones de interpretación, paralelización, justificación de corrección,
código clave, ejemplo y puntos de estudio.

---

### Operación 1 — Espacios en blanco en una cadena

**Enunciado.** Contar cuántos espacios en blanco tiene una cadena.

**Idea.** Leer la cadena completa y recorrerla carácter a carácter; cada
carácter que cumpla `isspace()` incrementa un contador.

**Decisiones de interpretación.**

- Se cuenta **todo** lo que C considera espacio en blanco (`' '`, `'\t'`,
  `'\v'`, `'\f'`, `'\r'`), no solo el espacio literal. Por eso una cadena con
  tabuladores cuenta esos tabuladores.
- La cadena se lee sin el salto de línea final, así que `\n` no se cuenta.
- Longitud arbitraria: se usa `leer_linea_dinamica`, no un arreglo fijo.

**Paralelización con OpenMP.**

```c
size_t espacios = 0;

#pragma omp parallel for reduction(+:espacios)
for (size_t i = 0; i < n; i++) {
    if (isspace((unsigned char)cadena[i])) {
        espacios++;
    }
}
```

- `parallel for` reparte las iteraciones `i` entre los hilos.
- Sin `reduction`, todos los hilos harían `espacios++` sobre la misma variable
  y se perderían incrementos (*condición de carrera*). La cláusula
  `reduction(+:espacios)` crea **una copia privada por hilo**, inicializada a 0,
  y al final de la región las suma en orden; el resultado es el mismo que el
  secuencial porque la suma de enteros es asociativa y conmutativa.
- Cada hilo solo **lee** `cadena`, que no se modifica dentro de la región.

**¿Por qué es correcto?** Cada iteración solo depende de `cadena[i]` y del
contador privado. El contador final es la suma de los contadores privados, que
es exactamente el número de posiciones con espacio.

**Ejemplo de ejecución.**

```text
Ingresa la cadena: hola	mundo  openmp
Numero de espacios en blanco: 3
```

Entrada: `hola` + tabulador + `mundo` + dos espacios + `openmp` → 1 + 2 = 3.

**Para estudiar.**

- ¿Qué pasaría si se declara `espacios` dentro de la región `parallel`? Dejaría
  de ser compartida y el valor se perdería al salir de la región.
- ¿Por qué se hace el *cast* a `unsigned char` antes de `isspace`? Porque
  `isspace` solo está definida para valores representables como `unsigned char`
  o `EOF`; con un `char` negativo (bytes UTF-8) el comportamiento es indefinido.
- ¿Tiene sentido paralelizar una cadena de 20 caracteres? No: el coste de crear
  la región paralela supera el trabajo. Se hace porque lo pide la práctica y
  porque el mismo patrón escala a cadenas grandes.

---

### Operación 2 — Palabras y caracteres de un archivo

**Enunciado.** Contar las palabras (caracteres consecutivos sin espacio) y los
caracteres de un archivo de texto.

**Idea.** Cargar el archivo completo en memoria y recorrerlo una sola vez,
calculando ambos conteos en el mismo bucle.

**Decisiones de interpretación.**

- **Palabra**: comienza en una posición no-espacio cuyo carácter anterior es
  espacio (o que es la posición 0). Así, una secuencia de caracteres sin
  espacios cuenta como una palabra y varias palabras separadas por varios
  espacios se cuentan bien.
- **Carácter**: se cuenta un byte como carácter si **no** es un byte de
  continuación UTF-8 (`(c & 0xC0) != 0x80`). De este modo `á`, `ñ`, `ü`, etc.
  cuentan como **un** carácter y no como dos bytes. Es una aproximación por
  puntos de código: no interpreta secuencias inválidas ni combina acentos.
- El archivo se abre en modo binario (`"rb"`) para no alterar los bytes.

**Paralelización con OpenMP.**

```c
size_t palabras = 0;
size_t caracteres = 0;

#pragma omp parallel for reduction(+:palabras, caracteres)
for (size_t i = 0; i < n; i++) {
    unsigned char c = (unsigned char)contenido[i];

    if ((c & 0xC0) != 0x80) {            /* no es byte de continuación UTF-8 */
        caracteres++;
    }

    int es_caracter_palabra = !isspace(c);
    int anterior_es_espacio = (i == 0) || isspace((unsigned char)contenido[i - 1]);
    if (es_caracter_palabra && anterior_es_espacio) {
        palabras++;
    }
}
```

- Dos reducciones independientes en la misma directiva.
- La decisión de si `i` inicia palabra depende de `contenido[i-1]`. Esto **no
  es una dependencia entre iteraciones**: leer un elemento vecino que nunca se
  escribe no genera carrera. La iteración `i` solo *lee* `i-1`.
- No hace falta dividir el archivo: cada hilo recibe un rango contiguo de
  índices y aplica la misma regla local.

**¿Por qué es correcto?** La regla de inicio de palabra usa únicamente el byte
actual y el anterior; ambos son de solo lectura. El conteo total es la suma de
los conteos por rango, y las reducciones combinan los parciales.

**Ejemplo de ejecución.** Archivo con el contenido `acción rápida\n`:

```text
Numero de palabras: 2
Numero de caracteres: 14
```

Son 13 caracteres visibles (`acción` = 6, espacio, `rápida` = 6) más el salto
de línea. Nótese que la `ó` y la `á` cuentan una sola vez, aunque ocupen dos
bytes cada una.

**Para estudiar.**

- ¿Qué contaría el programa si se usara `strlen` como número de caracteres?
  Bytes: obtendría 15 para el ejemplo, no 14.
- ¿Qué pasa con palabras separadas por tabuladores o saltos de línea? También
  son separadores, porque se usa `isspace`.
- ¿Por qué no se puede paralelizar la lectura del archivo? Porque `fread` es
  secuencial y el archivo puede no caber en una sola posición de memoria
  compartida; primero se carga y después se paraleliza el recorrido.

---

### Operación 3 — Suma de matrices cuadradas

**Enunciado.** Sumar dos matrices cuadradas de enteros.

**Idea.** Leer la dimensión `n`, reservar tres arreglos planos de `n*n`
elementos (`A`, `B` y `C`) y calcular `C[i][j] = A[i][j] + B[i][j]` para cada
posición. Se usa indexación plana `i*n + j` en lugar de arreglos de punteros:
es más simple de reservar y de recorrer.

**Decisiones de interpretación.**

- `A` y `B` se guardan como `int`; `C` como `long long` para que la suma no se
  desborde con facilidad (con entradas de hasta 100, `C` cabe en `int`, pero el
  tipo ancho es más seguro si se cambian los rangos).
- La dimensión debe ser positiva; se valida.
- Si `n <= 10` se imprimen las tres matrices; si es mayor, solo se avisa del
  resultado, para no inundar la consola.
- Entrada por teclado (por filas) o generación aleatoria en `[-100, 100]`.

**Paralelización con OpenMP.**

```c
/* Cada elemento de C es independiente -> bucles anidados en paralelo */
#pragma omp parallel for collapse(2)
for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
        size_t idx = (size_t)i * (size_t)n + (size_t)j;
        C[idx] = (long long)A[idx] + (long long)B[idx];
    }
}
```

- `collapse(2)` fusiona los dos bucles en un único espacio de iteraciones de
  `n*n` elementos, que se reparte entre los hilos. Sin `collapse`, solo se
  paralelizaría el bucle externo (útil con `n` grande, pero con menos
  granularidad).
- Cada iteración escribe una posición **distinta** de `C`, así que no hay
  carrera ni necesidad de sincronización.
- `A` y `B` solo se leen.

**¿Por qué es correcto?** La suma de matrices es *elemento a elemento*: el
resultado en `(i,j)` no depende de ningún otro elemento. El reparto de
iteraciones es una partición del espacio de índices.

**Ejemplo de ejecución** con `n = 2`, `A = [1 2; 3 4]`, `B = [5 6; 7 8]`:

```text
Matriz A + B:
     6      8
    10     12
```

**Para estudiar.**

- ¿Qué diferencia hay entre `collapse(2)` y dejar solo el `for` externo? Con
  `n` pequeño, el bucle externo da menos iteraciones que hilos y se desaprovecha
  paralelismo; `collapse` reparte `n²` iteraciones.
- ¿Por qué se usan arreglos planos y no `int **`? Menos reservas, memoria
  contigua (mejor localidad de caché) y una sola dirección base que compartir.
- Si se sumaran matrices de tipo `int` con valores cercanos a `INT_MAX`, ¿qué
  habría que cambiar? El tipo del acumulador/elemento de `C` a `long long`.

---

### Operación 4 — ¿Dos cadenas son iguales?

**Enunciado.** Revisar si dos cadenas son iguales.

**Idea.** Leer ambas cadenas, comparar longitudes y, si coinciden, comparar
carácter a carácter contando las diferencias.

**Decisiones de interpretación.**

- La comparación es **exacta**: distingue mayúsculas y minúsculas y no ignora
  espacios. Es el comportamiento de `strcmp`.
- Si las longitudes difieren, se responde inmediatamente sin recorrer nada.
- Se informa el número de posiciones distintas, no solo sí/no, lo que ayuda a
  depurar y a explicar el resultado.

**Paralelización con OpenMP.**

```c
size_t diferencias = 0;

#pragma omp parallel for reduction(+:diferencias)
for (size_t i = 0; i < n1; i++) {
    if (s1[i] != s2[i]) {
        diferencias++;
    }
}
```

- Solo se entra al bucle si `n1 == n2`, condición imprescindible para no leer
  fuera de rango.
- Cada posición se compara de forma independiente; las dos cadenas son de solo
  lectura.
- La reducción cuenta las diferencias, no las posiciones: no importa el orden
  en que se detecten.

**¿Por qué es correcto?** Dos cadenas de la misma longitud son iguales si y
solo si coinciden en todas las posiciones. `diferencias == 0` equivale a
igualdad. La reducción devuelve el cardinal de posiciones distintas.

**Ejemplo de ejecución.**

```text
Ingresa la primera cadena: abc
Ingresa la segunda cadena: abc
Las cadenas son iguales.
```

```text
Ingresa la primera cadena: abc
Ingresa la segunda cadena: abd
Las cadenas NO son iguales (difieren en 1 posicion(es)).
```

**Para estudiar.**

- ¿Por qué no se puede hacer un `break` en cuanto se encuentra una diferencia
  dentro de un `for` con `reduction`? Porque las iteraciones se reparten entre
  hilos y el `break` solo afectaría al hilo actual; además, OpenMP no permite
  salir de una región `for` de forma arbitraria.
- Alternativa "de verdad" para cadenas iguales: comparar por bloques y usar
  `reduction(&:and)` o `reduction(||:hay_diferencia)`.
- ¿Qué diferencia hay con `strcmp(s1, s2) == 0`? Ninguna en resultado; la
  versión paralela existe para practicar reducciones.

---

### Operación 5 — Ordenar enteros

**Enunciado.** Ordenar un conjunto de números enteros.

**Idea.** Se implementa **ordenamiento par-impar** (*odd-even transposition
sort*), un algoritmo pensado para paralelizarse:

- En la fase par se comparan e intercambian los pares `(0,1), (2,3), (4,5)…`
- En la fase impar se comparan `(1,2), (3,4), (5,6)…`
- Se repiten `n` fases alternando el inicio (`0` o `1`).

En cada fase, los pares son **disjuntos**: ninguna posición participa en dos
comparaciones a la vez.

**Decisiones de interpretación.**

- Orden **ascendente**.
- Entrada por teclado o aleatoria en `[-100, 100]`.
- Se imprime el arreglo original y el ordenado solo si `n <= 50`.

**Paralelización con OpenMP.**

```c
for (int fase = 0; fase < n; fase++) {
    int inicio = fase % 2;

    #pragma omp parallel for
    for (int i = inicio; i < n - 1; i += 2) {
        if (v[i] > v[i + 1]) {
            int tmp = v[i];
            v[i] = v[i + 1];
            v[i + 1] = tmp;
        }
    }
}
```

- El bucle de fases es **secuencial**; dentro de cada fase se crea una región
  paralela que reparte las comparaciones.
- Al final de cada `parallel for` hay una **barrera implícita**: ningún hilo
  empieza la fase siguiente hasta que todos terminan la actual. Eso es lo que
  evita que un hilo compare `v[i+1]` mientras otro lo está intercambiando.
- Dentro de una fase, las iteraciones tocan posiciones disjuntas: `i` y `i+1`
  para la iteración `i`, y `i+2`, `i+3` para la siguiente. No hay solapamiento.

**¿Por qué es correcto?** El algoritmo par-impar es una red de ordenamiento:
tras `n` fases, todo par de elementos ha sido comparado en el orden correcto
(equivalentemente, el elemento mayor "burbujea" hasta el final y el proceso se
repite). La barrera entre fases garantiza que cada comparación ve el estado
completo de la fase anterior.

**Ejemplo de ejecución.**

```text
Cuantos numeros enteros? 5
Ingresa 5 enteros separados por espacio:
5 1 4 2 3
Original: [5, 1, 4, 2, 3]
Ordenado: [1, 2, 3, 4, 5]
```

**Para estudiar.**

- ¿Por qué no se pueden paralelizar a la vez las fases par e impar? Porque
  escriben sobre posiciones solapadas (`v[1]` participa en `(0,1)` y en
  `(1,2)`); necesitan una barrera entre ellas.
- ¿Cuál es el coste? `O(n)` fases × `O(n)` comparaciones = `O(n²)` en el peor
  caso; es didáctico, no compite con `qsort` (`O(n log n)`).
- ¿Por qué el rendimiento deja de mejorar a partir de 4–8 hilos? Porque cada
  fase es muy corta y se crea una región paralela nueva en cada una; además, la
  memoria compartida y las barreras limitan la escalabilidad. Ver
  [mediciones](#cómo-medir-el-rendimiento).
- ¿Sería más eficiente usar una sola región `parallel` con `#pragma omp for`
  dentro del bucle de fases? Sí: se ahorraría la creación/destrucción de hilos
  en cada fase (las barreras siguen siendo necesarias).

---

### Operación 6 — ¿La palabra está en el texto?

**Enunciado.** Revisar si una palabra se encuentra en un texto.

**Idea.** Convertir el texto en una lista de palabras normalizadas y buscar la
palabra objetivo en esa lista.

Pasos:

1. Elegir entrada: teclado o archivo.
2. Leer el texto completo.
3. **Tokenizar** secuencialmente: separar por espacios en blanco.
4. **Normalizar** cada token: quitar puntuación inicial/final y pasar a
   minúsculas.
5. Buscar el objetivo en paralelo.

**Decisiones de interpretación.**

- La comparación **no distingue mayúsculas** y **ignora la puntuación pegada**:
  `(OpenMP).` se normaliza a `openmp`.
- La puntuación **interna** se conserva (`don't` sigue siendo `don't`), porque
  solo se recortan los extremos.
- La búsqueda es por palabra completa, no por subcadena: buscar `omp` no
  encuentra `openmp`.
- La tokenización es secuencial a propósito: es dependiente del orden y no
  domina el tiempo frente a la búsqueda cuando hay muchas palabras.

**Paralelización con OpenMP.**

```c
int encontrada = 0;

#pragma omp parallel for reduction(+:encontrada)
for (size_t k = 0; k < num_palabras; k++) {
    if (strcmp(palabras[k], objetivo) == 0) {
        encontrada = 1;
    }
}
```

- `strcmp` solo lee ambas cadenas; cada iteración es independiente.
- La reducción suma los `1`; al final basta con comprobar `encontrada > 0`.
- No se puede abandonar el bucle al encontrar la palabra (no hay `break`
  válido en un `for` paralelo). Se recorren todas las palabras, lo cual está
  bien para responder "¿está?" y es la base de la operación 9.

**¿Por qué es correcto?** La existencia de la palabra es la disyunción de las
comparaciones individuales; contar cuántas coinciden y comprobar si el conteo
es mayor que cero es equivalente.

**Ejemplo de ejecución.**

```text
Ingresa el texto: Hola, OpenMP! openmp es genial; (OpenMP).
Ingresa la palabra a buscar: openmp
La palabra 'openmp' SI se encuentra en el texto.
```

```text
Ingresa el texto: Hola mundo
Ingresa la palabra a buscar: python
La palabra 'python' NO se encuentra en el texto.
```

**Para estudiar.**

- ¿Qué pasaría si no se normalizara? `(OpenMP).` no coincidiría con `openmp`.
- ¿Por qué se guardan las palabras ya normalizadas en lugar de normalizar
  dentro del bucle paralelo? Para no repetir trabajo y para que el bucle
  paralelo solo haga lecturas.
- La lista de palabras se construye con `malloc`/`realloc` y cada token se
  libera al final: cuidado con las fugas de memoria si se añaden salidas
  anticipadas.

---

### Operación 7 — Intersección de conjuntos

**Enunciado.** Obtener la intersección de dos conjuntos de enteros.

**Idea.** Para cada elemento de `A`, comprobar si aparece en `B`; si aparece y
no se había registrado, añadirlo al resultado. Se eliminan duplicados.

**Decisiones de interpretación.**

- El resultado es un **conjunto**: sin elementos repetidos, aunque `A` o `B`
  tengan duplicados.
- El orden de salida **no está garantizado** (depende del orden en que los
  hilos hacen la fusión). Ver el ejemplo.
- Entrada por teclado o aleatoria en `[-25, 25]`.

**Paralelización con OpenMP.** Es la operación con la estrategia más elaborada:

```c
#pragma omp parallel
{
    int *local_inter = malloc((size_t)max_inter * sizeof *local_inter);
    int local_count = 0;

    #pragma omp for
    for (int i = 0; i < n; i++) {
        int val = A[i];
        /* ¿ya está en el buffer local del hilo? */
        /* ¿está en B?  (búsqueda lineal con break) */
        /* si pasa ambas, local_inter[local_count++] = val; */
    }

    #pragma omp critical
    {
        /* fusionar local_inter en interseccion, descartando duplicados */
    }

    free(local_inter);
}
```

- `#pragma omp parallel` crea la región; `#pragma omp for` reparte las
  iteraciones sobre `A`.
- Cada hilo acumula sus hallazgos en un **búfer privado** (`local_inter`). Así
  evita tocar el resultado compartido en cada coincidencia.
- Solo al final, cada hilo entra una vez en la **región crítica** para fusionar
  su búfer con el resultado global. La sección crítica es corta: proporcional
  al número de coincidencias, no al trabajo total.
- La deduplicación se hace dos veces: dentro del hilo (al añadir a su búfer) y
  al fusionar (por si dos hilos encontraron el mismo valor).

**¿Por qué es correcto?** La intersección es `{x ∈ A : x ∈ B}`. Cada elemento
de `A` se examina exactamente una vez entre todos los hilos. Los búferes
privados evitan carreras y la fusión bajo `critical` garantiza exclusión mutua
al escribir `interseccion`/`count`. La comprobación de duplicados en la fusión
hace que el resultado sea un conjunto.

**Ejemplo de ejecución** con `A = {1,2,3,4,5}` y `B = {3,4,5,6,7}`:

```text
Interseccion de A y B (3 elementos):
[5, 4, 3]
```

El contenido es correcto, pero el **orden cambia entre ejecuciones** porque los
hilos entran en la sección crítica en orden no determinista. Si se necesitara
un orden estable habría que ordenar el resultado al final.

**Para estudiar.**

- ¿Por qué no meter todo el bucle dentro de `critical`? Funcionaría, pero
  serializaría el cómputo y eliminaría el paralelismo. El patrón correcto es
  *trabajo en paralelo, fusión en exclusión*.
- ¿Qué complejidad tiene? `O(n·m)` en el peor caso por la búsqueda lineal de
  cada elemento de `A` en `B`. Con conjuntos ordenados podría hacerse con
  búsqueda binaria (`O(n log m)`) o con una tabla hash (`O(n+m)`).
- ¿Qué pasa con los duplicados dentro de `A`? Se descartan tanto en el búfer
  local como en la fusión.
- Detalle a revisar: si `malloc` del búfer local devolviera `NULL`, el código
  no lo comprueba antes de escribir. Es una mejora pendiente (ver
  [limitaciones](#limitaciones-y-posibles-mejoras)).

---

### Operación 8 — Raíces reales de una cuadrática

**Enunciado.** Calcular las raíces reales de una ecuación cuadrática
`ax² + bx + c = 0`.

**Idea.** Aplicar la fórmula general:

```text
D = b² - 4ac
x₁ = (-b + √D) / (2a)
x₂ = (-b - √D) / (2a)
```

**Decisiones de interpretación.**

- Si `a == 0` no es una ecuación cuadrática (sería lineal); se avisa y se sale.
- Si `D < 0` no hay raíces reales; se avisa y se sale.
- Si `D == 0` se imprimen dos raíces iguales (no se distingue raíz doble).
- Los coeficientes se leen como `double` con `scanf("%lf")`.
- Se imprimen cuatro decimales.

**Paralelización con OpenMP.** Se usa **descomposición funcional**, no reparto
de iteraciones: las tareas son distintas y se ejecutan en hilos distintos.

```c
double b2 = 0.0, ac4 = 0.0;

#pragma omp parallel sections
{
    #pragma omp section
    { b2 = b * b; }

    #pragma omp section
    { ac4 = 4.0 * a * c; }
}

/* barrera implícita: aquí b2 y ac4 ya están calculados */
D = b2 - ac4;
```

y, si `D >= 0`:

```c
#pragma omp parallel sections
{
    #pragma omp section
    { x1 = (-b + r) / (2.0 * a); }

    #pragma omp section
    { x2 = (-b - r) / (2.0 * a); }
}
```

- `sections` divide el trabajo en bloques que pueden ejecutarse en paralelo.
- Al final del bloque hay una **barrera implícita**: `D = b2 - ac4` no se
  ejecuta hasta que ambas secciones terminan. Sin ella, `D` podría leer una
  variable aún sin calcular.
- Cada sección escribe una variable **distinta**, así que no hay carrera.
- `r = sqrt(D)` se calcula en serie porque ambas raíces lo necesitan.

**¿Por qué es correcto?** Las dos primeras secciones calculan términos
independientes de la fórmula; la barrera garantiza que la resta vea ambos.
Después, cada raíz depende de `b`, `r`, `a`, que ya son de solo lectura, y
escribe su propia variable.

**Ejemplo de ejecución.**

```text
Coeficiente a: 1
Coeficiente b: -3
Coeficiente c: 2
Discriminante D = 1.0000
Raiz 1 = 2.0000
Raiz 2 = 1.0000
```

```text
Coeficiente a: 1
Coeficiente b: 0
Coeficiente c: 1
Discriminante D = -4.0000
No hay raices reales.
```

```text
Coeficiente a: 0
No es una ecuacion cuadratica (a = 0).
```

**Para estudiar.**

- ¿Por qué aquí no se usa `for`? Porque no hay un conjunto de iteraciones
  homogéneas que repartir; hay dos tareas diferentes. `sections` es el
  mecanismo adecuado para paralelismo funcional.
- ¿Aporta velocidad? No: el trabajo por sección es una multiplicación. El
  coste de crear los hilos es mucho mayor. Se usa para practicar `sections`;
  en producción sería código secuencial.
- ¿Qué pasaría sin la barrera implícita? `D` podría calcularse con `b2` o
  `ac4` a 0, dando un resultado incorrecto de forma intermitente.
- ¿Qué pasa si `a` es muy pequeño pero no 0? La fórmula es numéricamente
  inestable; se podría usar la forma de Vieta para mejorar la precisión.

---

### Operación 9 — Apariciones de una palabra

**Enunciado.** Revisar cuántas veces aparece una palabra en un texto.

**Idea.** Es la operación 6 con un contador en lugar de un booleano. Comparte
tokenización y normalización; cambia solo el bucle final.

**Decisiones de interpretación.** Las mismas que en la operación 6: sin
distinguir mayúsculas, ignorando puntuación en los extremos, por palabra
completa.

**Paralelización con OpenMP.**

```c
size_t apariciones = 0;

#pragma omp parallel for reduction(+:apariciones)
for (size_t k = 0; k < num_palabras; k++) {
    if (strcmp(palabras[k], objetivo) == 0) {
        apariciones++;
    }
}
```

- Cada hilo acumula en su copia privada el número de coincidencias de su
  rango; la reducción las suma al final.
- A diferencia de la operación 6, aquí **no se puede evitar** recorrer todas
  las palabras: hay que contarlas todas.

**¿Por qué es correcto?** El número total de apariciones es la suma de las
apariciones en cada subconjunto del reparto, y la suma es asociativa.

**Ejemplo de ejecución** con el texto
`Hola, OpenMP! openmp es genial; (OpenMP).` y objetivo `openmp`:

```text
La palabra 'openmp' aparece 3 veces.
```

Coinciden `OpenMP`, `openmp` y `(OpenMP)` tras normalizar.

**Para estudiar.**

- ¿Qué diferencia hay con la operación 6? Solo el tipo de acumulador y la
  pregunta final (`> 0` frente al valor exacto). Comparar ambas funciones es un
  buen ejercicio de refactorización.
- ¿Cómo se podría contar también subcadenas o frases? Habría que cambiar la
  tokenización, no el bucle paralelo.
- ¿Por qué `size_t` y no `int`? Para que el contador pueda representar el
  número de palabras de un archivo grande sin desbordar.

---

### Operación 10 — Sumatoria de n enteros

**Enunciado.** Calcular la sumatoria de `n` números enteros.

**Idea.** Leer `n`, reservar un arreglo, llenarlo (teclado o aleatorio) y sumar
todos sus elementos.

**Decisiones de interpretación.**

- Acumulador `long long` para reducir el riesgo de desbordamiento (con
  `int` y `n` grande, la suma podría superar `INT_MAX`).
- `n` debe ser positivo y se valida.
- Entrada aleatoria en `[-100, 100]`.

**Paralelización con OpenMP.**

```c
long long suma = 0;

#pragma omp parallel for reduction(+:suma) schedule(static)
for (int i = 0; i < n; i++) {
    suma += (long long)v[i];
}
```

- `reduction(+:suma)` da a cada hilo su propio acumulador y los combina al
  final. Es el patrón clásico de reducción y evita la condición de carrera de
  `suma += ...`.
- `schedule(static)` reparte el rango en bloques contiguos de tamaño similar
  (el reparto por defecto). Es adecuado aquí porque todas las iteraciones
  cuestan lo mismo. `dynamic` añadiría sobrecarga de planificación sin
  beneficio.
- La suma de enteros es asociativa y conmutativa, por lo que el resultado no
  depende del reparto (ni del orden de combinación).

**¿Por qué es correcto?** El valor final es la suma de los acumuladores
privados; cada elemento del arreglo se suma exactamente una vez.

**Ejemplo de ejecución.**

```text
Cuantos numeros enteros? 5
Ingresa 5 enteros separados por espacio:
1 2 3 4 5
arreglo: [2, 3, 4, 5, 1, ]
Sumatoria = 15
```

**Atención al ejemplo:** la línea `arreglo:` se imprime dentro de un
`#pragma omp parallel for`, por lo que **el orden de los números no está
garantizado** (cada hilo imprime su bloque cuando le toca). El valor de la
sumatoria sí es siempre correcto. Para una salida ordenada habría que imprimir
en serie o usar `ordered`.

**Para estudiar.**

- ¿Qué pasaría si se quitara `reduction`? Varios hilos harían
  `suma += v[i]` a la vez: incrementos perdidos y resultados distintos en cada
  ejecución (además de comportamiento indefinido por carrera de datos).
- ¿Por qué el tiempo no baja linealmente con los hilos? Porque la suma es una
  operación que consume memoria: a partir de cierto número de hilos el cuello
  de botella es el ancho de banda de memoria, no la CPU. Ver
  [mediciones](#cómo-medir-el-rendimiento).
- ¿Por qué no se paraleliza la generación aleatoria? `rand()` usa estado
  global y no es segura entre hilos; además, generar es mucho más barato que
  sumar para `n` grande. Si se quisiera paralelizar, cada hilo debería usar su
  propio generador (`rand_r` o `drand48` con semilla distinta).

---

## Resumen de directivas OpenMP usadas

| Operación | Directiva / cláusula | Patrón | Qué se comparte y qué no |
|---|---|---|---|
| 1 | `parallel for reduction(+:espacios)` | Reparto de iteraciones | `cadena` compartida (solo lectura); contador privado. |
| 2 | `parallel for reduction(+:palabras, caracteres)` | Reparto + 2 reducciones | `contenido` compartido; dos contadores privados. |
| 3 | `parallel for collapse(2)` | Reparto 2D | `A`, `B` leídas; cada hilo escribe filas distintas de `C`. |
| 4 | `parallel for reduction(+:diferencias)` | Reparto + reducción | Ambas cadenas de solo lectura. |
| 5 | `parallel for` (en bucle externo secuencial) | Reparto por fase | `v` compartido; posiciones disjuntas por fase; barrera entre fases. |
| 6 | `parallel for reduction(+:encontrada)` | Reparto + reducción | Lista de tokens de solo lectura; contador privado. |
| 7 | `parallel` + `omp for` + `omp critical` | Reparto + búfer local + exclusión mutua | `A`, `B` compartidas; `local_inter` privado; fusión crítica. |
| 8 | `parallel sections` ×2 | Descomposición funcional | Cada sección escribe su propia variable; barrera implícita. |
| 9 | `parallel for reduction(+:apariciones)` | Reparto + reducción | Igual que 6, contando en vez de comprobando. |
| 10 | `parallel for reduction(+:suma) schedule(static)` | Reparto + reducción estática | `v` de solo lectura; acumulador privado. |

**Clasificación por mecanismo:**

- **Reparto de iteraciones (`for`)**: 1, 2, 3, 4, 5, 6, 9, 10.
- **Descomposición funcional (`sections`)**: 8.
- **Sincronización explícita (`critical`)**: 7.
- **Reducciones**: 1, 2, 4, 6, 9, 10.
- **Barreras implícitas**: al final de cada `for`/`sections`; en la operación 5
  son las que hacen correcto el algoritmo.

---

## Glosario OpenMP

| Concepto | Definición breve |
|---|---|
| **Región paralela** (`#pragma omp parallel`) | Bloque de código que ejecutan todos los hilos. Fuera de ella el programa es secuencial. |
| **Reparto de trabajo** (`#pragma omp for`) | Divide las iteraciones de un bucle entre los hilos del equipo actual. |
| **`parallel for`** | Abreviatura de una región paralela que contiene un único `for` repartido. |
| **Barrera** | Punto donde todos los hilos esperan hasta que llegue el último. Al final de `for`/`sections` es implícita; también existe `#pragma omp barrier`. |
| **Reducción** (`reduction(op:var)`) | Cada hilo opera sobre una copia privada de `var`; al final se combinan con `op` (suma, producto, mínimo, máximo, AND/OR lógicos…). |
| **Variable compartida** | Visible y modificable por todos los hilos; es la opción por defecto para variables declaradas fuera de la región. |
| **Variable privada** | Cada hilo tiene su propia copia; se declara con `private`, `firstprivate` o `lastprivate`, o nace dentro de la región. |
| **Sección crítica** (`#pragma omp critical`) | Zona de exclusión mutua: solo un hilo la ejecuta a la vez. |
| **`sections`** | Reparte bloques de código distintos (no iteraciones) entre hilos. |
| **`collapse(n)`** | Fusiona `n` bucles anidados en un solo espacio de iteraciones para repartirlo mejor. |
| **`schedule(static/dynamic/guided)`** | Cómo se asignan las iteraciones a los hilos: bloques fijos, bajo demanda o decrecientes. |
| **Condición de carrera** | Dos hilos acceden a la misma variable y al menos uno escribe, sin sincronización; el resultado depende del orden. |
| **`false sharing`** | Dos hilos escriben variables distintas que caen en la misma línea de caché; degrada el rendimiento aunque no haya carrera lógica. |
| **Ley de Amdahl** | La ganancia máxima está limitada por la fracción secuencial del programa. |
| **`OMP_NUM_THREADS`** | Variable de entorno que fija el número de hilos del equipo por defecto. |

---

## Complejidad y Ley de Amdahl

| # | Trabajo paralelizable | Parte secuencial | Complejidad |
|---|---|---|---|
| 1 | Recorrido de la cadena | Lectura de la línea | `O(n)` |
| 2 | Recorrido del archivo | Lectura del archivo | `O(n)` |
| 3 | Suma de `n²` elementos | Lectura/impresión | `O(n²)` |
| 4 | Comparación de `n` posiciones | Lectura de cadenas | `O(n)` |
| 5 | Comparaciones de cada fase | `n` fases con barreras | `O(n²)` peor caso |
| 6 | Búsqueda en la lista de tokens | Tokenización + normalización | `O(T)` + `O(P·L)` |
| 7 | Búsqueda de cada elemento de `A` en `B` | Fusión crítica | `O(n·m)` |
| 8 | Cálculo de `b²` y `4ac` | Lectura y `sqrt` | `O(1)` |
| 9 | Conteo sobre los tokens | Tokenización + normalización | `O(T)` + `O(P·L)` |
| 10 | Suma de `n` elementos | Generación de datos | `O(n)` |

La **Ley de Amdahl** explica por qué este programa no escala infinitamente:

```text
S(p) = 1 / ((1 - f) + f / p)
```

donde `f` es la fracción paralelizable y `p` el número de hilos. Si el 90 % del
tiempo es paralelizable, el máximo teórico con infinitos hilos es `10×`. En
este programa, además:

- La lectura de archivos y la tokenización son secuenciales.
- La impresión por consola es secuencial y, en la operación 10, se hace dentro
  de un `for` paralelo, lo que la convierte en el cuello de botella real.
- Las operaciones 1, 4, 8 y 10 tienen muy poco trabajo por iteración: el coste
  de crear hilos y sincronizar puede superar la ganancia.

**Conclusión práctica:** el programa está pensado para *demostrar* OpenMP. Las
operaciones 2, 5, 6, 7, 9 y 10 son las que tienen sentido medir con tamaños
grandes; las demás son ejercicios de corrección.

---

## Cómo medir el rendimiento

Compilar con optimizaciones (el `Makefile` ya usa `-O2`) y comparar con
distinto número de hilos:

```bash
OMP_NUM_THREADS=1 ./programa
OMP_NUM_THREADS=4 ./programa
```

Para medir tiempo de forma objetiva, usar `time` (o `perf_counter` en Python)
y **redirigir la salida**, porque imprimir domina el tiempo:

```bash
for t in 1 2 4 8; do
    printf '5\n20000\n2\n0\n' | OMP_NUM_THREADS=$t /usr/bin/time -f "$t hilos: %e s" ./programa > /dev/null
done
```

### Medición de la operación 5 (ordenamiento)

Máquina de pruebas: 12 CPUs lógicas, `gcc 16.2.1`, `-O2`, `n = 20 000` enteros
aleatorios, salida descartada.

| Hilos | Tiempo (s) | *Speedup* |
|---:|---:|---:|
| 1 | 0.225 | 1.00× |
| 2 | 0.131 | 1.72× |
| 4 | 0.078 | 2.88× |
| 8 | 0.082 | 2.74× |
| 12 | 0.072 | 3.13× |

El *speedup* se estanca por la creación de una región paralela por fase y por
las barreras; aun así, el paralelismo es real.

### Medición aislada de la reducción de la operación 10

El programa imprime todo el arreglo dentro de un `parallel for`, así que medir
la opción 10 completa mide principalmente `printf`. Para ver el efecto de la
reducción se puede compilar un banco de pruebas mínimo:

```c
/* bench_suma.c — solo el bucle de suma */
long long suma = 0;
double t0 = omp_get_wtime();
#pragma omp parallel for reduction(+:suma) schedule(static)
for (int i = 0; i < n; i++) suma += (long long)v[i];
double t1 = omp_get_wtime();
printf("hilos=%d suma=%lld tiempo=%.4f s\n",
       omp_get_max_threads(), suma, t1 - t0);
```

```bash
gcc -std=c11 -O2 -fopenmp bench_suma.c -o bench_suma
for t in 1 2 4 8 12; do OMP_NUM_THREADS=$t ./bench_suma 200000000; done
```

Resultados con `n = 200 000 000`:

| Hilos | Tiempo (s) | *Speedup* |
|---:|---:|---:|
| 1 | 0.0541 | 1.00× |
| 2 | 0.0279 | 1.94× |
| 4 | 0.0190 | 2.85× |
| 8 | 0.0189 | 2.86× |
| 12 | 0.0203 | 2.66× |

La suma coincide en todas las configuraciones (`37435` en la ejecución de
prueba), lo que confirma que la reducción no altera el resultado. El
*speedup* se satura porque el bucle está limitado por el **ancho de banda de
memoria**: sumar es casi gratis, traer los datos a la CPU no.

> **Moraleja:** medir siempre con el mismo `n`, los mismos datos y la salida
> descartada. Si se mide la opción 10 completa, el tiempo estará dominado por
> la impresión y no por la suma.

---

## Trampas y errores comunes

1. **Olvidar `reduction` en un acumulador.** `suma += v[i]` con `suma`
   compartida produce incrementos perdidos. Síntoma: el resultado cambia entre
   ejecuciones.
2. **Creer que `parallel for` sirve para cualquier bucle.** No vale con
   `break`, `return` ni con dependencias entre iteraciones (por ejemplo,
   `v[i] = v[i-1] + 1`).
3. **Variables declaradas antes de la región.** Por defecto son compartidas.
   Si cada hilo necesita su propia copia, hay que declararla dentro o usar
   `private`/`firstprivate`.
4. **Escribir en el mismo índice desde varios hilos.** Es una carrera aunque
   el valor escrito sea "el mismo".
5. **Imprimir dentro de una región paralela.** `printf` es seguro (no se
   corrompe la salida), pero el **orden** es arbitrario. En la operación 10 se
   ve claramente.
6. **Confundir barrera implícita con sincronización de datos.** La barrera
   ordena la ejecución, pero las variables compartidas necesitan además
   exclusión mutua si se escriben.
7. **Paralelizar grano demasiado fino.** Crear hilos cuesta; para operaciones
   `O(1)` (operación 8) no compensa.
8. **Asumir que más hilos = más rápido.** Con memoria compartida, el
   *false sharing* y el ancho de banda pueden hacer que 12 hilos vayan peor que
   4.
9. **`rand()` dentro de una región paralela.** No es thread-safe. En este
   programa la generación es secuencial a propósito.
10. **No comprobar `malloc`.** Varias operaciones reservan memoria; las rutas
    principales lo comprueban, pero el búfer local de la operación 7 no.

---

## Pruebas realizadas

Todas las pruebas se ejecutaron con el binario generado por `make` y entradas
redirigidas.

| Caso | Entrada | Resultado |
|---|---|---|
| Op. 1 | `hola<TAB>mundo  openmp` | `Numero de espacios en blanco: 3` |
| Op. 2 | archivo `acción rápida\n` | 2 palabras, 14 caracteres |
| Op. 3 | `n=2`, `A=[1 2; 3 4]`, `B=[5 6; 7 8]` | `A+B = [6 8; 10 12]` |
| Op. 4 | `abc` / `abc` | `Las cadenas son iguales.` |
| Op. 4 | `abc` / `abd` | `NO son iguales (difieren en 1 posicion(es))` |
| Op. 5 | `5 1 4 2 3` | `[1, 2, 3, 4, 5]` |
| Op. 6 | `Hola, OpenMP! openmp es genial; (OpenMP).` / `openmp` | `SI se encuentra` |
| Op. 6 | `Hola mundo` / `python` | `NO se encuentra` |
| Op. 7 | `A={1,2,3,4,5}`, `B={3,4,5,6,7}` | 3 elementos: `5, 4, 3` (orden variable) |
| Op. 8 | `a=1, b=-3, c=2` | `D = 1`, raíces `2` y `1` |
| Op. 8 | `a=0` | `No es una ecuacion cuadratica (a = 0).` |
| Op. 8 | `a=1, b=0, c=1` | `D = -4`, `No hay raices reales.` |
| Op. 9 | texto anterior / `openmp` | `aparece 3 veces` |
| Op. 10 | `1 2 3 4 5` | `Sumatoria = 15` |
| Op. 10 | `n = 10 000 000`, aleatorio | ejecuta correctamente con 1–12 hilos |
| Menú | opción inválida o texto | aviso y vuelta al menú |
| Menú | `0` | `Fin del programa.` |

**Comprobaciones adicionales.**

- Compila sin *warnings* con `-Wall -Wextra`.
- La suma de la operación 10 da el mismo valor con 1, 2, 4, 8 y 12 hilos.
- Las matrices de la operación 3 se imprimen solo cuando `n <= 10`.
- La operación 2 cuenta correctamente acentos y `ñ` (un carácter por punto de
  código).

---

## Preguntas de repaso

Las respuestas están plegadas para poder intentarlo primero.

1. **¿Por qué la operación 1 necesita `reduction` y la 3 no?**

   <details>
   <summary>Ver respuesta</summary>

   Porque en la 1 todos los hilos incrementan **la misma** variable `espacios`.
   En la 3 cada iteración escribe una posición **distinta** de `C`, así que no
   hay ningún dato compartido que se escriba a la vez.
   </details>

2. **¿Qué diferencia hay entre `private` y `reduction`?**

   <details>
   <summary>Ver respuesta</summary>

   `private` crea una copia por hilo, pero al salir de la región el valor
   privado se descarta (salvo `lastprivate`). `reduction` crea además una copia
   inicializada con el elemento neutro y **combina** las copias al final con la
   operación indicada.
   </details>

3. **En la operación 5, ¿qué pasaría si se quitara la barrera implícita entre
   fases?**

   <details>
   <summary>Ver respuesta</summary>

   Un hilo podría empezar la fase siguiente mientras otro aún está
   intercambiando en la fase actual, leyendo posiciones a medio actualizar. El
   resultado sería incorrecto y no determinista. La barrera es lo que convierte
   las fases en pasos atómicos.
   </details>

4. **En la operación 7, ¿por qué se usa un búfer por hilo en lugar de escribir
   directamente en el resultado?**

   <details>
   <summary>Ver respuesta</summary>

   Para reducir el tiempo dentro de la sección crítica. Si cada coincidencia
   entrara en `critical`, los hilos se serializarían constantemente. Con un
   búfer local, cada hilo entra una sola vez y solo al final.
   </details>

5. **¿Qué significa que el orden de la intersección cambie entre
   ejecuciones?**

   <details>
   <summary>Ver respuesta</summary>

   Que los hilos llegan a la sección crítica en orden no determinista. El
   **contenido** del conjunto es siempre el mismo; el orden de inserción, no.
   Si se necesitara orden, habría que ordenar al final (fuera de la región
   paralela).
   </details>

6. **¿Por qué la operación 8 usa `sections` y no `for`?**

   <details>
   <summary>Ver respuesta</summary>

   Porque no hay un conjunto de iteraciones que repartir, sino dos tareas
   distintas e independientes. `sections` está pensado para paralelismo
   funcional, `for` para paralelismo de datos.
   </details>

7. **¿Qué le pasa a la operación 10 si se usa `int` como acumulador?**

   <details>
   <summary>Ver respuesta</summary>

   Puede desbordar. Con `n` grande y valores de hasta 100, la suma puede superar
   `INT_MAX` (2 147 483 647). El desbordamiento de enteros con signo es
   comportamiento indefinido en C; por eso se usa `long long`.
   </details>

8. **¿Por qué la opción 10 imprime el arreglo desordenado?**

   <details>
   <summary>Ver respuesta</summary>

   Porque el `printf` está dentro de un `#pragma omp parallel for`: cada hilo
   imprime sus índices cuando le toca. La suma se calcula aparte con
   `reduction`, por eso sí es correcta.
   </details>

9. **¿La operación 2 cuenta bytes o caracteres?**

   <details>
   <summary>Ver respuesta</summary>

   Cuenta caracteres aproximados por puntos de código UTF-8: descarta los bytes
   de continuación (`10xxxxxx`). Para ASCII coincide con bytes; para texto con
   acentos, no.
   </details>

10. **¿Qué operación tiene el mayor potencial de aceleración y por qué?**

    <details>
    <summary>Ver respuesta</summary>

    Las de mayor carga por iteración y menor parte secuencial: 2 (archivo
    grande), 5 (`O(n²)`), 7 (`O(n·m)`) y 9 (muchos tokens). Las de grano fino
    (1, 4, 8) no se benefician porque el coste de sincronizar supera el trabajo.
    </details>

---

## Checklist: sé explicar esto

Antes de dar la práctica por estudiada, cada integrante debería poder explicar
sin mirar el código:

- [ ] Qué hace `#pragma omp parallel for` y qué reparte exactamente.
- [ ] Qué es una condición de carrera y cómo la evita `reduction`.
- [ ] Por qué la suma de enteros se puede reducir y una resta no.
- [ ] La diferencia entre `private`, `shared`, `firstprivate` y `reduction`.
- [ ] Qué es una barrera implícita y en qué operación es crítica (la 5).
- [ ] Por qué `sections` es adecuado para la operación 8 y no para la 3.
- [ ] Por qué la operación 7 usa búfer local + `critical` en vez de solo
      `critical`.
- [ ] Por qué no se puede usar `break` dentro de un `for` paralelo.
- [ ] Cómo se mide el *speedup* y por qué no es lineal.
- [ ] Qué parte de cada operación queda secuencial (lectura, tokenización,
      impresión) y cómo limita la Ley de Amdahl.
- [ ] Por qué el `printf` dentro de un `for` paralelo desordena la salida.
- [ ] Por qué la operación 2 cuenta bien los acentos y `strlen` no.

---

## Limitaciones y posibles mejoras

Son observaciones honestas sobre el código actual; no afectan a la corrección
de los casos probados, pero conviene conocerlas:

1. **Operación 10 imprime dentro del bucle paralelo.** La salida no está
   ordenada y el `printf` domina el tiempo de ejecución. Mejora: imprimir en
   serie (o solo una muestra) y dejar el bucle paralelo únicamente para la
   suma.
2. **Operación 5 crea una región paralela por fase.** Con `n` grande, el coste
   de creación de hilos es notable. Mejora: una sola región `parallel` con
   `#pragma omp for` dentro del bucle de fases.
3. **Operación 7 no comprueba el `malloc` del búfer local.** Si fallara, se
   escribiría en `NULL`. Mejora: comprobar y abortar la iteración o usar
   `#pragma omp critical` para informar.
4. **Operación 7 usa búsqueda lineal.** `O(n·m)`. Mejora: ordenar y usar
   búsqueda binaria, o una tabla hash.
5. **Operación 8 no valida `scanf`.** Si el usuario escribe algo que no es un
   número, `a`, `b` y `c` quedan sin inicializar. Mejora: comprobar el valor
   devuelto por `scanf` como se hace en las demás operaciones.
6. **La tokenización de 6 y 9 está duplicada.** Mejora: extraer una función
   `tokenizar_y_normalizar` y reutilizarla; además facilitaría comparar ambas
   operaciones.
7. **`rand() % 201 - 100` no es uniforme del todo** (sesgo del módulo) y
   `rand()` no es thread-safe. Para pruebas es suficiente; si se necesitara
   calidad estadística, usar `rand_r` o `<random>`.
8. **No hay control de errores de asignación en todas las rutas** ni límites
   para `n` (por ejemplo, `n` enorme podría agotar la memoria). Mejora: validar
   y liberar siempre.

---

## Conclusión

El programa cubre las diez operaciones del enunciado y, sobre todo, muestra el
abanico de mecanismos de OpenMP sobre problemas reales:

- **Paralelismo de datos** con `parallel for` (1, 2, 3, 4, 5, 6, 9, 10).
- **Reducciones** para acumuladores (1, 2, 4, 6, 9, 10).
- **Colapso de bucles** para aumentar la granularidad (3).
- **Paralelismo funcional** con `sections` (8).
- **Sincronización** con barreras implícitas (5 y 8) y `critical` (7).
- **Búferes privados** para reducir la contención (7).

Las mediciones confirman que el *speedup* es real cuando hay trabajo suficiente
(operación 5) y que se satura cuando el cuello de botella es la memoria
(operación 10) o el grano es demasiado fino (operación 8). Esa es,
precisamente, la lección que deja la práctica.
