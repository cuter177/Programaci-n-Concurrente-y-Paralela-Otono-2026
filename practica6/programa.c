#include <ctype.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINEA 1024
#define MAX_TEXTO 8192
#define MAX_PALABRA 128
#define MAX_N 100000
#define MAX_DIM 100

/* ---------- Prototipos de las 10 operaciones ---------- */
static void op1_espacios(void);
static void op2_palabras_caracteres(void);
static void op3_sumar_matrices(void);
static void op4_cadenas_iguales(void);
static void op5_ordenar(void);
static void op6_palabra_en_texto(void);
static void op7_interseccion(void);
static void op8_raices_cuadraticas(void);
static void op9_contar_palabra(void);
static void op10_sumatoria(void);

/* ---------- Utilidades ---------- */
static void imprimir_menu(void);
static void limpiar_entrada(void);
static void leer_linea(char *buf, size_t cap);
static int leer_entero(const char *prompt, int *out, int positivo);
static int leer_archivo(const char *ruta, char *buf, size_t cap, size_t *len);
static void normalizar(char *s);

static void imprimir_menu(void) {
    printf("\n==================================================\n");
    printf("Selecciona la operacion:\n");
    printf("[0]  Salir.\n");
    printf("[1]  Contar el numero de espacios en blanco que tiene una cadena.\n");
    printf("[2]  Contar el numero de palabras y el numero de caracteres de un archivo de texto.\n");
    printf("[3]  Sumar dos matrices cuadradas de numeros enteros.\n");
    printf("[4]  Revisar si dos cadenas son iguales.\n");
    printf("[5]  Ordenar un conjunto de numeros enteros.\n");
    printf("[6]  Revisar si una palabra se encuentra en un texto.\n");
    printf("[7]  Obtener la interseccion de dos conjuntos de enteros.\n");
    printf("[8]  Calcular las raices reales de una ecuacion cuadratica.\n");
    printf("[9]  Revisar cuantas veces aparece una palabra en un texto.\n");
    printf("[10] Calcular la sumatoria de n numeros enteros.\n");
    printf("Opcion: ");
}

static void limpiar_entrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

