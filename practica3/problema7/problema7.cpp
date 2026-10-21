#include <iostream>
#include <thread>

// Datos globales de la compra
double monto_total = 1600.00;
bool tiene_antiacido = true;

// Arreglo global donde cada hilo escribe su condicion calculada
// Tarea 0 -> c20 = (T >= 1000)
// Tarea 1 -> c30 = (T >= 1500)
// Tarea 2 -> c10 = (q >= 1)
bool condiciones[3] = {false, false, false};

// Salida global final
double descuento_maximo = 0.0;
double total_con_descuento = 0.0;

// Tarea 0: Evaluar regla del 20% para compras >= $1000
void tarea_regla_1000() {
    condiciones[0] = (monto_total >= 1000.00);
}

// Tarea 1: Evaluar regla del 30% para compras >= $1500
void tarea_regla_1500() {
    condiciones[1] = (monto_total >= 1500.00);
}

// Tarea 2: Evaluar regla del 10% por compra de antiácido
void tarea_regla_antiacido() {
    condiciones[2] = tiene_antiacido;
}

int main() {
    std::thread hilos[3];

    // COBEGIN: Lanzamiento en paralelo de las evaluaciones independientes
    hilos[0] = std::thread(tarea_regla_1000);
    hilos[1] = std::thread(tarea_regla_1500);
    hilos[2] = std::thread(tarea_regla_antiacido);

    // COEND: Espera de finalizacion de todos los hilos
    for (int i = 0; i < 3; ++i) {
        hilos[i].join();
    }

    // Fase de Consolidacion Secuencial: los descuentos no son acumulables,
    // se elige uno solo con prioridad c30 > c20 > c10.
    if (condiciones[1]) {
        descuento_maximo = 0.30;
    } else if (condiciones[0]) {
        descuento_maximo = 0.20;
    } else if (condiciones[2]) {
        descuento_maximo = 0.10;
    } else {
        descuento_maximo = 0.0;
    }

    total_con_descuento = monto_total * (1.0 - descuento_maximo);

    // Mostrar resultados
    std::cout << "--- Resultados por Regla de Descuento ---\n";
    std::cout << "Regla 1 (Monto >= $1000.00) : " << (condiciones[0] ? 20 : 0) << "%\n";
    std::cout << "Regla 2 (Monto >= $1500.00) : " << (condiciones[1] ? 30 : 0) << "%\n";
    std::cout << "Regla 3 (Tiene Antiácido)  : " << (condiciones[2] ? 10 : 0) << "%\n";

    std::cout << "\n--- Consolidacion Final ---\n";
    std::cout << "Descuento máximo aplicable  : " << (descuento_maximo * 100) << "%\n";
    std::cout << "Monto original             : $" << monto_total << "\n";
    std::cout << "Total final a pagar        : $" << total_con_descuento << "\n";

    return 0;
}
