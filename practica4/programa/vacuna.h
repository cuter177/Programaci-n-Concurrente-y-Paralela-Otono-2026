#ifndef VACUNAS_H
#define VACUNAS_H

#include <string>
#include <ctime>

class Vacuna{
  private:
    std::string nombre;
    std::tm fechaAplicacion;

  public:
    Vacuna(std::string nombre, int dia, int mes, int anio);
    void setNombre(std::string nombre);
    std::string getNombre() const;
    void setFechaAplicacion(int dia, int mes, int anio);
    std::tm getFechaAplicacion() const;
    std::string toString() const;
};


#endif // !VACUNAS_H

