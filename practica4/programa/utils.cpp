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
