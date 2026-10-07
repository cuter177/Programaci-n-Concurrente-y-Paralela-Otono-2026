#include "sala.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>

Sala::Sala(int filas, int columnas) : filas(filas), columnas(columnas) {
    if (filas <= 0 || columnas <= 0) {
        throw std::invalid_argument("Las dimensiones de la sala deben ser positivas");
    }
    for (int f = 0; f < filas; ++f) {
        asientos.emplace_back();
        for (int c = 0; c < columnas; ++c) {
            asientos.back().emplace_back(f * columnas + c + 1);
        }
    }
}

bool Sala::dentroDeRango(int fila, int columna) const {
    return fila >= 0 && fila < filas && columna >= 0 && columna < columnas;
}

bool Sala::asignarAsiento(int fila, int columna, const Persona& persona) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!dentroDeRango(fila, columna)) {
        return false;
    }
    return asientos[fila][columna].ocupar(persona);
}

bool Sala::liberarAsiento(int fila, int columna) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!dentroDeRango(fila, columna)) {
        return false;
    }
    return asientos[fila][columna].liberar();
}

std::optional<Persona> Sala::buscarPorAsiento(int fila, int columna) const {
    std::lock_guard<std::mutex> lock(mtx);
    if (!dentroDeRango(fila, columna)) {
        return std::nullopt;
    }
    return asientos[fila][columna].getOcupante();
}

std::vector<std::pair<int, int>> Sala::buscarPorNombre(const std::string& nombre) const {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<std::pair<int, int>> encontrados;
    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            if (asientos[f][c].perteneceA(nombre)) {
                encontrados.emplace_back(f, c);
            }
        }
    }
    return encontrados;
}

int Sala::asientosOcupados() const {
    std::lock_guard<std::mutex> lock(mtx);
    int ocupados = 0;
    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            if (asientos[f][c].isOcupado()) {
                ++ocupados;
            }
        }
    }
    return ocupados;
}

void Sala::mostrarSala() const {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "\n     ";
    for (int c = 0; c < columnas; ++c) {
        std::cout << std::setw(3) << c;
    }
    std::cout << "\n";
    for (int f = 0; f < filas; ++f) {
        std::cout << "F" << f << "  ";
        for (int c = 0; c < columnas; ++c) {
            std::cout << std::setw(3) << (asientos[f][c].isOcupado() ? "[X]" : "[ ]");
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}
