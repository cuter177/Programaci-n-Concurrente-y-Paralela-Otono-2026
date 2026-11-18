#include <ctype.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
static char *leer_linea_dinamica(FILE *stream);
static char *leer_archivo_completo(const char *ruta, size_t *tam_out);
static char *normalizar_palabra(const char *inicio, size_t len);

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

static char *leer_linea_dinamica(FILE *stream) {
    size_t cap = 128;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf) {
        return NULL;
    }

    int ch;
    while ((ch = fgetc(stream)) != EOF && ch != '\n') {
        if (ch == '\r') {
            continue;
        }
        if (len + 1 >= cap) {
            size_t nueva_cap = cap * 2;
            char *tmp = realloc(buf, nueva_cap);
            if (!tmp) {
                free(buf);
                return NULL;
            }
            buf = tmp;
            cap = nueva_cap;
        }
        buf[len++] = (char)ch;
    }
    buf[len] = '\0';
    return buf;
}

static char *leer_archivo_completo(const char *ruta, size_t *tam_out) {
    FILE *f = fopen(ruta, "rb");
    if (!f) {
        return NULL;
    }

    size_t cap = 4096;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t nread;
    while ((nread = fread(buf + len, 1, cap - len - 1, f)) > 0) {
        len += nread;
        if (cap - len - 1 == 0) {
            size_t nueva_cap = cap * 2;
            char *tmp = realloc(buf, nueva_cap);
            if (!tmp) {
                free(buf);
                fclose(f);
                return NULL;
            }
            buf = tmp;
            cap = nueva_cap;
        }
    }

    fclose(f);
    buf[len] = '\0';
    *tam_out = len;
    return buf;
}

static char *normalizar_palabra(const char *inicio, size_t len) {
    size_t i = 0;
    size_t j = len;

    while (i < j && ispunct((unsigned char)inicio[i])) {
        i++;
    }
    while (j > i && ispunct((unsigned char)inicio[j - 1])) {
        j--;
    }

    size_t out_len = j - i;
    char *res = malloc(out_len + 1);
    if (!res) {
        return NULL;
    }

    for (size_t k = 0; k < out_len; k++) {
        res[k] = (char)tolower((unsigned char)inicio[i + k]);
    }
    res[out_len] = '\0';
    return res;
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
        limpiar_entrada();   /* descarta el salto de linea */

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
    printf("Ingresa la cadena: ");
    char *cadena = leer_linea_dinamica(stdin);
    if (!cadena) {
        printf("Sin memoria.\n");
        return;
    }

    size_t n = strlen(cadena);
    size_t espacios = 0;

    #pragma omp parallel for reduction(+:espacios)
    for (size_t i = 0; i < n; i++) {
        if (isspace((unsigned char)cadena[i])) {
            espacios++;
        }
    }

    printf("Numero de espacios en blanco: %zu\n", espacios);
    free(cadena);
}

/* ---------- Operacion 2: palabras y caracteres en un archivo ---------- */
static void op2_palabras_caracteres(void) {
    printf("Ingresa la ruta del archivo de texto: ");
    char *ruta = leer_linea_dinamica(stdin);
    if (!ruta) {
        printf("Sin memoria.\n");
        return;
    }

    size_t n = 0;
    char *contenido = leer_archivo_completo(ruta, &n);
    if (!contenido) {
        printf("Error: no se pudo abrir el archivo '%s'.\n", ruta);
        free(ruta);
        return;
    }

    size_t palabras = 0;
    size_t caracteres = 0;

    #pragma omp parallel for reduction(+:palabras, caracteres)
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)contenido[i];

        /* En UTF-8 los bytes de continuacion son 10xxxxxx (0x80..0xBF) */
        if ((c & 0xC0) != 0x80) {
            caracteres++;
        }

        int es_caracter_palabra = !isspace(c);
        int anterior_es_espacio = (i == 0) || isspace((unsigned char)contenido[i - 1]);
        if (es_caracter_palabra && anterior_es_espacio) {
            palabras++;
        }
    }

    printf("Numero de palabras: %zu\n", palabras);
    printf("Numero de caracteres: %zu\n", caracteres);

    free(contenido);
    free(ruta);
}

