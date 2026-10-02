#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>

#define MAX_TAREAS 4

typedef struct {
    const char **v;
    int n;
} Orden;

typedef struct {
    const char **A, **B, **C;
    int nB, lo, hi;
    int *cnt, des;
    int modo;
} Tarea;

static int cmp(const void *a, const void *b) {
    return strcmp(*(char *const *)a, *(char *const *)b);
}

static int existe(const char **B, int n, const char *s) {
    int lo = 0, hi = n;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        int r = strcmp(s, B[mid]);
        if (r == 0) return 1;
        if (r < 0) hi = mid; else lo = mid + 1;
    }
    return 0;
}

static void *ordenar(void *arg) {
    Orden *o = (Orden *)arg;
    qsort(o->v, o->n, sizeof(char *), cmp);
    return NULL;
}

static void *ejecutar(void *arg) {
    Tarea *t = (Tarea *)arg;
    if (t->modo == 0) {
        int c = 0;
        for (int i = t->lo; i < t->hi; i++)
            if (existe(t->B, t->nB, t->A[i])) c++;
        *t->cnt = c;
    } else {
        int d = t->des;
        for (int i = t->lo; i < t->hi; i++)
            if (existe(t->B, t->nB, t->A[i])) t->C[d++] = t->A[i];
    }
    return NULL;
}

static void cobegin_coend(void *(*fn)(void *), void **args, int n) {
    pthread_t h[MAX_TAREAS];
    for (int i = 0; i < n; i++) pthread_create(&h[i], NULL, fn, args[i]);
    for (int i = 0; i < n; i++) pthread_join(h[i], NULL);
}

void interseccion(const char **A, int nA, const char **B, int nB, const char **C, int *total) {
    Orden o1 = {A, nA}, o2 = {B, nB};
    void *f1[2] = {&o1, &o2};
    cobegin_coend(ordenar, f1, 2);

    int b1 = nA / 4, b2 = nA / 2, b3 = 3 * nA / 4;
    int cnt[MAX_TAREAS] = {0};
    Tarea t1[MAX_TAREAS];
    int ini[MAX_TAREAS] = {0, b1, b2, b3};
    int fin[MAX_TAREAS] = {b1, b2, b3, nA};
    void *f2[MAX_TAREAS];
    for (int i = 0; i < MAX_TAREAS; i++) {
        t1[i] = (Tarea){A, B, NULL, nB, ini[i], fin[i], &cnt[i], 0, 0};
        f2[i] = &t1[i];
    }
    cobegin_coend(ejecutar, f2, MAX_TAREAS);

    int off[MAX_TAREAS], acc = 0;
    for (int i = 0; i < MAX_TAREAS; i++) { off[i] = acc; acc += cnt[i]; }

    Tarea t2[MAX_TAREAS];
    void *f3[MAX_TAREAS];
    for (int i = 0; i < MAX_TAREAS; i++) {
        t2[i] = (Tarea){A, B, C, nB, ini[i], fin[i], NULL, off[i], 1};
        f3[i] = &t2[i];
    }
    cobegin_coend(ejecutar, f3, MAX_TAREAS);

    *total = acc;
}

int main(void) {
    const char *A[] = {"mango", "pera", "uva", "kiwi", "manzana", "fresa", "melon"};
    const char *B[] = {"uva", "kiwi", "pera", "sandia", "melon", "naranja"};
    int nA = sizeof(A) / sizeof(A[0]), nB = sizeof(B) / sizeof(B[0]);

    const char **C = (const char **)malloc((nA < nB ? nA : nB) * sizeof(char *));
    int total;

    interseccion(A, nA, B, nB, C, &total);

    for (int i = 0; i < total; i++) printf("%s\n", C[i]);
    free(C);
    return 0;
}
