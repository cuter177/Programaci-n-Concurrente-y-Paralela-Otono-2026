#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>

#define MAX_TAREAS 4

typedef struct {
    const char **src, **dst;
    int lo, mid, hi;
} Tarea;

static int cmp(const void *a, const void *b) {
    return strcmp(*(char *const *)a, *(char *const *)b);
}

static void mezclar(const char **src, int lo, int mid, int hi, const char **dst) {
    int i = lo, j = mid, k = lo;
    while (i < mid && j < hi)
        dst[k++] = (strcmp(src[i], src[j]) <= 0) ? src[i++] : src[j++];
    while (i < mid) dst[k++] = src[i++];
    while (j < hi)  dst[k++] = src[j++];
}

static void *ejecutar(void *arg) {
    Tarea *t = (Tarea *)arg;
    if (t->mid < 0)
        qsort(t->src + t->lo, t->hi - t->lo, sizeof(char *), cmp);
    else
        mezclar(t->src, t->lo, t->mid, t->hi, t->dst);
    return NULL;
}

static void cobegin_coend(Tarea *t, int n) {
    pthread_t h[MAX_TAREAS];
    for (int i = 0; i < n; i++) pthread_create(&h[i], NULL, ejecutar, &t[i]);
    for (int i = 0; i < n; i++) pthread_join(h[i], NULL);
}

void ordenar_paralelo(const char **A, int n) {
    const char **T = (const char **)malloc(n * sizeof(char *));
    int b0 = 0, b1 = n / 4, b2 = n / 2, b3 = 3 * n / 4, b4 = n;

    Tarea fase1[4] = {
        {A, NULL, b0, -1, b1}, {A, NULL, b1, -1, b2},
        {A, NULL, b2, -1, b3}, {A, NULL, b3, -1, b4}
    };
    cobegin_coend(fase1, 4);

    Tarea fase2[2] = {
        {A, T, b0, b1, b2},
        {A, T, b2, b3, b4}
    };
    cobegin_coend(fase2, 2);

    Tarea fase3 = {T, A, b0, b2, b4};
    ejecutar(&fase3);

    free(T);
}

int main(void) {
    const char *A[] = {"mango", "pera", "manzana", "uva", "kiwi", "durazno",
                 "fresa", "limon", "naranja", "piña", "cereza", "melon",
                 "sandia", "platano", "guayaba"};
    int n = sizeof(A) / sizeof(A[0]);

    ordenar_paralelo(A, n);

    for (int i = 0; i < n; i++) printf("%s\n", A[i]);
    return 0;
}
