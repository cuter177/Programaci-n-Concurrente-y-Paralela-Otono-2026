#include "utils.h"
#include <algorithm>
#include <cctype>

std::string Utils::aMayusculas(const std::string& texto){
  std::string resultado = texto;
  std::transform(resultado.begin(), resultado.end(), resultado.begin(),
    [](unsigned char c){ return std::toupper(c); });
  return resultado;
}

std::string Utils::aMinusculas(const std::string& texto){
  std::string resultado = texto;
  std::transform(resultado.begin(), resultado.end(), resultado.begin(),
    [](unsigned char c){ return std::tolower(c); });
  return resultado;
}

bool Utils::sonIguales(const std::string& a, const std::string& b){
  return aMayusculas(a) == aMayusculas(b);
}

int Utils::busquedaBinaria(int buscado, std::vector<Paciente> pacientes ){
  int inicio = 0;
  int final = static_cast<int>(pacientes.size()) - 1;
  int medio;

  while(inicio <= final){
    medio = inicio + (final - inicio) / 2;
    int idMedio = pacientes[medio].getId();
    if (buscado == idMedio)
      return medio;
    
    else if (pacientes[medio].getId() < buscado)
      inicio = medio + 1;

    else 
      final = medio - 1;
  }
  return -1;
}