/* ---------- Operacion 3: suma de dos matrices cuadradas de enteros ---------- */
static void op3_sumar_matrices(void) {
    int n, modo;

    printf("Dimension n de las matrices cuadradas (n x n): ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("n debe ser un entero positivo.\n");
        limpiar_entrada();
        return;
    }
    limpiar_entrada();

    size_t total = (size_t)n * (size_t)n;
    int *A = malloc(total * sizeof *A);
    int *B = malloc(total * sizeof *B);
    long long *C = malloc(total * sizeof *C);
    if (!A || !B || !C) {
        printf("Sin memoria.\n");
        free(A);
        free(B);
        free(C);
        return;
    }

    printf("Como quieres las matrices?\n");
    printf("[1] Escribirlas por teclado\n");
    printf("[2] Generarlas aleatoriamente\n");
    printf("Opcion: ");

    if (scanf("%d", &modo) != 1) {
        printf("Entrada invalida.\n");
        limpiar_entrada();
        free(A);
        free(B);
        free(C);
        return;
    }
    limpiar_entrada();

    if (modo == 1) {
        printf("Ingresa los %zu elementos de la matriz A (por filas):\n", total);
        for (size_t i = 0; i < total; i++) {
            if (scanf("%d", &A[i]) != 1) {
                printf("Entrada invalida.\n");
                limpiar_entrada();
                free(A);
                free(B);
                free(C);
                return;
            }
        }
        printf("Ingresa los %zu elementos de la matriz B (por filas):\n", total);
        for (size_t i = 0; i < total; i++) {
            if (scanf("%d", &B[i]) != 1) {
                printf("Entrada invalida.\n");
                limpiar_entrada();
                free(A);
                free(B);
                free(C);
                return;
            }
        }
        limpiar_entrada();
    } else if (modo == 2) {
        srand((unsigned)time(NULL));
        for (size_t i = 0; i < total; i++) {
            A[i] = rand() % 201 - 100;   /* entre -100 y 100 */
            B[i] = rand() % 201 - 100;
        }
        printf("Se generaron dos matrices de %d x %d con enteros entre -100 y 100.\n", n, n);
    } else {
        printf("Opcion no valida.\n");
        free(A);
        free(B);
        free(C);
        return;
    }

    /* Cada elemento de C es independiente -> bucles anidados en paralelo */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            size_t idx = (size_t)i * (size_t)n + (size_t)j;
            C[idx] = (long long)A[idx] + (long long)B[idx];
        }
    }

    /* Impresion solo si n es menor o igual 10 ya que sino se inunda */
    if (n <= 10) {
        printf("\nMatriz A:\n");
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                printf("%6d ", A[(size_t)i * n + j]);
            }
            printf("\n");
        }
        printf("\nMatriz B:\n");
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                printf("%6d ", B[(size_t)i * n + j]);
            }
            printf("\n");
        }
        printf("\nMatriz A + B:\n");
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                printf("%6lld ", C[(size_t)i * n + j]);
            }
            printf("\n");
        }
    } else {
        printf("Suma calculada (matrices de %d x %d; no se imprimen por su tamano).\n", n, n);
    }

    free(A);
    free(B);
    free(C);
}

/* ---------- Operacion 4: revisar si dos cadenas son iguales ---------- */
static void op4_cadenas_iguales(void) {
    printf("Ingresa la primera cadena: ");
    char *s1 = leer_linea_dinamica(stdin);
    if (!s1) {
        printf("Sin memoria.\n");
        return;
    }

    printf("Ingresa la segunda cadena: ");
    char *s2 = leer_linea_dinamica(stdin);
    if (!s2) {
        printf("Sin memoria.\n");
        free(s1);
        return;
    }

    size_t n1 = strlen(s1);
    size_t n2 = strlen(s2);

    if (n1 != n2) {
        printf("Las cadenas NO son iguales (longitudes distintas: %zu y %zu).\n", n1, n2);
    } else {
        size_t diferencias = 0;

        /* Cada posicion se compara de forma independiente -> reduccion */
        #pragma omp parallel for reduction(+:diferencias)
        for (size_t i = 0; i < n1; i++) {
            if (s1[i] != s2[i]) {
                diferencias++;
            }
        }

        if (diferencias == 0) {
            printf("Las cadenas son iguales.\n");
        } else {
            printf("Las cadenas NO son iguales (difieren en %zu posicion(es)).\n", diferencias);
        }
    }

    free(s1);
    free(s2);
}

