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
    int getId();
    void setNombre(std::string nombre);
    std::string getNombre();
    void setFechaNacimiento(int dia, int mes, int anio);
    std::tm getFechaNacimiento();
    void setVacunasRecibidas(std::vector<Vacuna> vacunasRecibidas);
    std::vector<Vacuna> getVacunasRecibidas();
    void setVacunasProximas(std::vector<Vacuna> vacunasProximas);
    std::vector<Vacuna> getVacunasProximas();

    void agregarVacunaRecibida(const Vacuna& vacuna);
    void agregarVacunaProxima(const Vacuna& vacuna);

    void eliminarVacunaRecibida(const std::string& nombreVacuna);
    void eliminarVacunaProxima(const std::string& nombreVacuna);

    bool modificarVacunaRecibida(const std::string& nombreBuscado, const Vacuna& nueva);
    bool modificarVacunaRecibida(const std::tm& fechaBuscada,      const Vacuna& nueva);

    bool modificarVacunaProxima(const std::string& nombreBuscado, const Vacuna& nueva);
    bool modificarVacunaProxima(const std::tm& fechaBuscada,      const Vacuna& nueva);

    bool eliminarVacunaDePaciente(const std::string& nombrePaciente, const std::string& nombreVacuna);
    bool modificarVacunaDePaciente(const std::string& nombrePaciente, const std::string& nombreVacuna);

    std::string toString();

};


#endif // !PACIENTE_H
