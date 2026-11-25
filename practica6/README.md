# Práctica 6 — Diez operaciones con OpenMP

Programa en C con un menú de consola que resuelve las diez operaciones del
enunciado. Cada operación está en su propia función y usa OpenMP para repartir
el trabajo entre hilos.

El programa trabaja siempre con buffers y arreglos de tamaño fijo: no reserves
memoria dinámica, no hay que liberar nada y cada límite está declarado con un
nombre claro al principio del archivo. Si un archivo excede el tope, se avisa y
se procesa solo lo que cabe.

> Este documento sirve a la vez como informe de la práctica y como guía de
> estudio: para cada operación explica qué se pedía, cómo se resolvió, por qué
> la versión paralela es correcta y qué conviene saber para defenderla.

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

Varias operaciones son de grano muy fino: paralelizarlas no las hace más
rápidas. El objetivo es **aplicar y entender los mecanismos de OpenMP**:
reparto de iteraciones (`for`), reducciones, descomposición funcional
(`sections`) y sincronización (`critical`).

---

## Archivos del proyecto

| Archivo | Descripción |
|---|---|
| `programa.c` | Menú, utilidades, las diez operaciones y sus directivas OpenMP. |
| `Makefile` | Compilación con `gcc -std=c11`, OpenMP y `-lm`. |
| `README.md` | Este informe/guía de estudio. |
| `texto_prueba.txt` | Archivo de texto de ejemplo. |

---

## Requisitos

- `gcc` con soporte de OpenMP (`libgomp`). Comprobar con:
  ```bash
  echo | gcc -fopenmp -dM -E - | grep _OPENMP
  # Debe imprimir una línea con #define _OPENMP <versión>
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

Número de hilos (por defecto OpenMP usa todos los núcleos lógicos):

```bash
OMP_NUM_THREADS=4 ./programa
```

Al arrancar, el programa entra en un bucle: muestra el menú, lee una opción,
ejecuta la operación y vuelve al menú. La opción `0` termina.

---

## Estructura del programa

### `main` y el menú

`main` es un bucle infinito con este esqueleto:

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

Cada `case` delega en una función `opN_*`, así que añadir o quitar operaciones
no toca el flujo del menú.

### Utilidades

Hay seis, todas cortas y con nombres que describen lo que hacen:

| Función | Para qué sirve |
|---|---|
| `imprimir_menu()` | Imprime las diez opciones. |
| `limpiar_entrada()` | Descarta caracteres hasta el fin de línea o `EOF`. |
| `leer_linea(buf, cap)` | Lee con `fgets` y quita el salto de línea final. |
| `leer_entero(prompt, &n, positivo)` | Lee un `int`, valida la lectura y, si `positivo` es 1, exige `n > 0`. |
| `leer_archivo(ruta, buf, cap, &len)` | Lee un archivo en el buffer, devuelve su longitud y avisa si no cabe. |
| `normalizar(char *s)` | Recorta puntuación en los extremos y pasa la cadena a minúsculas, en el mismo buffer. |

`leer_linea` usa `fgets` porque las cadenas pueden contener espacios; con
`fscanf("%s")` se cortarían en el primer espacio. Los números se leen con
`scanf`/`fscanf`.

### Límites fijos

```c
#define MAX_LINEA   1024
#define MAX_TEXTO   8192
#define MAX_PALABRA 128
#define MAX_N       100000
#define MAX_DIM     100
```

Las cadenas viven en arreglos locales y los datos grandes en arreglos `static`.
No hay `malloc`, `realloc` ni `free`. Si se pide un tamaño por encima del límite
se imprime un aviso y la operación vuelve al menú.

En la lectura de archivos se reserva espacio para `MAX_TEXTO - 1` bytes y el
último se deja para el `'\0'`. Si el archivo es más grande se muestra:

```text
El archivo es demasiado grande; se procesan solo los primeros 8191 bytes.
```

y se trabaja con ese fragmento.

### Comentarios

Solo aparecen al final de las líneas con algo que se presta a confusión, con
formato `//...` y en minúsculas. Por ejemplo, `//indice plano` en el acceso a
una matriz o `//evitar carrera` en una sección crítica. El código evidente no
lleva comentario.

---

## Mapa de lectura del código

Las referencias son por nombre de función, no por número de línea.

