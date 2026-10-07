#include "persona.h"
#include <stdexcept>

Persona::Persona(std::string nombre, int edad) {
    setNombre(nombre);
    setEdad(edad);
}

const std::string& Persona::getNombre() const {
    return nombre;
}

int Persona::getEdad() const {
    return edad;
}

void Persona::setNombre(std::string nombre) {
    if (nombre.empty()) {
        throw std::invalid_argument("El nombre no puede estar vacio");
    }
    this->nombre = nombre;
}

void Persona::setEdad(int edad) {
    if (edad < 0) {
        throw std::invalid_argument("La edad no puede ser negativa");
    }
    this->edad = edad;
}
