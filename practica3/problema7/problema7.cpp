#include <iostream>
#include <thread>

// Datos globales de la compra
double monto_total = 1600.00;
bool tiene_antiacido = true;

// Arreglo global donde cada hilo escribe su descuento calculado
// Tarea 0 -> Regla 20% ($1000)
// Tarea 1 -> Regla 30% ($1500)
// Tarea 2 -> Regla 10% (Antiácido)
double descuentos[3] = {0.0, 0.0, 0.0};

// Salida global final
double descuento_maximo = 0.0;
double total_con_descuento = 0.0;

// Tarea 0: Evaluar regla del 20% para compras >= $1000
void tarea_regla_1000() {
    if (monto_total >= 1000.00) {
        descuentos[0] = 0.20;
    }
}

// Tarea 1: Evaluar regla del 30% para compras >= $1500
void tarea_regla_1500() {
    if (monto_total >= 1500.00) {
        descuentos[1] = 0.30;
    }
}

// Tarea 2: Evaluar regla del 10% por compra de antiácido
void tarea_regla_antiacido() {
    if (tiene_antiacido) {
        descuentos[2] = 0.10;
    }
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

    // Fase de Consolidacion Secuencial: Determinacion del descuento maximo aplicable
    descuento_maximo = descuentos[0];
    for (int i = 1; i < 3; ++i) {
        if (descuentos[i] > descuento_maximo) {
            descuento_maximo = descuentos[i];
        }
    }

    total_con_descuento = monto_total * (1.0 - descuento_maximo);

    // Mostrar resultados
    std::cout << "--- Resultados por Regla de Descuento ---\n";
    std::cout << "Regla 1 (Monto >= $1000.00) : " << (descuentos[0] * 100) << "%\n";
    std::cout << "Regla 2 (Monto >= $1500.00) : " << (descuentos[1] * 100) << "%\n";
    std::cout << "Regla 3 (Tiene Antiácido)  : " << (descuentos[2] * 100) << "%\n";

    std::cout << "\n--- Consolidacion Final ---\n";
    std::cout << "Descuento máximo aplicable  : " << (descuento_maximo * 100) << "%\n";
    std::cout << "Monto original             : $" << monto_total << "\n";
    std::cout << "Total final a pagar        : $" << total_con_descuento << "\n";

    return 0;
}