| # | Función | Directiva principal | Patrón |
|---|---|---|---|
| 1 | `op1_espacios` | `parallel for reduction(+:espacios)` | Reparto + reducción |
| 2 | `op2_palabras_caracteres` | `parallel for reduction(+:palabras, caracteres)` | Reparto + dos reducciones |
| 3 | `op3_sumar_matrices` | `parallel for collapse(2)` | Reparto 2D |
| 4 | `op4_cadenas_iguales` | `parallel for reduction(+:diferencias)` | Reparto + reducción |
| 5 | `op5_ordenar` | `parallel` + `omp for` por fase | Reparto con barreras entre fases |
| 6 | `op6_palabra_en_texto` | `parallel for reduction(+:encontrada)` | Reparto + reducción |
| 7 | `op7_interseccion` | `parallel for` + `critical` | Reparto + exclusión mutua |
| 8 | `op8_raices_cuadraticas` | `parallel sections` (dos bloques, dos veces) | Descomposición funcional |
| 9 | `op9_contar_palabra` | `parallel for reduction(+:apariciones)` | Reparto + reducción |
| 10 | `op10_sumatoria` | `parallel for reduction(+:suma) schedule(static)` | Reparto + reducción estática |

---

## Las diez operaciones

Cada ficha sigue la misma plantilla: enunciado, idea, paralelización,
corrección y ejemplo.

---

### Operación 1 — Espacios en blanco en una cadena

**Enunciado.** Contar cuántos espacios en blanco tiene una cadena.

**Idea.** Leer la cadena con `leer_linea` y recorrerla carácter a carácter; cada
carácter que cumpla `isspace()` incrementa un contador. Se cuenta todo lo que C
considera espacio en blanco (`' '`, `'\t'`, `'\v'`, `'\f'`, `'\r'`), no solo el
espacio literal. El salto de línea se quita al leer, así que no se cuenta.

**Paralelización.**

```c
size_t espacios = 0;

#pragma omp parallel for reduction(+:espacios)
for (size_t i = 0; i < n; i++) {
    if (isspace((unsigned char)cadena[i]))
        espacios++;
}
```

Sin `reduction`, todos los hilos harían `espacios++` sobre la misma variable y
se perderían incrementos (*condición de carrera*). La cláusula crea una copia
privada por hilo inicializada a 0 y al final las suma. Cada hilo solo **lee**
`cadena`.

**Ejemplo.**

```text
Ingresa la cadena: hola	mundo  openmp
Numero de espacios en blanco: 3
```

**Puntos clave.**

- El *cast* a `unsigned char` es necesario: `isspace` solo está definida para
  valores representables como `unsigned char` o `EOF`.
- Paralelizar una cadena corta no compensa el coste de crear hilos. Se hace
  porque lo pide la práctica y porque el patrón escala.

---

### Operación 2 — Palabras y caracteres de un archivo

**Enunciado.** Contar las palabras y los caracteres de un archivo de texto.

**Idea.** Cargar el archivo en un arreglo `static` y recorrerlo una sola vez,
calculando ambos conteos en el mismo bucle.

- **Palabra**: comienza en una posición no-espacio cuyo carácter anterior es
  espacio (o que es la posición 0). Varios espacios seguidos no generan palabras
  vacías.
- **Carácter**: se cuenta un byte como carácter si no es de continuación UTF-8.
  Así `á`, `ñ`, `ü` cuentan como **uno**, aunque ocupen dos bytes. Es una
  aproximación por puntos de código. El archivo se abre en modo binario (`"rb"`).

**Paralelización.**

```c
size_t palabras = 0;
size_t caracteres = 0;

#pragma omp parallel for reduction(+:palabras, caracteres)
for (size_t i = 0; i < n; i++) {
    unsigned char c = (unsigned char)contenido[i];

    if ((c & 0xC0) != 0x80)
        caracteres++;

    int es_caracter_palabra = !isspace(c);
    int anterior_es_espacio = (i == 0) || isspace((unsigned char)contenido[i - 1]);
    if (es_caracter_palabra && anterior_es_espacio)
        palabras++;
}
```

La máscara `(c & 0xC0) != 0x80` detecta los bytes `10xxxxxx`, que son de
continuación en UTF-8. Leer `contenido[i-1]` **no es una dependencia entre
iteraciones**: ese byte nunca se escribe, así que no hay carrera. Dos reducciones
independientes conviven en la misma directiva.

**Ejemplo.** Archivo con `acción rápida\n`:

```text
Numero de palabras: 2
Numero de caracteres: 14
```

**Puntos clave.**

- Con `strlen` se contarían bytes: 15 en el ejemplo.
- Tabuladores y saltos de línea también separan palabras (`isspace`).
- Si el archivo pasa de 8191 bytes se avisa y se procesa solo ese trozo.

