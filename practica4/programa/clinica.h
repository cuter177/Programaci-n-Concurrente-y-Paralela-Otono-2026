#ifndef CLINICA_H
#define CLINICA_H

#include <string>
#include <sstream>
#include <vector>
#include "paciente.h"
#include "vacuna.h"
#include <mutex>

class Clinica {
  private:
    std::vector<Paciente> pacientes;
    std::tm fechaActual;
    mutable std::mutex mtx;

    void cargarArchivo(const std::string& ruta);
    void guardarArchivo(const std::string& ruta);
    void setFechaSistema();

    // Helpers internos
    int buscarPorNombreSinLock(const std::string& nombre);
    int buscarPorFechaNSinLock(int dia, int mes, int anio);

  public:
    Clinica();
    ~Clinica();
    void setFechaActual(int dia, int mes, int anio);
    std::tm getFechaActual();
    void agregarPaciente(const Paciente& paciente);

    // Devuelve el indice del paciente con ese id, o -1 si no existe.
    int buscarPorId(int idPaciente);

    int buscarPorNombre(const std::string& nombre);

    int buscarPorFechaN(int dia, int mes, int anio);

    // Devuelve el id del paciente en esa posicion del arreglo, o -1 si el indice esta fuera de rango.
    int idEnIndice(int indice);

    // Registra la aplicacion de una vacuna al paciente con ese id.
    // Devuelve true si encontro al paciente, false en caso contrario.
    bool registrarAplicacionVacuna(int idPaciente, const Vacuna& vacuna);

    bool agengarVacunaProxima(int idPaciente, const Vacuna& vacuna);
    bool eliminarPaciente(int idPaciente);

    bool modificarDatos(int idPaciente, std::string nuevoNomre);
    bool modificarDatos(int idPaciente, int dia, int mes, int anio);

    void mostrarInfoPaciente(int idPaciente);

    bool eliminarVacunaDePaciente(const std::string& nombrePaciente, const std::string& nombreVacuna);
    bool modificarVacunaDePaciente(const std::string& nombrePaciente, const std::string& nombreVacuna, const Vacuna& nueva);

    void mostrarNombresAlfabetico();

    void mostrarTodo5en5();
    void buscarRangoEdad(int min, int max);

    void pacientesConVacunaProxima7Dias(const std::string& nombreVacuna);

    int contarVacunasAplicadaSemana();

    int contarVacunasdelsiguenteMes(const std::string& nombreVacuna);

    void pacientesVacunadosdelMes();


};

#endif // !CLINICA_H
