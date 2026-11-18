#include "vacuna.h"
#include "fecha.h"
#include <string>
#include <ctime>
#include <stdexcept>

Vacuna::Vacuna(std::string nombre, int dia, int mes, int anio){
  setNombre(nombre);
  setFechaAplicacion(dia, mes, anio);
}

void Vacuna::setNombre(std::string nombre){
  this->nombre = nombre;
}

std::string Vacuna::getNombre() const{
  return this->nombre;
}

void Vacuna::setFechaAplicacion(int dia, int mes, int anio){
  if (!esFechaValida(dia, mes, anio)) {
    throw std::invalid_argument("Fecha de aplicacion de vacuna invalida");
  }
  std::tm fecha = {};
  fecha.tm_mday = dia;
  fecha.tm_mon = mes - 1; // Los meses en std::tm van de 0 a 11
  fecha.tm_year = anio - 1900; // Los años en std::tm se cuentan desde 1900
  this->fechaAplicacion = fecha;
}

std::tm Vacuna::getFechaAplicacion() const{
  return this->fechaAplicacion;
}

std::string Vacuna::toString() const{
  char buffer[11];
  std::strftime(buffer, sizeof(buffer), "%d/%m/%Y", &fechaAplicacion);
  return "Vacuna: " + nombre + ", Fecha de aplicación: " + std::string(buffer);
}
