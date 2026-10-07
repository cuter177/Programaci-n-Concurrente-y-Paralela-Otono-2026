#ifndef SALA_H
#define SALA_H

#include <vector>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include "asiento.h"

class Sala {
  private:
    int filas;
    int columnas;
    std::vector<std::vector<Asiento>> asientos;
    mutable std::mutex mtx;

    bool dentroDeRango(int fila, int columna) const;

  public:
    Sala(int filas, int columnas);

    bool asignarAsiento(int fila, int columna, const Persona& persona);
    bool liberarAsiento(int fila, int columna);

    std::optional<Persona> buscarPorAsiento(int fila, int columna) const;
    std::vector<std::pair<int, int>> buscarPorNombre(const std::string& nombre) const;

    int asientosOcupados() const;
    void mostrarSala() const;
};

#endif // !SALA_H