/* ---------- Operacion 5: ordenar un conjunto de enteros ---------- */
static void op5_ordenar(void) {
    int n, modo;

    printf("Cuantos numeros enteros? ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("n debe ser un entero positivo.\n");
        limpiar_entrada();
        return;
    }
    limpiar_entrada();

    int *v = malloc((size_t)n * sizeof *v);
    if (v == NULL) {
        printf("Sin memoria.\n");
        return;
    }

    printf("Como quieres los %d numeros?\n", n);
    printf("[1] Escribirlos por teclado\n");
    printf("[2] Generarlos aleatoriamente\n");
    printf("Opcion: ");

    if (scanf("%d", &modo) != 1) {
        printf("Entrada invalida.\n");
        free(v);
        limpiar_entrada();
        return;
    }
    limpiar_entrada();

    if (modo == 1) {
        printf("Ingresa %d enteros separados por espacio:\n", n);
        for (int i = 0; i < n; i++) {
            if (scanf("%d", &v[i]) != 1) {
                printf("Entrada invalida.\n");
                free(v);
                limpiar_entrada();
                return;
            }
        }
        limpiar_entrada();
    } else if (modo == 2) {
        srand((unsigned)time(NULL));
        for (int i = 0; i < n; i++) {
            v[i] = rand() % 201 - 100;   /* entre -100 y 100 */
        }
        printf("Se generaron %d enteros aleatorios entre -100 y 100.\n", n);
    } else {
        printf("Opcion no valida.\n");
        free(v);
        return;
    }

    /* Impresion secuencial */
    if (n <= 50) {
        printf("Original: [");
        for (int i = 0; i < n; i++) {
            printf("%d%s", v[i], (i < n - 1) ? ", " : "");
        }
        printf("]\n");
    }

    /* Ordenamiento par-impar en cada fase los pares (i, i+1) son disjuntos -> se comparan en paralelo */
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

    if (n <= 50) {
        printf("Ordenado: [");
        for (int i = 0; i < n; i++) {
            printf("%d%s", v[i], (i < n - 1) ? ", " : "");
        }
        printf("]\n");
    } else {
        printf("Arreglo de %d elementos ordenado (no se imprime por su tamano).\n", n);
    }

    free(v);
}