static void leer_linea(char *buf, size_t cap) {
    if (!fgets(buf, (int)cap, stdin)) {
        buf[0] = '\0';
        return;
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';  //quitar salto de linea
}

/* con positivo=1 exige un valor mayor que cero */
static int leer_entero(const char *prompt, int *out, int positivo) {
    printf("%s", prompt);
    if (scanf("%d", out) != 1) {
        printf("Entrada invalida.\n");
        limpiar_entrada();
        return 0;
    }
    limpiar_entrada();
    if (positivo && *out <= 0) {
        printf("El valor debe ser un entero positivo.\n");
        return 0;
    }
    return 1;
}

static int leer_archivo(const char *ruta, char *buf, size_t cap, size_t *len) {
    FILE *f = fopen(ruta, "rb");
    if (!f) {
        return 0;
    }

    size_t total = 0;
    if (fseek(f, 0, SEEK_END) == 0) {
        long tam = ftell(f);  //tamano del archivo
        if (tam > 0) total = (size_t)tam;
        rewind(f);
    }

    size_t leido = fread(buf, 1, cap - 1, f);  //leer hasta el tope
    fclose(f);
    buf[leido] = '\0';
    *len = leido;

    if (total > cap - 1) {
        printf("El archivo es demasiado grande; se procesan solo los primeros %zu bytes.\n", cap - 1);
    }
    return 1;
}

static void normalizar(char *s) {
    size_t n = strlen(s);
    size_t i = 0;
    size_t j = n;

    while (i < j && ispunct((unsigned char)s[i])) i++;      //recortar inicio
    while (j > i && ispunct((unsigned char)s[j - 1])) j--;  //recortar final

    size_t out = 0;
    for (size_t k = i; k < j; k++) {
        s[out++] = (char)tolower((unsigned char)s[k]);  //convertir a minusculas
    }
    s[out] = '\0';
}

/* ---------- main: bucle del menu ---------- */
int main(void) {
    int opcion;

    for (;;) {
        imprimir_menu();
        if (scanf("%d", &opcion) != 1) {
            printf("Entrada invalida.\n");
            limpiar_entrada();
            continue;
        }
        limpiar_entrada();

        if (opcion == 0) {
            printf("Fin del programa.\n");
            break;
        }

        switch (opcion) {
            case 1:  op1_espacios();            break;
            case 2:  op2_palabras_caracteres(); break;
            case 3:  op3_sumar_matrices();      break;
            case 4:  op4_cadenas_iguales();     break;
            case 5:  op5_ordenar();             break;
            case 6:  op6_palabra_en_texto();    break;
            case 7:  op7_interseccion();        break;
            case 8:  op8_raices_cuadraticas();  break;
            case 9:  op9_contar_palabra();      break;
            case 10: op10_sumatoria();          break;
            default: printf("Opcion no valida.\n"); break;
        }
    }
    return 0;
}

/* ---------- Operacion 1: espacios en blanco en una cadena ---------- */
static void op1_espacios(void) {
    char cadena[MAX_LINEA];

    printf("Ingresa la cadena: ");
    leer_linea(cadena, sizeof cadena);

    size_t n = strlen(cadena);
    size_t espacios = 0;

    #pragma omp parallel for reduction(+:espacios)
    for (size_t i = 0; i < n; i++) {
        if (isspace((unsigned char)cadena[i]))  //casteo para isspace
            espacios++;
    }

    printf("Numero de espacios en blanco: %zu\n", espacios);
}

/* ---------- Operacion 2: palabras y caracteres en un archivo ---------- */
static void op2_palabras_caracteres(void) {
    static char contenido[MAX_TEXTO];
    char ruta[MAX_LINEA];

    printf("Ingresa la ruta del archivo de texto: ");
    leer_linea(ruta, sizeof ruta);

    size_t n = 0;
    if (!leer_archivo(ruta, contenido, sizeof contenido, &n)) {
        printf("Error: no se pudo abrir el archivo '%s'.\n", ruta);
        return;
    }

    size_t palabras = 0;
    size_t caracteres = 0;

    #pragma omp parallel for reduction(+:palabras, caracteres)
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)contenido[i];

        if ((c & 0xC0) != 0x80)  //no es byte de continuacion
            caracteres++;

        int es_caracter_palabra = !isspace(c);
        int anterior_es_espacio = (i == 0) || isspace((unsigned char)contenido[i - 1]);  //solo leo el anterior
        if (es_caracter_palabra && anterior_es_espacio)
            palabras++;
    }

    printf("Numero de palabras: %zu\n", palabras);
    printf("Numero de caracteres: %zu\n", caracteres);
}

/* ---------- Operacion 3: suma de dos matrices cuadradas ---------- */
static void op3_sumar_matrices(void) {
    static long long A[MAX_DIM * MAX_DIM];
    static long long B[MAX_DIM * MAX_DIM];
    static long long C[MAX_DIM * MAX_DIM];
    int n;

    if (!leer_entero("Dimension n de las matrices cuadradas (n x n): ", &n, 1)) return;
    if (n > MAX_DIM) {
        printf("n no puede superar %d.\n", MAX_DIM);
        return;
    }

    size_t total = (size_t)n * n;

    printf("Ingresa los %zu elementos de la matriz A (por filas):\n", total);
    for (size_t i = 0; i < total; i++) {
        if (scanf("%lld", &A[i]) != 1) {
            printf("Entrada invalida.\n");
            limpiar_entrada();
            return;
        }
    }

    printf("Ingresa los %zu elementos de la matriz B (por filas):\n", total);
    for (size_t i = 0; i < total; i++) {
        if (scanf("%lld", &B[i]) != 1) {
            printf("Entrada invalida.\n");
            limpiar_entrada();
            return;
        }
    }
    limpiar_entrada();

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            size_t idx = (size_t)i * n + j;  //indice plano
            C[idx] = A[idx] + B[idx];
        }
    }

    if (n > 10) {
        printf("Suma calculada (matrices de %d x %d; no se imprimen por su tamano).\n", n, n);
        return;
    }

    const long long *M[3] = { A, B, C };
    const char *titulos[3] = { "Matriz A:", "Matriz B:", "Matriz A + B:" };
    for (int m = 0; m < 3; m++) {
        printf("\n%s\n", titulos[m]);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++)
                printf("%6lld ", M[m][(size_t)i * n + j]);  //matriz plana
            printf("\n");
        }
    }
}

