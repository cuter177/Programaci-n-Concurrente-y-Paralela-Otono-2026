#include "clinica.h"
#include "fecha.h"
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <ctime>
#include <vector>
#include "paciente.h"
#include <mutex>

namespace  {
  const std::string ARCHIVO = "pacientes.txt";
    std::string trim(const std::string& s){
      size_t ini = s.find_first_not_of(" \t\r\n");
      if (ini == std::string::npos) return "";
      size_t fin = s.find_last_not_of(" \t\r\n");
      return s.substr(ini, fin - ini + 1);
    }

  // separa conservando campos vacios 
  std::vector<std::string> split(const std::string& s, char delim){
    std::vector<std::string> partes;
    size_t ini = 0, pos;
    while ((pos = s.find(delim, ini)) != std::string::npos){
      partes.push_back(s.substr(ini, pos - ini));
      ini = pos + 1;
    }
    partes.push_back(s.substr(ini));
    return partes;
  }

  // dd/mm/aaaa  dia/mes/anio
  bool parseFecha(const std::string& texto, int& dia, int& mes, int& anio){
    std::vector<std::string> p = split(trim(texto), '/');
    if (p.size() != 3) return false;
    try {
      dia  = std::stoi(trim(p[0]));
      mes  = std::stoi(trim(p[1]));
      anio = std::stoi(trim(p[2]));
    } catch (...) {
      return false;
    }
    return esFechaValida(dia, mes, anio);
  }

  // formato de las vacunas  triple viral, 14/12/2022;   tétanos, 22/10/2027, difteria, 07/11/2027.
  void cargarListaVacunas(Paciente& p, const std::string& lista, bool recibidas){
    std::vector<std::string> tokens = split(lista, ',');
    for (size_t i = 0; i + 1 < tokens.size(); i += 2){   // de 2 en 2
      std::string nombre = trim(tokens[i]);
      int d, m, a;
      if (nombre.empty() || !parseFecha(tokens[i + 1], d, m, a)) continue;
      try {
        Vacuna v(nombre, d, m, a);
        if (recibidas) p.agregarVacunaRecibida(v);
        else           p.agregarVacunaProxima(v);
      } catch (const std::invalid_argument&) {
        // vacuna con fecha fuera de rango: se ignora
      }
    }
  }

  std::string formatoFecha(const std::tm& f){
    char buf[11];
    std::strftime(buf, sizeof(buf), "%d/%m/%Y", &f);
    return std::string(buf);
  }

  std::string formatoLista(const std::vector<Vacuna>& lista){
    std::string r;
    for (size_t i = 0; i < lista.size(); ++i){
      if (i) r += ", ";
      r += lista[i].getNombre() + ", " + formatoFecha(lista[i].getFechaAplicacion());
    }
    return r;
  }

  std::string formatoPaciente(Paciente& p){  
    return p.getNombre() + "; " + formatoFecha(p.getFechaNacimiento()) + "; "
         + formatoLista(p.getVacunasRecibidas()) + "; "
         + formatoLista(p.getVacunasProximas());
  }

}


Clinica::Clinica(){
  fechaActual = {};
  setFechaSistema();
  cargarArchivo(ARCHIVO);
}

Clinica::~Clinica(){
  guardarArchivo(ARCHIVO);
}

void Clinica::setFechaSistema(){
  std::time_t tiempo = std::time(nullptr);
  std::tm* local = std::localtime(&tiempo);

  setFechaActual(local->tm_mday, local->tm_mon + 1, local->tm_year + 1900);

}

void Clinica::cargarArchivo(const std::string& ruta){
  std::ifstream archivo(ruta);

  if(!archivo.is_open())
    return;

  std::string linea;

  while(std::getline(archivo,linea)){
    linea = trim(linea); //quitar espacios sobrantes

    if (linea.empty()){
      continue;
    }

    std::vector<std::string> campos = split(linea, ';'); //leer los datos separados por ;

    if (campos.size() < 4)
      continue;

    std::string nombre = trim(campos[0]);

    int d, m, a;
    if(nombre.empty() || !parseFecha(campos[1], d, m, a))
      continue;

    try{
      Paciente p(nombre, d, m, a);
      cargarListaVacunas(p, campos[2], true);
      cargarListaVacunas(p, campos[3], false); 
      pacientes.push_back(p);
      
    }catch (const std::invalid_argument&){
      //ignorar
    }
  }
}

void Clinica::guardarArchivo(const std::string& ruta){
  std::ofstream archivo(ruta);                        // abrir para sobreescribir
  if (!archivo.is_open()) return;

  for (size_t i = 0; i < pacientes.size(); ++i){
    archivo << formatoPaciente(pacientes[i]) << "\n"; // linea por linea
  }
}

