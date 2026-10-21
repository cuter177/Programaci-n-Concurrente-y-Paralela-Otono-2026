#include "asiento.h"

Asiento::Asiento(int numero) : numero(numero), ocupante(std::nullopt) {}

int Asiento::getNumero() const {
    return numero;
}

bool Asiento::isOcupado() const {
    return ocupante.has_value();
}

bool Asiento::ocupar(const Persona& persona) {
    if (ocupante.has_value()) {
        return false;
    }
    ocupante = persona;
    return true;
}

bool Asiento::liberar() {
    if (!ocupante.has_value()) {
        return false;
    }
    ocupante.reset();
    return true;
}

bool Asiento::perteneceA(const std::string& nombre) const {
    return ocupante.has_value() && ocupante->getNombre() == nombre;
}

std::optional<Persona> Asiento::getOcupante() const {
    return ocupante;
}
