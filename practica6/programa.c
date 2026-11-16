#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>

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
    for(int i = 0; i < n; i++){
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

/* ---------- Stubs (aun no implementadas) ---------- */
static void op1_espacios(void)            { printf("Operacion 1 aun no implementada.\n"); }
static void op2_palabras_caracteres(void) { printf("Operacion 2 aun no implementada.\n"); }
static void op3_sumar_matrices(void)      { printf("Operacion 3 aun no implementada.\n"); }
static void op4_cadenas_iguales(void)     { printf("Operacion 4 aun no implementada.\n"); }
static void op5_ordenar(void)             { printf("Operacion 5 aun no implementada.\n"); }
static void op6_palabra_en_texto(void)    { printf("Operacion 6 aun no implementada.\n"); }
static void op7_interseccion(void)        { printf("Operacion 7 aun no implementada.\n"); }
static void op9_contar_palabra(void)      { printf("Operacion 9 aun no implementada.\n"); }