---

### Operación 3 — Suma de matrices cuadradas

**Enunciado.** Sumar dos matrices cuadradas de enteros.

**Idea.** Tres arreglos `static` de `MAX_DIM * MAX_DIM` elementos `long long` y
calcular `C[i][j] = A[i][j] + B[i][j]`. La indexación plana `i*n + j` evita
arreglos de punteros: una sola dirección base y memoria contigua. Con `n <= 10`
se imprimen A, B y A+B recorriendo un arreglo de punteros a esas tres matrices.

**Paralelización.**

```c
#pragma omp parallel for collapse(2)
for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
        size_t idx = (size_t)i * n + j;
        C[idx] = A[idx] + B[idx];
    }
}
```

`collapse(2)` fusiona los dos bucles en un espacio de `n*n` iteraciones y lo
reparte entre los hilos. Cada iteración escribe una posición distinta de `C` y
`A`, `B` solo se leen: no hay carrera.

**Ejemplo.** `n=2`, `A=[1 2; 3 4]`, `B=[5 6; 7 8]`:

```text
Matriz A + B:
     6      8
    10     12
```

**Puntos clave.**

- `n` no puede superar `MAX_DIM` (100).
- `long long` evita desbordamientos aunque se cambien los rangos de entrada.

---

### Operación 4 — ¿Dos cadenas son iguales?

**Enunciado.** Revisar si dos cadenas son iguales.

**Idea.** Leer ambas cadenas con `leer_linea`, comparar longitudes y, si
coinciden, contar las posiciones distintas. La comparación es exacta: distingue
mayúsculas y no ignora espacios. Si las longitudes difieren se responde sin
recorrer nada.

**Paralelización.**

```c
size_t diferencias = 0;

#pragma omp parallel for reduction(+:diferencias)
for (size_t i = 0; i < n1; i++) {
    if (s1[i] != s2[i])
        diferencias++;
}
```

Solo se entra al bucle si `n1 == n2`. Dos cadenas de la misma longitud son
iguales si y solo si coinciden en todas las posiciones, así que
`diferencias == 0` equivale a igualdad.

**Ejemplo.**

```text
Ingresa la primera cadena: abc
Ingresa la segunda cadena: abd
Las cadenas NO son iguales (difieren en 1 posicion(es)).
```

**Puntos clave.**

- No se puede hacer `break` al encontrar la primera diferencia: las iteraciones
  están repartidas entre hilos.
- El resultado coincide con `strcmp(s1, s2) == 0`.

---

### Operación 5 — Ordenar enteros

**Enunciado.** Ordenar un conjunto de números enteros.

**Idea.** Ordenamiento par-impar (*odd-even transposition sort*):

- En la fase par se comparan los pares `(0,1), (2,3), (4,5)…`
- En la fase impar se comparan `(1,2), (3,4), (5,6)…`
- Se repiten `n` fases alternando el inicio (`0` o `1`).

En cada fase los pares son disjuntos. El orden es ascendente y solo se imprime
el arreglo si `n <= 50`.

**Paralelización.**

```c
#pragma omp parallel
{
    for (int fase = 0; fase < n; fase++) {
        int inicio = fase % 2;
        #pragma omp for
        for (int i = inicio; i < n - 1; i += 2) {
            if (v[i] > v[i + 1]) {
                int tmp = v[i];
                v[i] = v[i + 1];
                v[i + 1] = tmp;
            }
        }
    }
}
```

Hay **una sola región paralela** para todas las fases. Cada `omp for` reparte
las comparaciones y tiene una barrera implícita al final; esa barrera impide que
una fase empiece mientras otra todavía está intercambiando. `inicio` se calcula
igual en todos los hilos, así que los límites del `omp for` coinciden.

**Ejemplo.**

```text
Cuantos numeros enteros? 5
Ingresa 5 enteros separados por espacio:
5 1 4 2 3
Original: [5, 1, 4, 2, 3]
Ordenado: [1, 2, 3, 4, 5]
```

**Puntos clave.**

- Las fases par e impar no pueden ir juntas: comparten posiciones.
- El coste es `O(n²)`; es didáctico, no compite con `qsort`.
- `n` no puede superar `MAX_N` (100000).

---

### Operación 6 — ¿La palabra está en el texto?

**Enunciado.** Revisar si una palabra se encuentra en un texto.

