
#include "clinica.h"
#include "fecha.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <ctime>

namespace {

  std::mutex g_out;   // protege la salida por consola entre hilos

  void say(const std::string& msg){
    std::lock_guard<std::mutex> lock(g_out);
    std::cout << msg << "\n";
  }

  void titulo(const std::string& t){
    say("\n===== " + t + " =====");
  }

  std::string fechaTexto(const std::tm& f){
    char buf[11];
    std::strftime(buf, sizeof(buf), "%d/%m/%Y", &f);
    return std::string(buf);
  }

}

int main(){
  try {
    Clinica clinica;   // al construirse carga pacientes.txt

    clinica.setFechaActual(13, 10, 2026);
    say("Fecha de trabajo: " + fechaTexto(clinica.getFechaActual()));

    std::vector<Paciente> nuevos;
    nuevos.push_back(Paciente("Ana Lopez",  10,  5, 2015));
    nuevos.push_back(Paciente("Beto Ruiz",  20, 11, 2018));
    nuevos.push_back(Paciente("Carla Diaz",  1,  1, 2022));
    nuevos.push_back(Paciente("Diego Soto", 15,  6, 2019));
    nuevos.push_back(Paciente("Elena Vega",  5,  3, 2021));

    // Nombre final de cada paciente (Ana se renombra mas adelante). Sirve para
    // no volver a agregarlo si ya existe y que el demo se pueda repetir.
    const std::vector<std::string> nombreFinal = {
      "Ana Lopez Gomez", "Beto Ruiz", "Carla Diaz", "Diego Soto", "Elena Vega"
    };

    std::vector<int>  ids(nuevos.size(), -1);       // id de cada paciente
    std::vector<char> esNuevo(nuevos.size(), 0);    // 1 si se agrega en esta corrida

    titulo("alta de pacientes (5 hilos)");
    {
      std::vector<std::thread> hilos;
      for (size_t k = 0; k < nuevos.size(); ++k){
        hilos.push_back(std::thread([&clinica, &nuevos, &nombreFinal, &ids, &esNuevo, k](){
          int idx = clinica.buscarPorNombre(nombreFinal[k]);
          if (idx >= 0){
            ids[k] = clinica.idEnIndice(idx);       // ya existia: no duplicar
            say("  [hilo] ya existia: " + nombreFinal[k]);
          } else {
            clinica.agregarPaciente(nuevos[k]);     // se inserta ordenado por id
            ids[k] = nuevos[k].getId();             // id directo, sin buscar por nombre
            esNuevo[k] = 1;
            say("  [hilo] alta: " + nuevos[k].getNombre());
          }
        }));
      }
      for (size_t k = 0; k < hilos.size(); ++k) hilos[k].join();
    }

    int idAna   = ids[0];
    int idBeto  = ids[1];
    int idCarla = ids[2];
    int idDiego = ids[3];
    int idElena = ids[4];

    titulo("vacunas y modificaciones (hilos)");
    {
      std::vector<std::thread> hilos;

      // Las vacunas solo se registran si el paciente se acaba de agregar, para
      // que repetir el demo no acumule vacunas repetidas.
      if (esNuevo[0]) hilos.push_back(std::thread([&](){
        clinica.registrarAplicacionVacuna(idAna, Vacuna("triple viral", 10, 10, 2026));
        clinica.registrarAplicacionVacuna(idAna, Vacuna("varicela", 1, 9, 2026));
        say("  [hilo] aplicaciones a Ana");
      }));
      if (esNuevo[1]) hilos.push_back(std::thread([&](){
        clinica.registrarAplicacionVacuna(idBeto, Vacuna("hepatitis B", 12, 10, 2026));
        clinica.registrarAplicacionVacuna(idBeto, Vacuna("influenza", 15, 3, 2026));
        say("  [hilo] aplicaciones a Beto");
      }));
      if (esNuevo[2]) hilos.push_back(std::thread([&](){
        clinica.agengarVacunaProxima(idCarla, Vacuna("tetanos", 18, 10, 2026));
        say("  [hilo] proxima a Carla");
      }));
      if (esNuevo[3]) hilos.push_back(std::thread([&](){
        clinica.agengarVacunaProxima(idDiego, Vacuna("difteria", 5, 11, 2026));
        say("  [hilo] proxima a Diego");
      }));
      hilos.push_back(std::thread([&](){
        clinica.modificarDatos(idAna, "Ana Lopez Gomez");
        clinica.modificarDatos(idBeto, 21, 11, 2018);
        say("  [hilo] datos modificados");
      }));
      hilos.push_back(std::thread([&](){
        if (clinica.eliminarPaciente(idElena)) say("  [hilo] Elena eliminada");
      }));

      for (size_t k = 0; k < hilos.size(); ++k) hilos[k].join();
    }

    titulo(" consultas");

    say("buscarPorId(idAna)               -> indice " + std::to_string(clinica.buscarPorId(idAna)));
    say("buscarPorNombre(Ana Lopez Gomez) -> indice " + std::to_string(clinica.buscarPorNombre("Ana Lopez Gomez")));
    say("buscarPorFechaN(27/09/2020)      -> indice " + std::to_string(clinica.buscarPorFechaN(27, 9, 2020)));

    say("\n-- modificar / eliminar vacuna por nombre de paciente --");
    say("eliminar varicela de Ana: " +
        std::string(clinica.eliminarVacunaDePaciente("Ana Lopez Gomez", "varicela") ? "ok" : "no"));
    say("modificar influenza de Beto: " +
        std::string(clinica.modificarVacunaDePaciente("Beto Ruiz", "influenza", Vacuna("influenza", 16, 3, 2026)) ? "ok" : "no"));

    titulo("mostrarInfoPaciente(idBeto)");
    clinica.mostrarInfoPaciente(idBeto);

    titulo("mostrarNombresAlfabetico()");
    clinica.mostrarNombresAlfabetico();

    titulo("buscarRangoEdad(0, 120)");
    clinica.buscarRangoEdad(0, 120);

    titulo("pacientesConVacunaProxima7Dias(tetanos)");
    clinica.pacientesConVacunaProxima7Dias("tetanos");

    titulo("conteos");
    say("contarVacunasAplicadaSemana()         = " + std::to_string(clinica.contarVacunasAplicadaSemana()));
    say("contarVacunasdelsiguenteMes(difteria) = " + std::to_string(clinica.contarVacunasdelsiguenteMes("difteria")));

    titulo("pacientesVacunadosdelMes()");
    clinica.pacientesVacunadosdelMes();

    titulo("utilidades (Paciente / Vacuna / Utils / fecha)");
    Paciente demo("Paciente Demo", 1, 1, 2000);
    demo.agregarVacunaProxima(Vacuna("tos ferina", 1, 1, 2026));
    demo.modificarVacunaProxima("tos ferina", Vacuna("tos ferina", 2, 2, 2026));
    demo.eliminarVacunaProxima("tos ferina");
    say("demo.getId() = " + std::to_string(demo.getId()));
    say("Utils::sonIguales(ABC, abc) = " + std::string(Utils::sonIguales("ABC", "abc") ? "true" : "false"));
    say("Utils::aMayusculas(hola)    = " + Utils::aMayusculas("hola"));
    say("calcularEdad(demo)          = " + std::to_string(calcularEdad(demo.getFechaNacimiento(), clinica.getFechaActual())));
    say("esMismoMes(15/10/2026, hoy) = " +
        std::string(esMismoMes(Vacuna("x", 15, 10, 2026).getFechaAplicacion(), clinica.getFechaActual()) ? "true" : "false"));

    titulo("mostrarTodo5en5()");
    clinica.mostrarTodo5en5();

  } catch (const std::exception& e){
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0; 
}