/* ---------- Operacion 6: revisar si una palabra se encuentra en un texto ---------- */
static void op6_palabra_en_texto(void) {
    int modo;
    printf("Como quieres ingresar el texto?\n");
    printf("[1] Escribirlo por teclado\n");
    printf("[2] Leerlo desde un archivo\n");
    printf("Opcion: ");

    if (scanf("%d", &modo) != 1) {
        printf("Entrada invalida.\n");
        limpiar_entrada();
        return;
    }
    limpiar_entrada();

    char *texto = NULL;
    size_t len_texto = 0;

    if (modo == 1) {
        printf("Ingresa el texto: ");
        texto = leer_linea_dinamica(stdin);
        if (!texto) {
            printf("Sin memoria.\n");
            return;
        }
        len_texto = strlen(texto);
    } else if (modo == 2) {
        printf("Ingresa la ruta del archivo: ");
        char *ruta = leer_linea_dinamica(stdin);
        if (!ruta) {
            printf("Sin memoria.\n");
            return;
        }
        texto = leer_archivo_completo(ruta, &len_texto);
        if (!texto) {
            printf("Error: no se pudo abrir el archivo '%s'.\n", ruta);
            free(ruta);
            return;
        }
        free(ruta);
    } else {
        printf("Opcion no valida.\n");
        return;
    }

    printf("Ingresa la palabra a buscar: ");
    char *palabra_raw = leer_linea_dinamica(stdin);
    if (!palabra_raw) {
        free(texto);
        printf("Sin memoria.\n");
        return;
    }

    char *objetivo = normalizar_palabra(palabra_raw, strlen(palabra_raw));
    if (!objetivo || objetivo[0] == '\0') {
        printf("La palabra '%s' NO se encuentra en el texto.\n", palabra_raw);
        free(objetivo);
        free(palabra_raw);
        free(texto);
        return;
    }

    /* Extraer tokens separados por espacios en blanco */
    size_t cap = 64;
    size_t num_palabras = 0;
    char **palabras = malloc(cap * sizeof *palabras);
    if (!palabras) {
        free(objetivo);
        free(palabra_raw);
        free(texto);
        printf("Sin memoria.\n");
        return;
    }

    size_t i = 0;
    while (i < len_texto) {
        while (i < len_texto && isspace((unsigned char)texto[i])) {
            i++;
        }
        if (i >= len_texto) {
            break;
        }
        size_t inicio = i;
        while (i < len_texto && !isspace((unsigned char)texto[i])) {
            i++;
        }
        char *limpia = normalizar_palabra(texto + inicio, i - inicio);
        if (limpia && limpia[0] != '\0') {
            if (num_palabras == cap) {
                size_t nueva_cap = cap * 2;
                char **tmp = realloc(palabras, nueva_cap * sizeof *palabras);
                if (!tmp) {
                    free(limpia);
                    break;
                }
                palabras = tmp;
                cap = nueva_cap;
            }
            palabras[num_palabras++] = limpia;
        } else {
            free(limpia);
        }
    }

    /* Busqueda en paralelo de la existencia de la palabra */
    int encontrada = 0;
    #pragma omp parallel for reduction(+:encontrada)
    for (size_t k = 0; k < num_palabras; k++) {
        if (strcmp(palabras[k], objetivo) == 0) {
            encontrada = 1;
        }
    }

    if (encontrada > 0) {
        printf("La palabra '%s' SI se encuentra en el texto.\n", palabra_raw);
    } else {
        printf("La palabra '%s' NO se encuentra en el texto.\n", palabra_raw);
    }

    for (size_t k = 0; k < num_palabras; k++) {
        free(palabras[k]);
    }
    free(palabras);
    free(objetivo);
    free(palabra_raw);
    free(texto);
}

