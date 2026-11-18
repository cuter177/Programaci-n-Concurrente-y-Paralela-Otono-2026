#include "clinica.h"
#include "fecha.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>
#include <ctime>
#include <vector>
#include "paciente.h"
#include "utils.h"
#include "vacuna.h"
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
        // vacuna invalida
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

void Clinica::setFechaActual(int dia, int mes, int anio){

  if(!esFechaValida(dia, mes, anio)){
    throw std::invalid_argument("Fecha actual Invalida"); 
  }

  fechaActual = {};
  fechaActual.tm_mday = dia;
  fechaActual.tm_mon = mes - 1;
  fechaActual.tm_year = anio - 1900;

}

std::tm Clinica::getFechaActual(){
  return fechaActual;
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
    archivo << formatoPaciente(pacientes[i]) << "\n";
  }
}


void Clinica::agregarPaciente(const Paciente& paciente){
  std::lock_guard<std::mutex> lock(mtx);
  // Inserta ordenado por id para que Utils::busquedaBinaria siga siendo valida
  // aunque los pacientes se registren desde varios hilos.
  int nuevoId = paciente.getId();
  size_t pos = 0;
  while (pos < pacientes.size() && pacientes[pos].getId() < nuevoId)
    ++pos;
  pacientes.insert(pacientes.begin() + pos, paciente);
}

int Clinica::buscarPorId(int idPaciente){
  std::lock_guard<std::mutex> lock(mtx);
  return Utils::busquedaBinaria(idPaciente, pacientes);
}

int Clinica::buscarPorNombreSinLock(const std::string& nombre){
  for (size_t i = 0; i < pacientes.size(); ++i){
    if (Utils::sonIguales(pacientes[i].getNombre(), nombre))
      return static_cast<int>(i);
  }
  return -1;
}

int Clinica::buscarPorNombre(const std::string& nombre){
  std::lock_guard<std::mutex> lock(mtx);
  return buscarPorNombreSinLock(nombre);         
}

int Clinica::buscarPorFechaNSinLock(int dia, int mes, int anio){
  for (size_t i = 0; i < pacientes.size(); ++i){
    std::tm f = pacientes[i].getFechaNacimiento();
    // ojo: tm_mon va de 0 a 11 y tm_year desde 1900
    if (f.tm_mday == dia && f.tm_mon == mes - 1 && f.tm_year == anio - 1900)
      return static_cast<int>(i);
  }
  return -1;
}

int Clinica::buscarPorFechaN(int dia, int mes, int anio){
  std::lock_guard<std::mutex> lock(mtx);
  return buscarPorFechaNSinLock(dia, mes, anio);
}

int Clinica::idEnIndice(int indice){
  std::lock_guard<std::mutex> lock(mtx);
  if (indice < 0 || static_cast<size_t>(indice) >= pacientes.size())
    return -1;
  return pacientes[indice].getId();
}

bool Clinica::registrarAplicacionVacuna(int idPaciente, const Vacuna& vacuna){
  std::lock_guard<std::mutex> lock(mtx);
  int i = Utils::busquedaBinaria(idPaciente,pacientes);

  if(i >= 0){
    pacientes[i].agregarVacunaRecibida(vacuna);
    return true;
  }

  return false;
}

bool Clinica::agengarVacunaProxima(int idPaciente, const Vacuna& vacuna){
  std::lock_guard<std::mutex> lock(mtx);
  int i = Utils::busquedaBinaria(idPaciente, pacientes);

  if(i >= 0){
    pacientes[i].agregarVacunaProxima(vacuna);
    return true;
  }

  return false;
}

bool Clinica::eliminarPaciente(int idPaciente){
  std::lock_guard<std::mutex> lock(mtx);
  int i = Utils::busquedaBinaria(idPaciente, pacientes);

  if (i < 0) 
    return false;
  
  pacientes.erase(pacientes.begin() + i);
  return true;
}

bool Clinica::modificarDatos(int idPaciente, std::string nuevoNomre){
  std::lock_guard<std::mutex> lock(mtx);
  int i = Utils::busquedaBinaria(idPaciente, pacientes);

  if(i < 0)
    return false;

  pacientes[i].setNombre(nuevoNomre);
  return true;
}

bool Clinica::modificarDatos(int idPaciente, int dia, int mes, int anio){
  std::lock_guard<std::mutex> lock(mtx);
  int i = Utils::busquedaBinaria(idPaciente, pacientes);

  if(i < 0)
    return false;

  pacientes[i].setFechaNacimiento(dia, mes, anio);
  return true;
}

void Clinica::mostrarInfoPaciente(int idPaciente){
  std::lock_guard<std::mutex> lock(mtx);
  int i = Utils::busquedaBinaria(idPaciente, pacientes);

  if(i < 0)
    return;

  std::cout << pacientes[i].toString() << "\n";
}


 bool Clinica::eliminarVacunaDePaciente(const std::string& nombrePaciente, const std::string& nombreVacuna){

   std::lock_guard<std::mutex> lock(mtx);
   int i = buscarPorNombreSinLock(nombrePaciente);

   if(i < 0)
     return false;

   pacientes[i].eliminarVacunaRecibida(nombreVacuna);
   return true;
 }

 bool Clinica::modificarVacunaDePaciente(const std::string& nombrePaciente, const std::string& nombreVacuna, const Vacuna& nueva){
   std::lock_guard<std::mutex> lock(mtx);
   int i = buscarPorNombreSinLock(nombrePaciente);

   if(i < 0)
     return false;

   pacientes[i].modificarVacunaRecibida(nombreVacuna, nueva);
   return true;
 }

 void Clinica::mostrarNombresAlfabetico(){
   std::lock_guard<std::mutex> lock(mtx);

   std::vector<std::string> nombres;

   for(size_t i = 0; i < pacientes.size(); ++i){
     nombres.push_back(pacientes[i].getNombre()); 
   }

   std::sort(nombres.begin(), nombres.end(), [](const std::string& a, const std::string& b){
       return Utils::aMayusculas(a) < Utils::aMayusculas(b);  
      });

   for(size_t i = 0; i < nombres.size(); ++i)
     std::cout << nombres[i] << "\n";
   
 }