**Idea.** Leer el texto (teclado o archivo) y la palabra. Normalizar la palabra
con `normalizar`: recorta puntuación en los extremos y la pasa a minúsculas en
el mismo buffer. Después se prueba la coincidencia en cada posición del texto.

- Comparación **sin distinción de mayúsculas**: se usa `tolower` sobre cada
  carácter del texto.
- **Palabra completa**: el carácter anterior y el posterior deben ser
  separadores (`!isalnum`), o bien estar en los extremos del texto.
- Por eso `pen` no cuenta dentro de `openmp`, pero `(OpenMP).` sí coincide con
  `openmp`.
- La palabra que se muestra en el mensaje es la ya normalizada.

**Paralelización.**

```c
size_t tope = (plen <= len) ? len - plen + 1 : 0;
int encontrada = 0;

#pragma omp parallel for reduction(+:encontrada)
for (size_t i = 0; i < tope; i++) {
    if (i > 0) {
        if (isalnum((unsigned char)texto[i - 1])) continue;
    }
    if (i + plen < len) {
        if (isalnum((unsigned char)texto[i + plen])) continue;
    }

    int coincide = 1;
    for (size_t k = 0; k < plen; k++) {
        if (tolower((unsigned char)texto[i + k]) != palabra[k]) {
            coincide = 0;
            break;
        }
    }
    if (coincide) encontrada = 1;
}
```

Los dos `if` del principio descartan la posición si el carácter de al lado no
es separador: si está dentro del texto se mira, y si está en un extremo se
acepta directamente, así que nunca se lee fuera del arreglo. Después `coincide`
empieza en 1 y se pone a 0 en cuanto falla una letra.

Cada posición `i` es independiente y el texto solo se lee. La reducción cuenta
coincidencias y basta con comprobar si hay al menos una. No se puede abandonar
el bucle al encontrarla, pero eso no cambia la respuesta.

**Ejemplo.**

```text
Ingresa el texto: Hola, OpenMP! openmp es genial; (OpenMP).
Ingresa la palabra a buscar: openmp
La palabra 'openmp' SI se encuentra en el texto.
```

---

### Operación 7 — Intersección de conjuntos

**Enunciado.** Obtener la intersección de dos conjuntos de enteros.

**Idea.** Para cada elemento de `A`, comprobar si aparece en `B`; si aparece y
no estaba registrado, añadirlo al resultado. El resultado es un **conjunto**:
sin repetidos aunque `A` o `B` los tengan. El orden de salida no está
garantizado.

**Paralelización.**

```c
int count = 0;

#pragma omp parallel for
for (int i = 0; i < n; i++) {
    int val = A[i];
    int en_b = 0;
    for (int j = 0; j < m; j++) {
        if (B[j] == val) { en_b = 1; break; }
    }
    if (en_b) {
        #pragma omp critical
        {
            int duplicado = 0;
            for (int c = 0; c < count; c++) {
                if (interseccion[c] == val) { duplicado = 1; break; }
            }
            if (!duplicado)
                interseccion[count++] = val;
        }
    }
}
```

El trabajo de búsqueda se reparte entre los hilos. `A` y `B` son de solo lectura.
Solo la inserción en `interseccion` —compartida— se protege con `critical`, que
garantiza exclusión mutua sobre `count` y el arreglo. El chequeo de duplicados
ocurre dentro de esa zona, así que el resultado siempre es un conjunto.

**Ejemplo.** `A={1,2,3,4,5}`, `B={3,4,5,6,7}`:

```text
Interseccion de A y B (3 elementos):
[5, 4, 3]
```

El contenido es correcto; el orden cambia entre ejecuciones porque los hilos
entran en `critical` en orden no determinista.

**Puntos clave.**

- Proteger todo el bucle serializaría el cómputo; aquí solo se protege la fusión.
- La complejidad es `O(n·m)` por la búsqueda lineal.

---

### Operación 8 — Raíces reales de una cuadrática

**Enunciado.** Calcular las raíces reales de `ax² + bx + c = 0`.

**Idea.** Aplicar la fórmula general:

```text
D = b² - 4ac
x₁ = (-b + √D) / (2a)
x₂ = (-b - √D) / (2a)
```

Si `a == 0` no es cuadrática; si `D < 0` no hay raíces reales; si `D == 0` se
imprimen dos raíces iguales. Coeficientes `double`, salida con cuatro decimales.

**Paralelización.** Descomposición funcional: hay dos tareas distintas, no un
conjunto de iteraciones que repartir.

```c
#pragma omp parallel sections
{
    #pragma omp section
    { b2 = b * b; }

    #pragma omp section
    { ac4 = 4.0 * a * c; }
}

D = b2 - ac4;
```