/* ---------- Operacion 7: interseccion de dos conjuntos de enteros ---------- */
static void op7_interseccion(void) {
    int n, m, modo;

    printf("Cuantos elementos tiene el conjunto A? ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("n debe ser un entero positivo.\n");
        limpiar_entrada();
        return;
    }

    printf("Cuantos elementos tiene el conjunto B? ");
    if (scanf("%d", &m) != 1 || m <= 0) {
        printf("m debe ser un entero positivo.\n");
        limpiar_entrada();
        return;
    }
    limpiar_entrada();

    int *A = malloc((size_t)n * sizeof *A);
    int *B = malloc((size_t)m * sizeof *B);
    if (!A || !B) {
        printf("Sin memoria.\n");
        free(A);
        free(B);
        return;
    }

    printf("Como quieres los conjuntos?\n");
    printf("[1] Escribirlos por teclado\n");
    printf("[2] Generarlos aleatoriamente\n");
    printf("Opcion: ");

    if (scanf("%d", &modo) != 1) {
        printf("Entrada invalida.\n");
        free(A);
        free(B);
        limpiar_entrada();
        return;
    }
    limpiar_entrada();

    if (modo == 1) {
        printf("Ingresa los %d enteros del conjunto A:\n", n);
        for (int i = 0; i < n; i++) {
            if (scanf("%d", &A[i]) != 1) {
                printf("Entrada invalida.\n");
                free(A);
                free(B);
                limpiar_entrada();
                return;
            }
        }
        printf("Ingresa los %d enteros del conjunto B:\n", m);
        for (int j = 0; j < m; j++) {
            if (scanf("%d", &B[j]) != 1) {
                printf("Entrada invalida.\n");
                free(A);
                free(B);
                limpiar_entrada();
                return;
            }
        }
        limpiar_entrada();
    } else if (modo == 2) {
        srand((unsigned)time(NULL));
        for (int i = 0; i < n; i++) {
            A[i] = rand() % 50 - 25;
        }
        for (int j = 0; j < m; j++) {
            B[j] = rand() % 50 - 25;
        }
        printf("Se generaron conjuntos aleatorios para A y B.\n");
    } else {
        printf("Opcion no valida.\n");
        free(A);
        free(B);
        return;
    }

    int max_inter = (n < m) ? n : m;
    int *interseccion = malloc((size_t)max_inter * sizeof *interseccion);
    int count = 0;

    if (!interseccion) {
        printf("Sin memoria.\n");
        free(A);
        free(B);
        return;
    }

    /* Calculo de la interseccion en paralelo usando hilos y seccion critica */
    #pragma omp parallel
    {
        int *local_inter = malloc((size_t)max_inter * sizeof *local_inter);
        int local_count = 0;

        #pragma omp for
        for (int i = 0; i < n; i++) {
            int val = A[i];
            int ya_registrado = 0;
            for (int l = 0; l < local_count; l++) {
                if (local_inter[l] == val) {
                    ya_registrado = 1;
                    break;
                }
            }
            if (!ya_registrado) {
                int en_b = 0;
                for (int j = 0; j < m; j++) {
                    if (B[j] == val) {
                        en_b = 1;
                        break;
                    }
                }
                if (en_b) {
                    local_inter[local_count++] = val;
                }
            }
        }

        #pragma omp critical
        {
            for (int l = 0; l < local_count; l++) {
                int duplicado = 0;
                for (int c_idx = 0; c_idx < count; c_idx++) {
                    if (interseccion[c_idx] == local_inter[l]) {
                        duplicado = 1;
                        break;
                    }
                }
                if (!duplicado) {
                    interseccion[count++] = local_inter[l];
                }
            }
        }
        free(local_inter);
    }

    printf("Interseccion de A y B (%d elementos):\n[", count);
    for (int i = 0; i < count; i++) {
        printf("%d%s", interseccion[i], (i < count - 1) ? ", " : "");
    }
    printf("]\n");

    free(A);
    free(B);
    free(interseccion);
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

    /* b2 y ac4 son independientes -> en paralelo */
    #pragma omp parallel sections
    {
        #pragma omp section
        { b2 = b * b; }

        #pragma omp section
        { ac4 = 4.0 * a * c; }
    }

    /* punto de union: D depende de b2 y ac4 */
    D = b2 - ac4;
    printf("Discriminante D = %.4f\n", D);

    if (D < 0.0) {
        printf("No hay raices reales.\n");
        return;
    }

    r = sqrt(D);

    /* x1 y x2 son independientes -> en paralelo */
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
    int modo;
    printf("Como quieres ingresar el texto?\n");
    printf("[1] Escribirlo por teclado\n");
    printf("[2] Leerlo desde un archivo\n");
    printf("Opcion: ");

    if (scanf("%d", &modo) != 1) {
        printf("Entrada invalida.\n");
        limpiar_entrada();
        return;
    }
    limpiar_entrada();

    char *texto = NULL;
    size_t len_texto = 0;

    if (modo == 1) {
        printf("Ingresa el texto: ");
        texto = leer_linea_dinamica(stdin);
        if (!texto) {
            printf("Sin memoria.\n");
            return;
        }
        len_texto = strlen(texto);
    } else if (modo == 2) {
        printf("Ingresa la ruta del archivo: ");
        char *ruta = leer_linea_dinamica(stdin);
        if (!ruta) {
            printf("Sin memoria.\n");
            return;
        }
        texto = leer_archivo_completo(ruta, &len_texto);
        if (!texto) {
            printf("Error: no se pudo abrir el archivo '%s'.\n", ruta);
            free(ruta);
            return;
        }
        free(ruta);
    } else {
        printf("Opcion no valida.\n");
        return;
    }

    printf("Ingresa la palabra a buscar: ");
    char *palabra_raw = leer_linea_dinamica(stdin);
    if (!palabra_raw) {
        free(texto);
        printf("Sin memoria.\n");
        return;
    }

    char *objetivo = normalizar_palabra(palabra_raw, strlen(palabra_raw));
    if (!objetivo || objetivo[0] == '\0') {
        printf("La palabra '%s' aparece 0 veces.\n", palabra_raw);
        free(objetivo);
        free(palabra_raw);
        free(texto);
        return;
    }

    /* Extraer tokens separados por espacios en blanco */
    size_t cap = 64;
    size_t num_palabras = 0;
    char **palabras = malloc(cap * sizeof *palabras);
    if (!palabras) {
        free(objetivo);
        free(palabra_raw);
        free(texto);
        printf("Sin memoria.\n");
        return;
    }

    size_t i = 0;
    while (i < len_texto) {
        while (i < len_texto && isspace((unsigned char)texto[i])) {
            i++;
        }
        if (i >= len_texto) {
            break;
        }
        size_t inicio = i;
        while (i < len_texto && !isspace((unsigned char)texto[i])) {
            i++;
        }
        char *limpia = normalizar_palabra(texto + inicio, i - inicio);
        if (limpia && limpia[0] != '\0') {
            if (num_palabras == cap) {
                size_t nueva_cap = cap * 2;
                char **tmp = realloc(palabras, nueva_cap * sizeof *palabras);
                if (!tmp) {
                    free(limpia);
                    break;
                }
                palabras = tmp;
                cap = nueva_cap;
            }
            palabras[num_palabras++] = limpia;
        } else {
            free(limpia);
        }
    }

    size_t apariciones = 0;
    #pragma omp parallel for reduction(+:apariciones)
    for (size_t k = 0; k < num_palabras; k++) {
        if (strcmp(palabras[k], objetivo) == 0) {
            apariciones++;
        }
    }

    printf("La palabra '%s' aparece %zu veces.\n", palabra_raw, apariciones);

    for (size_t k = 0; k < num_palabras; k++) {
        free(palabras[k]);
    }
    free(palabras);
    free(objetivo);
    free(palabra_raw);
    free(texto);
}

