# Práctica 6 — Menú de operaciones con OpenMP

Programa en C con un menú de consola que agrupa diez operaciones. Por ahora
están implementadas la **8** (raíces reales de una ecuación cuadrática) y la
**10** (sumatoria de `n` números enteros); las demás muestran un mensaje de
"aún no implementada" para poder agregarlas después sin cambiar el menú.

## Archivos

| Archivo | Descripción |
|---|---|
| `programa.c` | Menú, operaciones 8 y 10, y stubs de las operaciones pendientes. |
| `Makefile` | Compilación con `gcc` y OpenMP. |

## Requisitos

- `gcc` con soporte de OpenMP (`libgomp`).
- Biblioteca matemática (`-lm`).

## Compilación y ejecución

```bash
cd practica6
make          # gcc -O2 -Wall -Wextra -fopenmp programa.c -o programa -lm
./programa
```

También se puede compilar directamente:

```bash
gcc -O2 -Wall -Wextra -fopenmp programa.c -o programa -lm
```

Para limpiar el ejecutable:

```bash
make clean
```

## Menú

```
Selecciona la operacion:
[1]  Contar el numero de espacios en blanco que tiene una cadena.
[2]  Contar el numero de palabras y el numero de caracteres de un archivo de texto.
[3]  Sumar dos matrices cuadradas de numeros enteros.
[4]  Revisar si dos cadenas son iguales.
[5]  Ordenar un conjunto de numeros enteros.
[6]  Revisar si una palabra se encuentra en un texto.
[7]  Obtener la interseccion de dos conjuntos de enteros.
[8]  Calcular las raices reales de una ecuacion cuadratica.
[9]  Revisar cuantas veces aparece una palabra en un texto.
[10] Calcular la sumatoria de n numeros enteros.
[0]  Salir.
```

El programa queda en un bucle hasta que se elige `0`. Cada operación se
encapsula en una función `static void` y se invoca desde un `switch`; las
operaciones pendientes ya tienen su función y su caso en el menú.

## Operación 8: raíces reales

Se piden los coeficientes `a`, `b` y `c`. Si `a == 0` se avisa que no es una
ecuación cuadrática. El cálculo se divide en dos fases paralelas:

1. `b²` y `4ac` son independientes, así que se calculan con
   `#pragma omp parallel sections`.
2. Al terminar las secciones hay una barrera implícita; después se obtiene el
   discriminante `D = b² - 4ac`.
3. Si `D >= 0`, `x1` y `x2` también son independientes y se calculan con otro
   bloque `parallel sections`.

```c
#pragma omp parallel sections
{
    #pragma omp section
    { b2 = b * b; }

    #pragma omp section
    { ac4 = 4.0 * a * c; }
}
```

Cada sección escribe una variable distinta, por lo que no se necesita
exclusión mutua. El resultado se imprime con cuatro decimales.

## Operación 10: sumatoria de `n` enteros

Se pide `n`, se reserva memoria con `malloc` y se elige cómo capturar los
números:

1. Escribirlos por teclado.
2. Generarlos aleatoriamente entre `-100` y `100` con `rand()`.

La suma se hace con una reducción de OpenMP, de modo que cada hilo acumula una
parte del arreglo y al final OpenMP combina los acumulados:

```c
long long suma = 0;

#pragma omp parallel for reduction(+:suma) schedule(static)
for (int i = 0; i < n; i++) {
    suma += (long long)v[i];
}
```

Se usa `long long` para reducir el riesgo de desbordamiento. La entrada por
teclado y la generación aleatoria permiten probar desde pocos números hasta
millones de elementos.

## Pruebas realizadas

| Caso | Resultado |
|---|---|
| Opción 8 con `a=1, b=-3, c=2` | `D = 1`, raíces `2` y `1` |
| Opción 8 con `a=0` | "No es una ecuacion cuadratica" |
| Opción 8 con `D = -4` | "No hay raices reales" |
| Opción 10 por teclado con `1 2 3 4 5` | `Sumatoria = 15` |
| Opción 10 aleatoria con `n = 1 000 000` | Ejecuta correctamente con 8 hilos |
| Opción no implementada / inválida | Muestra el aviso y vuelve al menú |

## Comparar el efecto de OpenMP

La operación 10 es la más adecuada para observar el paralelismo. Con la
generación aleatoria y un `n` grande se puede comparar el tiempo:

```bash
OMP_NUM_THREADS=1 time ./programa
OMP_NUM_THREADS=4 time ./programa
```

Al elegir la opción 10, usar un valor grande de `n` (por ejemplo `10000000`) y
la opción `2` (números aleatorios).

## Pendientes

Implementar las operaciones 1–7 y 9 sustituyendo los stubs correspondientes en
`programa.c`. El menú y el flujo del programa ya están preparados para ello.