Y, si `D >= 0`, otras dos secciones calculan `x1` y `x2`. Al final de cada bloque
hay una barrera implícita: `D` no se calcula hasta que ambas secciones terminan.
Cada sección escribe una variable distinta. `r = sqrt(D)` va en serie porque
ambas raíces lo necesitan.

**Ejemplo.**

```text
Coeficiente a: 1
Coeficiente b: -3
Coeficiente c: 2
Discriminante D = 1.0000
Raiz 1 = 2.0000
Raiz 2 = 1.0000
```

**Puntos clave.**

- Sin la barrera implícita, `D` podría leer `b2` o `ac4` todavía a 0.
- El trabajo por sección es una multiplicación: crear hilos cuesta mucho más.
  Se usa para practicar `sections`.

---

### Operación 9 — Apariciones de una palabra en un texto

**Enunciado.** Revisar cuántas veces aparece una palabra en un texto.

**Idea.** Igual preparación que la operación 6: texto, `normalizar` y búsqueda
por palabra completa sin distinción de mayúsculas. Solo cambia el acumulador y
el mensaje.

**Paralelización.**

```c
size_t apariciones = 0;

#pragma omp parallel for reduction(+:apariciones)
for (size_t i = 0; i < tope; i++) {
    if (i > 0) {
        if (isalnum((unsigned char)texto[i - 1])) continue;
    }
    if (i + plen < len) {
        if (isalnum((unsigned char)texto[i + plen])) continue;
    }

    int coincide = 1;
    for (size_t k = 0; k < plen; k++) {
        if (tolower((unsigned char)texto[i + k]) != palabra[k]) {
            coincide = 0;
            break;
        }
    }
    if (coincide) apariciones++;
}
```

Cada hilo acumula en su copia privada las coincidencias de su rango y la
reducción las suma al final. Aquí hay que recorrer todas las posiciones: se
cuentan todas las apariciones. `size_t` evita desbordar con textos grandes.

**Ejemplo.** Texto `Hola, OpenMP! openmp es genial; (OpenMP).`, objetivo
`openmp`:

```text
La palabra 'openmp' aparece 3 veces.
```

---

### Operación 10 — Sumatoria de n enteros

**Enunciado.** Calcular la sumatoria de `n` números enteros.

**Idea.** Leer `n`, llenar un arreglo `static` (teclado o aleatorio) y sumar sus
elementos. El acumulador es `long long` para reducir el riesgo de
desbordamiento.

**Paralelización.**

```c
#pragma omp parallel for reduction(+:suma) schedule(static)
for (int i = 0; i < n; i++)
    suma += (long long)v[i];
```

`reduction(+:suma)` da a cada hilo su propio acumulador y los combina al final,
evitando la carrera de `suma += ...`. `schedule(static)` reparte bloques
contiguos de tamaño similar: todas las iteraciones cuestan lo mismo. El arreglo
se imprime en un bucle secuencial para que la salida salga en orden y el tiempo
medido corresponda a la suma.

**Ejemplo.**

```text
Cuantos numeros enteros? 5
Ingresa 5 enteros separados por espacio:
1 2 3 4 5
arreglo: [1, 2, 3, 4, 5, ]
Sumatoria = 15
```

**Puntos clave.**

- Sin `reduction`, los incrementos se pierden y el resultado cambia entre
  ejecuciones.
- La generación aleatoria es secuencial: `rand()` tiene estado global.
- `n` no puede superar `MAX_N` (100000).

---

## Resumen de directivas OpenMP usadas

| Operación | Directiva / cláusula | Qué se comparte y qué no |
|---|---|---|
| 1 | `parallel for reduction(+:espacios)` | `cadena` compartida (solo lectura); contador privado. |
| 2 | `parallel for reduction(+:palabras, caracteres)` | `contenido` compartido; dos contadores privados. |
| 3 | `parallel for collapse(2)` | `A`, `B` leídas; cada hilo escribe posiciones distintas de `C`. |
| 4 | `parallel for reduction(+:diferencias)` | Ambas cadenas de solo lectura. |
| 5 | `parallel` + `omp for` por fase | `v` compartido; posiciones disjuntas por fase; barrera entre fases. |
| 6 | `parallel for reduction(+:encontrada)` | Texto y palabra de solo lectura; contador privado. |
| 7 | `parallel for` + `critical` | `A`, `B` leídas; fusión protegida de `interseccion`/`count`. |
| 8 | `parallel sections` ×2 | Cada sección escribe su propia variable; barrera implícita. |
| 9 | `parallel for reduction(+:apariciones)` | Igual que 6, contando en vez de comprobando. |
| 10 | `parallel for reduction(+:suma) schedule(static)` | `v` de solo lectura; acumulador privado. |

