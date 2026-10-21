#include <iostream>
#include <thread>

// Dimensiones de la matriz
const int N = 4; // Filas
const int M = 5; // Columnas

int A[N][M] = {
    { 12,  45,   7,  23,  19 },
    { 89,  34,  92,  11,  56 },
    {  3,  78,  65,  41,  15 },
    { 67,  28,  33,  95,  50 }
};

// Cada hilo i escribe únicamente en max_fila[i]
int max_fila[N];

// Salida: max_global
int max_global;

void tarea_fila(int i) {
    int local = A[i][0];
    for (int j = 1; j < M; ++j) {
        if (A[i][j] > local) {
            local = A[i][j];
        }
    }
    max_fila[i] = local;
}

int main() {
    std::thread hilos[N];

    //Se lanzan las tareas S_0, S_1, ..., S_N-1 en paralelo
    for (int i = 0; i < N; ++i) {
        hilos[i] = std::thread(tarea_fila, i);
    }

    for (int i = 0; i < N; ++i) {
        hilos[i].join();
    }

    max_global = max_fila[0];
    for (int i = 1; i < N; ++i) {
        if (max_fila[i] > max_global) {
            max_global = max_fila[i];
        }
    }

    // Mostrar resultados
    std::cout << "--- Maximos locales por fila (max_fila) ---\n";
    for (int i = 0; i < N; ++i) {
        std::cout << "Fila " << i << ": " << max_fila[i] << "\n";
    }

    std::cout << "\n--- Maximo global (S_final) ---\n";
    std::cout << "max_global = " << max_global << "\n";

    return 0;
}
