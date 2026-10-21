#include <iostream>
#include <thread>

// Tamaño del arreglo
const int N = 8;

// Arreglo global de entrada y trabajo
int arreglo[N] = {10, 20, 30, 40, 50, 60, 70, 80};

// Tarea individual que intercambia las posiciones i y (N - 1 - i)
void tarea_intercambio(int i) {
    int temp = arreglo[i];
    arreglo[i] = arreglo[N - 1 - i];
    arreglo[N - 1 - i] = temp;
}

int main() {
    // Numero de intercambios en paralelo es N / 2
    const int NUM_TAREAS = N / 2;
    std::thread hilos[NUM_TAREAS];

    std::cout << "--- Arreglo original ---\n";
    for (int i = 0; i < N; ++i) {
        std::cout << arreglo[i] << " ";
    }
    std::cout << "\n\n";

    // COBEGIN: Se lanzan en paralelo los N/2 intercambios independientes
    for (int i = 0; i < NUM_TAREAS; ++i) {
        hilos[i] = std::thread(tarea_intercambio, i);
    }

    // COEND: Sincronizacion de finalizacion de todos los hilos
    for (int i = 0; i < NUM_TAREAS; ++i) {
        hilos[i].join();
    }

    // Mostrar el arreglo invertido
    std::cout << "--- Arreglo invertido (S_final) ---\n";
    for (int i = 0; i < N; ++i) {
        std::cout << arreglo[i] << " ";
    }
    std::cout << "\n";

    return 0;
}
