#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>

// cmp-exch(A[i], A[i+1]): intercambia la pareja si estan desordenados.
// Cada tarea toca indices disjuntos dentro de una misma fase, por lo que
// cumple las condiciones de Bernstein y no requiere exclusion mutua.
static void cmp_exch(const char **A, int i) {
    if (strcmp(A[i], A[i + 1]) > 0) {
        const char *tmp = A[i];
        A[i] = A[i + 1];
        A[i + 1] = tmp;
    }
}

// COBEGIN-COEND de una fase: lanza en paralelo las parejas (inicio, inicio+1),
// (inicio+2, inicio+3), ... y espera a que todas terminen antes de continuar.
// La barrera de fin (COEND) respeta la dependencia RAW entre fases.
static void fase(const char **A, int n, int inicio) {
    std::vector<std::thread> tareas;
    for (int i = inicio; i + 1 < n; i += 2)
        tareas.emplace_back(cmp_exch, A, i);
    for (std::thread &t : tareas)
        t.join();
}

// Ordenamiento por transposicion par-impar.
static void ordenar_paralelo(const char **A, int n) {
    for (int f = 0; f < n; f++)
        fase(A, n, f % 2);
}

int main(void) {
    const char *A[] = {"mango", "pera", "manzana", "uva", "kiwi", "durazno",
                       "fresa", "limon", "naranja", "piña", "cereza", "melon",
                       "sandia", "platano", "guayaba"};
    int n = sizeof(A) / sizeof(A[0]);

    ordenar_paralelo(A, n);

    for (int i = 0; i < n; i++)
        printf("%s\n", A[i]);
    return 0;
}
