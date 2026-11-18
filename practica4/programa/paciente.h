#ifndef PACIENTE_H
#define PACIENTE_H

#include <string>
#include <ctime>
#include <vector>
#include "vacuna.h"


class Paciente{
  private:
    int id;
    std::string nombre;
    std::tm fechaNacimiento;
    std::vector<Vacuna> vacunasRecibidas;
    std::vector<Vacuna> vacunasProximas;

    static int contadorId; // lleva la cuenta para asignar ids unicos
  public:
    Paciente(std::string nombre, int dia, int mes, int anio);
    int getId() const;
    void setNombre(std::string nombre);
    std::string getNombre() const;
    void setFechaNacimiento(int dia, int mes, int anio);
    std::tm getFechaNacimiento() const;
    void setVacunasRecibidas(std::vector<Vacuna> vacunasRecibidas);
    std::vector<Vacuna> getVacunasRecibidas() const;
    void setVacunasProximas(std::vector<Vacuna> vacunasProximas);
    std::vector<Vacuna> getVacunasProximas() const;

    void agregarVacunaRecibida(const Vacuna& vacuna);
    void agregarVacunaProxima(const Vacuna& vacuna);

    bool eliminarVacunaRecibida(const std::string& nombreVacuna);
    bool eliminarVacunaProxima(const std::string& nombreVacuna);

    bool modificarVacunaRecibida(const std::string& nombreBuscado, const Vacuna& nueva);
    bool modificarVacunaProxima(const std::string& nombreBuscado, const Vacuna& nueva);

    std::string toString() const;

};


#endif // !PACIENTE_H
