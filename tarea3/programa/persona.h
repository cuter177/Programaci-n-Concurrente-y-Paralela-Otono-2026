#ifndef PERSONA_H
#define PERSONA_H

#include <string>

class Persona {
  private:
    std::string nombre;
    int edad;

  public:
    Persona(std::string nombre, int edad);

    const std::string& getNombre() const;
    int getEdad() const;

    void setNombre(std::string nombre);
    void setEdad(int edad);
};

#endif // !PERSONA_H