/* ---------- Operacion 4: revisar si dos cadenas son iguales ---------- */
static void op4_cadenas_iguales(void) {
    char s1[MAX_LINEA];
    char s2[MAX_LINEA];

    printf("Ingresa la primera cadena: ");
    leer_linea(s1, sizeof s1);
    printf("Ingresa la segunda cadena: ");
    leer_linea(s2, sizeof s2);

    size_t n1 = strlen(s1);
    size_t n2 = strlen(s2);

    if (n1 != n2) {
        printf("Las cadenas NO son iguales (longitudes distintas: %zu y %zu).\n", n1, n2);
        return;
    }

    size_t diferencias = 0;
    #pragma omp parallel for reduction(+:diferencias)
    for (size_t i = 0; i < n1; i++) {
        if (s1[i] != s2[i])
            diferencias++;
    }

    if (diferencias == 0) {
        printf("Las cadenas son iguales.\n");
    } else {
        printf("Las cadenas NO son iguales (difieren en %zu posicion(es)).\n", diferencias);
    }
}

/* ---------- Operacion 5: ordenar un conjunto de enteros ---------- */
static void op5_ordenar(void) {
    static int v[MAX_N];
    int n;

    if (!leer_entero("Cuantos numeros enteros? ", &n, 1)) return;
    if (n > MAX_N) {
        printf("n no puede superar %d.\n", MAX_N);
        return;
    }

    printf("Ingresa %d enteros separados por espacio:\n", n);
    for (int i = 0; i < n; i++) {
        if (scanf("%d", &v[i]) != 1) {
            printf("Entrada invalida.\n");
            limpiar_entrada();
            return;
        }
    }
    limpiar_entrada();

    if (n <= 50) {
        printf("Original: [");
        for (int i = 0; i < n; i++)
            printf("%d%s", v[i], (i < n - 1) ? ", " : "");
        printf("]\n");
    }

    #pragma omp parallel
    {
        for (int fase = 0; fase < n; fase++) {
            int inicio = fase % 2;  //alternar fases
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

    if (n <= 50) {
        printf("Ordenado: [");
        for (int i = 0; i < n; i++)
            printf("%d%s", v[i], (i < n - 1) ? ", " : "");
        printf("]\n");
    } else {
        printf("Arreglo de %d elementos ordenado (no se imprime por su tamano).\n", n);
    }
}

/* ---------- Operacion 6: revisar si una palabra se encuentra en un texto ---------- */
static void op6_palabra_en_texto(void) {
    static char texto[MAX_TEXTO];
    char palabra[MAX_PALABRA];
    int modo;

    if (!leer_entero("Como quieres ingresar el texto?\n"
                     "[1] Escribirlo por teclado\n"
                     "[2] Leerlo desde un archivo\n"
                     "Opcion: ", &modo, 0)) return;

    size_t len = 0;
    if (modo == 1) {
        printf("Ingresa el texto: ");
        leer_linea(texto, sizeof texto);
        len = strlen(texto);
    } else if (modo == 2) {
        char ruta[MAX_LINEA];
        printf("Ingresa la ruta del archivo: ");
        leer_linea(ruta, sizeof ruta);
        if (!leer_archivo(ruta, texto, sizeof texto, &len)) {
            printf("Error: no se pudo abrir el archivo '%s'.\n", ruta);
            return;
        }
    } else {
        printf("Opcion no valida.\n");
        return;
    }

    printf("Ingresa la palabra a buscar: ");
    leer_linea(palabra, sizeof palabra);
    normalizar(palabra);
    size_t plen = strlen(palabra);

    if (plen == 0) {
        printf("La palabra '%s' NO se encuentra en el texto.\n", palabra);
        return;
    }

    size_t tope = (plen <= len) ? len - plen + 1 : 0;
    int encontrada = 0;

    #pragma omp parallel for reduction(+:encontrada)
    for (size_t i = 0; i < tope; i++) {
        if (i > 0) {
            if (isalnum((unsigned char)texto[i - 1])) continue;  //anterior no separa
        }
        if (i + plen < len) {
            if (isalnum((unsigned char)texto[i + plen])) continue;  //siguiente no separa
        }

        int coincide = 1;
        for (size_t k = 0; k < plen; k++) {
            if (tolower((unsigned char)texto[i + k]) != palabra[k]) {  //sin mayusculas
                coincide = 0;
                break;
            }
        }
        if (coincide) encontrada = 1;
    }

    if (encontrada) {
        printf("La palabra '%s' SI se encuentra en el texto.\n", palabra);
    } else {
        printf("La palabra '%s' NO se encuentra en el texto.\n", palabra);
    }
}

/* ---------- Operacion 7: interseccion de dos conjuntos de enteros ---------- */
static void op7_interseccion(void) {
    static int A[MAX_N];
    static int B[MAX_N];
    static int interseccion[MAX_N];
    int n, m, modo;

    if (!leer_entero("Cuantos elementos tiene el conjunto A? ", &n, 1)) return;
    if (!leer_entero("Cuantos elementos tiene el conjunto B? ", &m, 1)) return;
    if (n > MAX_N || m > MAX_N) {
        printf("Los conjuntos no pueden superar %d elementos.\n", MAX_N);
        return;
    }

    if (!leer_entero("Como quieres los conjuntos?\n"
                     "[1] Escribirlos por teclado\n"
                     "[2] Generarlos aleatoriamente\n"
                     "Opcion: ", &modo, 0)) return;

    if (modo == 1) {
        printf("Ingresa los %d enteros del conjunto A:\n", n);
        for (int i = 0; i < n; i++) {
            if (scanf("%d", &A[i]) != 1) {
                printf("Entrada invalida.\n");
                limpiar_entrada();
                return;
            }
        }
        printf("Ingresa los %d enteros del conjunto B:\n", m);
        for (int j = 0; j < m; j++) {
            if (scanf("%d", &B[j]) != 1) {
                printf("Entrada invalida.\n");
                limpiar_entrada();
                return;
            }
        }
        limpiar_entrada();
    } else if (modo == 2) {
        srand((unsigned)time(NULL));
        for (int i = 0; i < n; i++)
            A[i] = rand() % 50 - 25;
        for (int j = 0; j < m; j++)
            B[j] = rand() % 50 - 25;
        printf("Se generaron conjuntos aleatorios para A y B.\n");
    } else {
        printf("Opcion no valida.\n");
        return;
    }

    int count = 0;

    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        int val = A[i];
        int en_b = 0;
        for (int j = 0; j < m; j++) {
            if (B[j] == val) {
                en_b = 1;
                break;
            }
        }
        if (en_b) {
            #pragma omp critical  //evitar carrera
            {
                int duplicado = 0;
                for (int c = 0; c < count; c++) {
                    if (interseccion[c] == val) {  //evitar duplicados
                        duplicado = 1;
                        break;
                    }
                }
                if (!duplicado)
                    interseccion[count++] = val;
            }
        }
    }

    printf("Interseccion de A y B (%d elementos):\n[", count);
    for (int i = 0; i < count; i++)
        printf("%d%s", interseccion[i], (i < count - 1) ? ", " : "");
    printf("]\n");
}

/* ---------- Operacion 8: raices reales ---------- */
static void op8_raices_cuadraticas(void) {
    double a, b, c, D, r, x1 = 0, x2 = 0;
    double b2 = 0.0, ac4 = 0.0;

    printf("Coeficiente a: ");
    scanf("%lf", &a);
    printf("Coeficiente b: ");
    scanf("%lf", &b);
    printf("Coeficiente c: ");
    scanf("%lf", &c);
    limpiar_entrada();

    if (a == 0.0) {
        printf("No es una ecuacion cuadratica (a = 0).\n");
        return;
    }

    #pragma omp parallel sections
    {
        #pragma omp section
        { b2 = b * b; }

        #pragma omp section
        { ac4 = 4.0 * a * c; }
    }

    D = b2 - ac4;  //barrera implicita
    printf("Discriminante D = %.4f\n", D);

    if (D < 0.0) {
        printf("No hay raices reales.\n");
        return;
    }

    r = sqrt(D);

    #pragma omp parallel sections
    {
        #pragma omp section
        { x1 = (-b + r) / (2.0 * a); }

        #pragma omp section
        { x2 = (-b - r) / (2.0 * a); }
    }

    printf("Raiz 1 = %.4f\n", x1);
    printf("Raiz 2 = %.4f\n", x2);
}

/* ---------- Operacion 9: apariciones de una palabra en un texto ---------- */
static void op9_contar_palabra(void) {
    static char texto[MAX_TEXTO];
    char palabra[MAX_PALABRA];
    int modo;

    if (!leer_entero("Como quieres ingresar el texto?\n"
                     "[1] Escribirlo por teclado\n"
                     "[2] Leerlo desde un archivo\n"
                     "Opcion: ", &modo, 0)) return;

    size_t len = 0;
    if (modo == 1) {
        printf("Ingresa el texto: ");
        leer_linea(texto, sizeof texto);
        len = strlen(texto);
    } else if (modo == 2) {
        char ruta[MAX_LINEA];
        printf("Ingresa la ruta del archivo: ");
        leer_linea(ruta, sizeof ruta);
        if (!leer_archivo(ruta, texto, sizeof texto, &len)) {
            printf("Error: no se pudo abrir el archivo '%s'.\n", ruta);
            return;
        }
    } else {
        printf("Opcion no valida.\n");
        return;
    }

    printf("Ingresa la palabra a buscar: ");
    leer_linea(palabra, sizeof palabra);
    normalizar(palabra);
    size_t plen = strlen(palabra);

    if (plen == 0) {
        printf("La palabra '%s' aparece 0 veces.\n", palabra);
        return;
    }

    size_t tope = (plen <= len) ? len - plen + 1 : 0;
    size_t apariciones = 0;

    #pragma omp parallel for reduction(+:apariciones)
    for (size_t i = 0; i < tope; i++) {
        if (i > 0) {
            if (isalnum((unsigned char)texto[i - 1])) continue;  //anterior no separa
        }
        if (i + plen < len) {
            if (isalnum((unsigned char)texto[i + plen])) continue;  //siguiente no separa
        }

        int coincide = 1;
        for (size_t k = 0; k < plen; k++) {
            if (tolower((unsigned char)texto[i + k]) != palabra[k]) {  //sin mayusculas
                coincide = 0;
                break;
            }
        }
        if (coincide) apariciones++;
    }

    printf("La palabra '%s' aparece %zu veces.\n", palabra, apariciones);
}

/* ---------- Operacion 10: sumatoria de n enteros ---------- */
static void op10_sumatoria(void) {
    static int v[MAX_N];
    int n, modo;
    long long suma = 0;

    if (!leer_entero("Cuantos numeros enteros? ", &n, 1)) return;
    if (n > MAX_N) {
        printf("n no puede superar %d.\n", MAX_N);
        return;
    }

    if (!leer_entero("Como quieres los numeros?\n"
                     "[1] Escribirlos por teclado\n"
                     "[2] Generarlos aleatoriamente\n"
                     "Opcion: ", &modo, 0)) return;

    if (modo == 1) {
        printf("Ingresa %d enteros separados por espacio:\n", n);
        for (int i = 0; i < n; i++) {
            if (scanf("%d", &v[i]) != 1) {
                printf("Entrada invalida.\n");
                limpiar_entrada();
                return;
            }
        }
        limpiar_entrada();
    } else if (modo == 2) {
        srand((unsigned)time(NULL));
        for (int i = 0; i < n; i++)
            v[i] = rand() % 201 - 100;
        printf("Se generaron %d enteros aleatorios entre -100 y 100.\n", n);
    } else {
        printf("Opcion no valida.\n");
        return;
    }

    printf("arreglo: [");
    for (int i = 0; i < n; i++)
        printf("%d, ", v[i]);
    printf("]\n");

    #pragma omp parallel for reduction(+:suma) schedule(static)  //bloques fijos
    for (int i = 0; i < n; i++)
        suma += (long long)v[i];

    printf("Sumatoria = %lld\n", suma);
}
