#ifndef ASIENTO_H
#define ASIENTO_H

#include <optional>
#include "persona.h"

class Asiento {
  private:
    int numero;
    std::optional<Persona> ocupante;

  public:
    explicit Asiento(int numero);

    int getNumero() const;
    bool isOcupado() const;

    bool ocupar(const Persona& persona);
    bool liberar();

    bool perteneceA(const std::string& nombre) const;
    std::optional<Persona> getOcupante() const;
};

#endif // !ASIENTO_H
