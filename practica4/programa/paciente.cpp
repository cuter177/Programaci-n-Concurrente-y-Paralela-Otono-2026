#include "paciente.h"
#include "fecha.h"
#include "utils.h"
#include <string>
#include <ctime>
#include <vector>
#include <stdexcept>
#include <sstream>

int Paciente::contadorId = 0; // se inicializa una sola vez para toda la clase

Paciente::Paciente(std::string nombre, int dia, int mes, int anio){
  this->id = Paciente::contadorId++; // primer id = 0, luego 1, 2, ...
  setNombre(nombre);
  setFechaNacimiento(dia,mes,anio);
}

int Paciente::getId() const{
  return id;
}

void Paciente::setNombre(std::string nombre){
  this->nombre = nombre;
}

std::string Paciente::getNombre() const{
  return nombre;
}

void Paciente::setFechaNacimiento(int dia, int mes, int anio){
  if (!esFechaValida(dia, mes, anio)) {
    throw std::invalid_argument("Fecha de nacimiento invalida");
  }
  fechaNacimiento = {};
  fechaNacimiento.tm_mday = dia;
  fechaNacimiento.tm_mon = mes - 1;
  fechaNacimiento.tm_year = anio - 1900;
}

std::tm Paciente::getFechaNacimiento() const{
  return fechaNacimiento;
}

void Paciente::setVacunasRecibidas(std::vector<Vacuna> vacunasRecibidas){
  this->vacunasRecibidas = vacunasRecibidas;
}

void Paciente::setVacunasProximas(std::vector<Vacuna> vacunasProximas){
  this->vacunasProximas = vacunasProximas;
}

std::vector<Vacuna> Paciente::getVacunasRecibidas() const{
  return vacunasRecibidas;
}

std::vector<Vacuna> Paciente::getVacunasProximas() const{
  return vacunasProximas;
}

void Paciente::agregarVacunaRecibida(const Vacuna& vacuna){
  vacunasRecibidas.push_back(vacuna);
}

void Paciente::agregarVacunaProxima(const Vacuna& vacuna){
  vacunasProximas.push_back(vacuna);
}

bool Paciente::eliminarVacunaRecibida(const std::string& nombreVacuna){
  for (size_t i = 0; i < vacunasRecibidas.size(); ++i){
    if (Utils::sonIguales(vacunasRecibidas[i].getNombre(), nombreVacuna)) {
      vacunasRecibidas.erase(vacunasRecibidas.begin() + i);
      return true;
    }
  }
  return false;
}

bool Paciente::eliminarVacunaProxima(const std::string& nombreVacuna){
  for (size_t i = 0; i < vacunasProximas.size(); ++i){
    if (Utils::sonIguales(vacunasProximas[i].getNombre(), nombreVacuna)) {
      vacunasProximas.erase(vacunasProximas.begin() + i);
      return true;
    }
  }
  return false;
}

bool Paciente::modificarVacunaRecibida(const std::string& nombreBuscado, const Vacuna& nueva){
  for (size_t i = 0; i < vacunasRecibidas.size(); ++i){
    if (Utils::sonIguales(vacunasRecibidas[i].getNombre(), nombreBuscado)) {
      vacunasRecibidas[i] = nueva;
      return true;
    }
  }
  return false;
}

bool Paciente::modificarVacunaProxima(const std::string& nombreBuscado, const Vacuna& nueva){
  for (size_t i = 0; i < vacunasProximas.size(); ++i){
    if (Utils::sonIguales(vacunasProximas[i].getNombre(), nombreBuscado)) {
      vacunasProximas[i] = nueva;
      return true;
    }
  }
  return false;
}

std::string Paciente::toString() const{
  std::ostringstream os;
  char buffer[11];
  std::strftime(buffer, sizeof(buffer), "%d/%m/%Y", &fechaNacimiento);
  os << "Id: " << id
     << ", Nombre: " << nombre
     << ", Fecha de nacimiento: " << buffer << "\n";
  os << "Vacunas recibidas:\n";
  for (size_t i = 0; i < vacunasRecibidas.size(); ++i) {
    os << "  " << vacunasRecibidas[i].toString() << "\n";
  }
  os << "Vacunas proximas:\n";
  for (size_t i = 0; i < vacunasProximas.size(); ++i) {
    os << "  " << vacunasProximas[i].toString() << "\n";
  }
  return os.str();
}