void Clinica::mostrarTodo5en5(){
  std::lock_guard<std::mutex> lock(mtx);

  const size_t TAM_PAGINA = 5;
  for (size_t i = 0; i < pacientes.size(); ++i){
    std::cout << pacientes[i].toString() << "\n";

    if ((i + 1) % TAM_PAGINA == 0 && i + 1 < pacientes.size()){
      std::cout << "Presione Enter para continuar...";
      std::cin.get();
    }
  }
}

void Clinica::buscarRangoEdad(int min, int max){
  std::lock_guard<std::mutex> lock(mtx);

  std::tm hoy = getFechaActual();
  std::vector<Paciente> pacientesEnRango;

  for (size_t i = 0; i < pacientes.size(); ++i){
    int edad = calcularEdad(pacientes[i].getFechaNacimiento(), hoy);
    if (edad >= min && edad <= max)
      pacientesEnRango.push_back(pacientes[i]); 
  }

  std::sort(pacientesEnRango.begin(), pacientesEnRango.end(),
  [&hoy](const Paciente& a, const Paciente& b){
    return calcularEdad(a.getFechaNacimiento(), hoy) >
           calcularEdad(b.getFechaNacimiento(), hoy);
  });

  for (size_t k = 0; k < pacientesEnRango.size(); ++k)
    std::cout << pacientesEnRango[k].toString() << "\n";
}

void Clinica::pacientesConVacunaProxima7Dias(const std::string& nombreVacuna){
  std::lock_guard<std::mutex> lock(mtx);

  std::tm hoy = getFechaActual();
  std::vector<std::string> nombres;

  for (size_t i = 0; i < pacientes.size(); ++i){
    std::vector<Vacuna> proximas = pacientes[i].getVacunasProximas(); 
    bool aplica = false;

    for (size_t j = 0; j < proximas.size(); ++j){
      if (!Utils::sonIguales(proximas[j].getNombre(), nombreVacuna))
        continue;

      long d = diasDiferencia(hoy, proximas[j].getFechaAplicacion());  
      if (d >= 0 && d <= 7){        // dentro del rango
        aplica = true;
        break;     
      }
    }

    if (aplica)
      nombres.push_back(pacientes[i].getNombre());
  }

  std::sort(nombres.begin(), nombres.end(),
    [](const std::string& a, const std::string& b){
      return Utils::aMayusculas(a) < Utils::aMayusculas(b);
    });

  for (size_t k = 0; k < nombres.size(); ++k)
    std::cout << nombres[k] << "\n";
}

 int Clinica::contarVacunasAplicadaSemana(){
   std::lock_guard<std::mutex> lock(mtx);

   std::tm hoy = getFechaActual();
   int contador = 0;

   for(size_t i = 0; i < pacientes.size(); ++i){
     std::vector<Vacuna> recibidas = pacientes[i].getVacunasRecibidas();
     
     for(size_t j = 0; j < recibidas.size(); ++j){
       long d = diasDiferencia(recibidas[j].getFechaAplicacion(), hoy);
       if(d >= 0 && d <= 7)
         contador = contador + 1;
     }
   }
   return contador;

 }

int Clinica::contarVacunasdelsiguenteMes(const std::string& nombreVacuna){
  std::lock_guard<std::mutex> lock(mtx);

  std::tm hoy = getFechaActual();

  std::tm siguiente = hoy;
  siguiente.tm_mday = 1;             
  siguiente.tm_mon  = hoy.tm_mon + 1; 
  std::mktime(&siguiente);

  int contador = 0;

  for (size_t i = 0; i < pacientes.size(); ++i){
    std::vector<Vacuna> proximas = pacientes[i].getVacunasProximas();

    for (size_t j = 0; j < proximas.size(); ++j){

      if (Utils::sonIguales(proximas[j].getNombre(), nombreVacuna) &&
          esMismoMes(proximas[j].getFechaAplicacion(), siguiente))
        contador++;
    }
  }

  return contador;
}

void Clinica::pacientesVacunadosdelMes(){
  std::lock_guard<std::mutex> lock(mtx);

  std::tm hoy = getFechaActual();

  std::vector<size_t> indices;
  for (size_t i = 0; i < pacientes.size(); ++i){
    std::vector<Vacuna> recibidas = pacientes[i].getVacunasRecibidas();

    for (size_t j = 0; j < recibidas.size(); ++j){

      if (esMismoMes(recibidas[j].getFechaAplicacion(), hoy)){
        indices.push_back(i);
        break;                       
      }
    }
  }

  std::sort(indices.begin(), indices.end(),
    [this](size_t a, size_t b){
      return Utils::aMayusculas(pacientes[a].getNombre()) <
             Utils::aMayusculas(pacientes[b].getNombre());
    });

  for (size_t k = 0; k < indices.size(); ++k){
    size_t i = indices[k];
    int edad = calcularEdad(pacientes[i].getFechaNacimiento(), hoy);

    std::cout << pacientes[i].getNombre() << " (" << edad << " anios)\n";

    std::vector<Vacuna> recibidas = pacientes[i].getVacunasRecibidas();
    for (size_t j = 0; j < recibidas.size(); ++j){
      if (esMismoMes(recibidas[j].getFechaAplicacion(), hoy))
        std::cout << "  " << recibidas[j].toString() << "\n";
    }
  }
}
