#include "sala.h"
#include <iostream>
#include <thread>
#include <vector>
#include <string>

void reservar(Sala& sala, int fila, int columna, const std::string& nombre, int edad) {
    Persona persona(nombre, edad);
    bool ok = sala.asignarAsiento(fila, columna, persona);
    std::cout << (ok ? "[OK] " : "[X]  ") << nombre
              << " -> asiento (" << fila << "," << columna << ")"
              << (ok ? " reservado\n" : " no disponible\n");
}

int main() {
    Sala sala(8, 10);

    std::cout << " Reservas de asiento \n";
    std::vector<std::thread> hilos;
    hilos.emplace_back(reservar, std::ref(sala), 0, 0, "Ana", 20);
    hilos.emplace_back(reservar, std::ref(sala), 0, 0, "Luis", 23);
    hilos.emplace_back(reservar, std::ref(sala), 0, 0, "Maria", 22);
    hilos.emplace_back(reservar, std::ref(sala), 2, 4, "Pedro", 21);
    hilos.emplace_back(reservar, std::ref(sala), 3, 7, "Sofia", 19);
    for (auto& hilo : hilos) {
        hilo.join();
    }

    std::cout << "\n Liberar asiento (2,4) \n";
    std::cout << (sala.liberarAsiento(2, 4) ? "[OK] Asiento liberado\n"
                                            : "[X]  No se pudo liberar\n");

    std::cout << "\n Buscar por asiento (0,0) \n";
    if (auto persona = sala.buscarPorAsiento(0, 0)) {
        std::cout << "Asiento (0,0): " << persona->getNombre()
                  << ", " << persona->getEdad() << " anios\n";
    } else {
        std::cout << "Asiento (0,0) libre\n";
    }

    std::cout << "\n=== Buscar por nombre 'Sofia' ===\n";
    auto ubicaciones = sala.buscarPorNombre("Sofia");
    if (ubicaciones.empty()) {
        std::cout << "Sofia no esta en la sala\n";
    }
    for (auto& [fila, columna] : ubicaciones) {
        std::cout << "Sofia esta en (" << fila << "," << columna << ")\n";
    }

    std::cout << "\n=== Estado de la sala ("
              << sala.asientosOcupados() << "/80) ===\n";
    sala.mostrarSala();

    return 0;
}
