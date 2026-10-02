#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>

static int iguales(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

// Fase 1: T_i,j escribe M[i][j] = (A[i] == B[j]).
// Cada tarea escribe una celda distinta -> Bernstein se cumple.
static void fase_comparacion(const char **A, int nA, const char **B, int nB,
                             char *M) {
    std::vector<std::thread> tareas;
    for (int i = 0; i < nA; i++)
        for (int j = 0; j < nB; j++)
            tareas.emplace_back([=]() {
                M[(size_t)i * nB + j] = iguales(A[i], B[j]) ? 1 : 0;
            });
    for (std::thread &t : tareas)
        t.join();
}

// Fase 2: S_i escribe P[i] = OR_j M[i][j].
// Las filas son independientes y cada S_i escribe P[i] distinto.
static void fase_reduccion(int nA, int nB, const char *M, char *P) {
    std::vector<std::thread> tareas;
    for (int i = 0; i < nA; i++)
        tareas.emplace_back([=]() {
            char p = 0;
            for (int j = 0; j < nB; j++)
                p = p || M[(size_t)i * nB + j];
            P[i] = p;
        });
    for (std::thread &t : tareas)
        t.join();
}

void interseccion(const char **A, int nA, const char **B, int nB,
                  const char **C, int *total) {
    std::vector<char> M((size_t)nA * nB);
    std::vector<char> P(nA, 0);

    // COBEGIN-COEND: comparaciones T_i,j
    fase_comparacion(A, nA, B, nB, M.data());
    // COBEGIN-COEND: reduccion OR por filas S_i
    fase_reduccion(nA, nB, M.data(), P.data());

    // JOIN secuencial: C = { A[i] | P[i] = 1 }.
    // Permanece secuencial porque todas las escrituras comparten el indice k.
    int k = 0;
    for (int i = 0; i < nA; i++)
        if (P[i])
            C[k++] = A[i];
    *total = k;
}

int main(void) {
    const char *A[] = {"mango", "pera", "uva", "kiwi", "manzana", "fresa", "melon"};
    const char *B[] = {"uva", "kiwi", "pera", "sandia", "melon", "naranja"};
    int nA = sizeof(A) / sizeof(A[0]);
    int nB = sizeof(B) / sizeof(B[0]);

    const char **C = (const char **)malloc((nA < nB ? nA : nB) * sizeof(char *));
    int total;

    interseccion(A, nA, B, nB, C, &total);

    for (int i = 0; i < total; i++)
        printf("%s\n", C[i]);
    free(C);
    return 0;
}