Clasificación por mecanismo:

- **Reparto de iteraciones (`for`)**: 1, 2, 3, 4, 5, 6, 7, 9, 10.
- **Descomposición funcional (`sections`)**: 8.
- **Sincronización explícita (`critical`)**: 7.
- **Reducciones**: 1, 2, 4, 6, 9, 10.
- **Barreras implícitas**: al final de cada `for`/`sections`; en la operación 5
  son las que hacen correcto el algoritmo.

---

## Glosario OpenMP

| Concepto | Definición breve |
|---|---|
| **Región paralela** (`#pragma omp parallel`) | Bloque que ejecutan todos los hilos. Fuera de ella el programa es secuencial. |
| **Reparto de trabajo** (`#pragma omp for`) | Divide las iteraciones de un bucle entre los hilos del equipo actual. |
| **`parallel for`** | Abreviatura de una región paralela con un único `for` repartido. |
| **Barrera** | Punto donde todos los hilos esperan al último. Al final de `for`/`sections` es implícita. |
| **Reducción** (`reduction(op:var)`) | Cada hilo opera sobre una copia privada iniciada con el elemento neutro; al final se combinan con `op`. |
| **Variable compartida** | Visible para todos los hilos; es el valor por defecto de lo declarado fuera de la región. |
| **Variable privada** | Cada hilo tiene su copia: se declara dentro de la región o con `private`/`firstprivate`/`lastprivate`. |
| **Sección crítica** (`#pragma omp critical`) | Exclusión mutua: solo un hilo la ejecuta a la vez. |
| **`sections`** | Reparte bloques de código distintos, no iteraciones. |
| **`collapse(n)`** | Fusiona `n` bucles anidados en un solo espacio de iteraciones. |
| **`schedule(static/dynamic/guided)`** | Cómo se asignan las iteraciones: bloques fijos, bajo demanda o decrecientes. |
| **Condición de carrera** | Dos hilos acceden a la misma variable y al menos uno escribe, sin sincronización. |
| **Ley de Amdahl** | La ganancia máxima está limitada por la fracción secuencial del programa. |
| **`OMP_NUM_THREADS`** | Variable de entorno que fija el número de hilos por defecto. |

---

## Complejidad y Ley de Amdahl

| # | Trabajo paralelizable | Parte secuencial | Complejidad |
|---|---|---|---|
| 1 | Recorrido de la cadena | Lectura de la línea | `O(n)` |
| 2 | Recorrido del archivo | Lectura del archivo | `O(n)` |
| 3 | Suma de `n²` elementos | Lectura e impresión | `O(n²)` |
| 4 | Comparación de `n` posiciones | Lectura de cadenas | `O(n)` |
| 5 | Comparaciones de cada fase | `n` fases con barreras | `O(n²)` peor caso |
| 6 | Búsqueda en cada posición | Lectura y normalización | `O(T·L)` |
| 7 | Búsqueda de cada elemento de `A` en `B` | Inserción protegida | `O(n·m)` |
| 8 | Cálculo de `b²` y `4ac` | Lectura y `sqrt` | `O(1)` |
| 9 | Conteo en cada posición | Lectura y normalización | `O(T·L)` |
| 10 | Suma de `n` elementos | Generación de datos | `O(n)` |

`T` es la longitud del texto y `L` la de la palabra buscada.

Ley de Amdahl:

```text
S(p) = 1 / ((1 - f) + f / p)
```

Si el 90 % del tiempo es paralelizable, el máximo teórico con infinitos hilos
es `10×`. En este programa además:

- La lectura del texto y la normalización de la palabra son secuenciales.
- La impresión por consola es secuencial.
- Las operaciones 1, 4 y 8 tienen muy poco trabajo por iteración.

**Conclusión práctica:** el programa está pensado para demostrar OpenMP. Las
operaciones 2, 5, 6, 7, 9 y 10 son las que tienen sentido medir con tamaños
grandes; las demás son ejercicios de corrección.

---

## Cómo medir el rendimiento

Compilar con optimizaciones (el `Makefile` ya usa `-O2`) y comparar con distinto
número de hilos:

```bash
OMP_NUM_THREADS=1 ./programa
OMP_NUM_THREADS=4 ./programa
```

