Práctica 6 — Menú de operaciones con OpenMPPrograma en C con un menú de consola que agrupa diez operaciones. Actualmente
están implementadas las operaciones 1 (espacios en blanco en una cadena),
2 (palabras y caracteres de un archivo de texto), 8 (raíces reales de
una ecuación cuadrática), 9 (apariciones de una palabra en un texto) y
10 (sumatoria de n números enteros); las demás muestran un mensaje de
"aún no implementada" para poder agregarlas después sin cambiar el menú.ArchivosArchivoDescripciónprograma.cMenú, operaciones 1, 2, 8, 9 y 10, y stubs de las operaciones pendientes.MakefileCompilación con gcc (-std=c11), OpenMP y -lm.Requisitosgcc con soporte de OpenMP (libgomp).Biblioteca matemática (-lm).Compilación y ejecucióncd practica6
make          # gcc -std=c11 -O2 -Wall -Wextra -fopenmp programa.c -o programa -lm
./programa    # o: make run
También se puede compilar directamente:gcc -std=c11 -O2 -Wall -Wextra -fopenmp programa.c -o programa -lm
Para limpiar el ejecutable:make clean
MenúSelecciona la operacion:
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
El programa queda en un bucle hasta que se elige 0. Cada operación se
encapsula en una función static void y se invoca desde un switch; las
operaciones pendientes ya tienen su función y su caso en el menú.Operación 1: espacios en blanco en una cadenaSe lee una cadena de longitud arbitraria desde teclado y se recorre en paralelo
con #pragma omp parallel for reduction(+:espacios), contando cada carácter
para el cual isspace() sea verdadero (espacios y tabuladores).#pragma omp parallel for reduction(+:espacios)
for (size_t i = 0; i < n; i++) {
    if (isspace((unsigned char)cadena[i])) {
        espacios++;
    }
}
Operación 2: palabras y caracteres en un archivo de textoSe solicita la ruta del archivo, se carga su contenido completo en memoria y se
ejecuta una doble reducción en paralelo:Palabras: se cuenta cada carácter no-espacio precedido por un espacio (o
al inicio del archivo), coincidiendo con la definición de caracteres
consecutivos sin espacio.Caracteres: se cuentan los bytes que no son de continuación UTF-8
((c & 0xC0) != 0x80), de modo que caracteres con acento o ñ cuentan como
un solo carácter.#pragma omp parallel for reduction(+:palabras, caracteres)
for (size_t i = 0; i < n; i++) {
    unsigned char c = (unsigned char)contenido[i];
    if ((c & 0xC0) != 0x80) {
        caracteres++;
    }
    int es_caracter_palabra = !isspace(c);
    int anterior_es_espacio = (i == 0) || isspace((unsigned char)contenido[i - 1]);
    if (es_caracter_palabra && anterior_es_espacio) {
        palabras++;
    }
}
Operación 8: raíces realesSe piden los coeficientes a, b y c. Si a == 0 se avisa que no es una
ecuación cuadrática. El cálculo se divide en dos fases paralelas:b² y 4ac son independientes, así que se calculan con
#pragma omp parallel sections.Al terminar las secciones hay una barrera implícita; después se obtiene el
discriminante D = b² - 4ac.Si D >= 0, x1 y x2 también son independientes y se calculan con otro
bloque parallel sections.#pragma omp parallel sections
{
    #pragma omp section
    { b2 = b * b; }

    #pragma omp section
    { ac4 = 4.0 * a * c; }
}
Cada sección escribe una variable distinta, por lo que no se necesita
exclusión mutua. El resultado se imprime con cuatro decimales.Operación 9: apariciones de una palabra en un textoPermite elegir entre escribir el texto por teclado o leerlo desde un archivo.
Cada token separado por espacios se normaliza (eliminando signos de puntuación
al inicio y al final y pasando a minúsculas) y después se compara en paralelo
contra la palabra buscada usando reduction(+:apariciones).#pragma omp parallel for reduction(+:apariciones)
for (size_t k = 0; k < num_palabras; k++) {
    if (strcmp(palabras[k], objetivo) == 0) {
        apariciones++;
    }
}
Operación 10: sumatoria de n enterosSe pide n, se reserva memoria con malloc y se elige cómo capturar los
números:Escribirlos por teclado.Generarlos aleatoriamente entre -100 y 100 con rand().La suma se hace con una reducción de OpenMP, de modo que cada hilo acumula una
parte del arreglo y al final OpenMP combina los acumulados:long long suma = 0;

#pragma omp parallel for reduction(+:suma) schedule(static)
for (int i = 0; i < n; i++) {
    suma += (long long)v[i];
}
Se usa long long para reducir el riesgo de desbordamiento. La entrada por
teclado y la generación aleatoria permiten probar desde pocos números hasta
millones de elementos.Pruebas realizadasCasoResultadoOpción 1 con "hola\tmundo  openmp"Numero de espacios en blanco: 3Opción 2 con archivo "acción rápida\n"2 palabras, 14 caracteresOpción 8 con a=1, b=-3, c=2D = 1, raíces 2 y 1Opción 8 con a=0"No es una ecuacion cuadratica"Opción 8 con D = -4"No hay raices reales"Opción 9 con "Hola, OpenMP! openmp es genial; (OpenMP)." y "openmp"La palabra 'openmp' aparece 3 veces.Opción 10 por teclado con 1 2 3 4 5Sumatoria = 15Opción 10 aleatoria con n = 1 000 000Ejecuta correctamente con 8 hilosOpción no implementada / inválidaMuestra el aviso y vuelve al menúComparar el efecto de OpenMPLa operación 10 es la más adecuada para observar el paralelismo. Con la
generación aleatoria y un n grande se puede comparar el tiempo:OMP_NUM_THREADS=1 time ./programa
OMP_NUM_THREADS=4 time ./programa
Al elegir la opción 10, usar un valor grande de n (por ejemplo 10000000) y
la opción 2 (números aleatorios).PendientesImplementar las operaciones 3–7 sustituyendo los stubs correspondientes en
programa.c. El menú y el flujo del programa ya están preparados para ello.
