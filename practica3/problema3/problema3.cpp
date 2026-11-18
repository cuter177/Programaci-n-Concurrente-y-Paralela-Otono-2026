#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>

void interseccion(const char **A, int nA, const char **B, int nB,
                  const char **C, int *total) {
    std::vector<char> P(nA, 0);
    std::vector<std::thread> tareas;

    for (int i = 0; i < nA; i++)
        tareas.emplace_back([=, &P]() {
            for (int j = 0; j < nB; j++)
                if (strcmp(A[i], B[j]) == 0) {
                    P[i] = 1;
                    break;
                }
        });

    for (std::thread &t : tareas)
        t.join();

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