Para medir tiempo de forma objetiva, redirigir la salida, porque imprimir
domina el tiempo:

```bash
for t in 1 2 4 8; do
    printf '5\n20000\n2\n0\n' | OMP_NUM_THREADS=$t /usr/bin/time -f "$t hilos: %e s" ./programa > /dev/null
done
```

La suma debe dar el mismo valor con cualquier número de hilos; si cambia, hay
una condición de carrera. El *speedup* se satura porque el bucle está limitado
por el ancho de banda de memoria: sumar es casi gratis, traer los datos a la
CPU no.

> **Moraleja:** medir siempre con el mismo `n`, los mismos datos y la salida
> descartada.

---

## Trampas y errores comunes

1. **Olvidar `reduction` en un acumulador.** `suma += v[i]` con `suma`
   compartida produce incrementos perdidos. Síntoma: el resultado cambia entre
   ejecuciones.
2. **Creer que `parallel for` sirve para cualquier bucle.** No vale con
   `break`, `return` ni con dependencias entre iteraciones.
3. **Variables declaradas antes de la región.** Por defecto son compartidas.
   Si cada hilo necesita su copia, hay que declararla dentro o usar
   `private`/`firstprivate`.
4. **Escribir en el mismo índice desde varios hilos.** Es carrera aunque el
   valor sea el mismo: por eso la operación 7 protege la inserción.
5. **Confundir barrera implícita con exclusión mutua.** La barrera ordena la
   ejecución; lo que se escribe necesita además `critical` o una reducción.
6. **Olvidar los bordes en la operación 6 o 9.** Sin `!isalnum`, `pen` contaría
   dentro de `openmp`.
7. **Olvidar `tolower` en la comparación.** `OpenMP` y `openmp` son cadenas
   distintas para `strcmp`.
8. **Pasarse de los límites.** El programa avisa, pero el trabajo se hace solo
   con lo que cabe en el arreglo.
9. **`rand()` dentro de una región paralela.** No es thread-safe; la
   generación se hace en serie.
10. **Mezclar impresión y cómputo en el mismo bucle paralelo.** Desordena la
    salida y domina el tiempo; la operación 10 imprime en serie.

---

## Pruebas realizadas

Todas las pruebas se ejecutaron con el binario generado por `make` y entradas
redirigidas.

| Caso | Entrada | Resultado |
|---|---|---|
| Op. 1 | `hola<TAB>mundo  openmp` | `Numero de espacios en blanco: 3` |
| Op. 2 | archivo `acción rápida\n` | 2 palabras, 14 caracteres |
| Op. 2 | archivo de 9000 bytes | aviso de tamaño, 1 palabra, 8191 caracteres |
| Op. 3 | `n=2`, `A=[1 2; 3 4]`, `B=[5 6; 7 8]` | `A+B = [6 8; 10 12]` |
| Op. 4 | `abc` / `abc` | `Las cadenas son iguales.` |
| Op. 4 | `abc` / `abd` | `NO son iguales (difieren en 1 posicion(es))` |
| Op. 5 | `5 1 4 2 3` | `[1, 2, 3, 4, 5]` |
| Op. 6 | `Hola, OpenMP! openmp es genial; (OpenMP).` / `openmp` | `SI se encuentra` |
| Op. 6 | `Hola, OpenMP! ...` / `(openmp)` | `SI se encuentra` |
| Op. 6 | `abrenpuertas` / `pen` | `NO se encuentra` |
| Op. 6 | `Hola mundo` / `!!!` | `NO se encuentra` |
| Op. 7 | `A={1,2,3,4,5}`, `B={3,4,5,6,7}` | 3 elementos (orden variable) |
| Op. 8 | `a=1, b=-3, c=2` | `D = 1`, raíces `2` y `1` |
| Op. 8 | `a=1, b=0, c=1` | `D = -4`, `No hay raices reales.` |
| Op. 9 | texto anterior / `openmp` | `aparece 3 veces` |
| Op. 9 | `abrenpuertas` / `pen` | `aparece 0 veces` |
| Op. 10 | `1 2 3 4 5` | `arreglo: [1, 2, 3, 4, 5, ]`, `Sumatoria = 15` |
| Op. 10 | `n=20000`, aleatorio, 1 y 4 hilos | misma suma |
| Menú | opción inválida o texto | aviso y vuelta al menú |
| Menú | `0` | `Fin del programa.` |

**Comprobaciones adicionales.**