/* ---------- Operacion 10: sumatoria de n enteros ---------- */
static void op10_sumatoria(void) {
    int n, modo;
    long long suma = 0;

    printf("Cuantos numeros enteros? ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("n debe ser un entero positivo.\n");
        limpiar_entrada();
        return;
    }
    limpiar_entrada();

    int *v = malloc((size_t)n * sizeof *v);
    if (v == NULL) {
        printf("Sin memoria.\n");
        return;
    }

    printf("Como quieres los %d numeros?\n", n);
    printf("[1] Escribirlos por teclado\n");
    printf("[2] Generarlos aleatoriamente\n");
    printf("Opcion: ");

    if (scanf("%d", &modo) != 1) {
        printf("Entrada invalida.\n");
        free(v);
        limpiar_entrada();
        return;
    }
    limpiar_entrada();

    if (modo == 1) {
        printf("Ingresa %d enteros separados por espacio:\n", n);
        for (int i = 0; i < n; i++) {
            if (scanf("%d", &v[i]) != 1) {
                printf("Entrada invalida.\n");
                free(v);
                limpiar_entrada();
                return;
            }
        }
        limpiar_entrada();
    } else if (modo == 2) {
        srand((unsigned)time(NULL));
        for (int i = 0; i < n; i++) {
            v[i] = rand() % 201 - 100;   /* entre -100 y 100 */
        }
        printf("Se generaron %d enteros aleatorios entre -100 y 100.\n", n);
    } else {
        printf("Opcion no valida.\n");
        free(v);
        return;
    }

    printf("arreglo: [");

    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        printf("%d, ", v[i]);
    }

    printf("]\n");

    /* reduccion: cada hilo acumula su parte y OpenMP las combina */
    #pragma omp parallel for reduction(+:suma) schedule(static)
    for (int i = 0; i < n; i++) {
        suma += (long long)v[i];
    }

    printf("Sumatoria = %lld\n", suma);
    free(v);
}