- Compila sin *warnings* con `-Wall -Wextra`.
- La suma de la operación 10 da el mismo valor con 1 y 4 hilos.
- La operación 2 cuenta acentos y `ñ` como un carácter cada uno.
- Los límites `MAX_DIM`, `MAX_N` y `MAX_TEXTO` se comprueban antes de trabajar.

---

## Preguntas de repaso

Las respuestas están plegadas para poder intentarlo primero.

1. **¿Por qué la operación 1 necesita `reduction` y la 3 no?**

   <details>
   <summary>Ver respuesta</summary>

   En la 1 todos los hilos incrementan la misma variable `espacios`. En la 3
   cada iteración escribe una posición distinta de `C`, así que no hay ningún
   dato compartido que se escriba a la vez.
   </details>

2. **¿Qué diferencia hay entre `private` y `reduction`?**

   <details>
   <summary>Ver respuesta</summary>

   `private` crea una copia por hilo que se descarta al salir (salvo
   `lastprivate`). `reduction` además inicializa la copia con el elemento
   neutro y la combina al final con la operación indicada.
   </details>

3. **En la operación 5, ¿por qué hace falta la barrera entre fases?**

   <details>
   <summary>Ver respuesta</summary>

   Un hilo podría empezar la fase siguiente mientras otro aún está
   intercambiando en la actual. El `omp for` termina con una barrera implícita
   que convierte cada fase en un paso completo.
   </details>

4. **En la operación 7, ¿por qué `critical` solo alrededor de la inserción?**

   <details>
   <summary>Ver respuesta</summary>

   La búsqueda de cada elemento de `A` en `B` es independiente. Solo
   `interseccion` y `count` se escriben; proteger todo el bucle serializaría el
   cómputo.
   </details>

5. **¿Para qué sirven los bordes con `!isalnum` en las operaciones 6 y 9?**

   <details>
   <summary>Ver respuesta</summary>

   Para exigir que la coincidencia empiece y termine en un límite de palabra.
   Así `pen` no se cuenta dentro de `openmp`, pero `(openmp).` sí coincide.
   </details>

6. **¿Por qué se normaliza la palabra antes del bucle paralelo?**

   <details>
   <summary>Ver respuesta</summary>

   Para no repetir `tolower` sobre la palabra en cada hilo y cada posición. Se
   hace una vez, en sitio, con `normalizar`.
   </details>

7. **¿Por qué la operación 8 usa `sections` y no `for`?**

   <details>
   <summary>Ver respuesta</summary>

   No hay iteraciones homogéneas que repartir, sino dos tareas diferentes e
   independientes. `sections` es paralelismo funcional; `for` es paralelismo de
   datos.
   </details>

8. **¿Qué pasaría si la operación 10 usara `int` como acumulador?**

   <details>
   <summary>Ver respuesta</summary>

   Podría desbordarse con `n` grande: el desbordamiento de enteros con signo es
   comportamiento indefinido en C. Por eso se usa `long long`.
   </details>

9. **¿Qué diferencia hay entre la operación 6 y la 9?**

   <details>
   <summary>Ver respuesta</summary>

   Comparten lectura, normalización y búsqueda. Solo cambia el acumulador:
   pregunta si hay al menos una coincidencia frente a contarlas todas.
   </details>

10. **¿Qué pasa si un archivo supera `MAX_TEXTO`?**

    <details>
    <summary>Ver respuesta</summary>

    Se imprime un aviso y se procesan solo los primeros 8191 bytes, que es lo
    que cabe dejando sitio para el `'\0'`.
    </details>

---

## Checklist: sé explicar esto

Antes de dar la práctica por estudiada, cada integrante debería poder explicar
sin mirar el código:

- [ ] Qué hace `#pragma omp parallel for` y qué reparte exactamente.
- [ ] Qué es una condición de carrera y cómo la evita `reduction`.
- [ ] La diferencia entre `private`, `shared` y `reduction`.
- [ ] Qué es una barrera implícita y por qué es crítica en la operación 5.
- [ ] Por qué `sections` conviene en la operación 8 y no en la 3.
- [ ] Por qué la operación 7 usa `critical` solo en la fusión del resultado.
- [ ] Cómo se logra la búsqueda por palabra completa con `!isalnum`.
- [ ] Por qué `normalizar` se llama una sola vez y fuera del bucle paralelo.
- [ ] Cómo se mide el *speedup* y por qué no es lineal.
- [ ] Qué parte de cada operación queda secuencial y cómo limita la Ley de
      Amdahl.
- [ ] Para qué sirve cada uno de los cinco límites `MAX_*`.
