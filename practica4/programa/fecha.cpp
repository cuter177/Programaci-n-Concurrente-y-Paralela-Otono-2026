#include "fecha.h"
#include <cmath>

namespace {
  const int ANIO_MIN = 1900;
  const int ANIO_MAX = 2100;
  const int SEG_POR_DIA = 60 * 60 * 24;
}

bool esFechaValida(int dia, int mes, int anio){
  if (anio < ANIO_MIN || anio > ANIO_MAX) {
    return false;
  }
  if (mes < 1 || mes > 12) {
    return false;
  }

  int diasDelMes;

  switch (mes) {
    case 1:  // enero
    case 3:  // marzo
    case 5:  // mayo
    case 7:  // julio
    case 8:  // agosto
    case 10: // octubre
    case 12: // diciembre
      diasDelMes = 31;
      break;
    case 4:  // abril
    case 6:  // junio
    case 9:  // septiembre
    case 11: // noviembre
      diasDelMes = 30;
      break;
    case 2:  // febrero
      // Año bisiesto: divisible entre 4, excepto los múltiplos de 100
      // que no sean múltiplos de 400.
      if ((anio % 4 == 0 && anio % 100 != 0) || (anio % 400 == 0)) {
        diasDelMes = 29;
      } else {
        diasDelMes = 28;
      }
      break;
    default:
      return false;
  }

  // El dia existe si está dentro del rango de dias de ese mes.
  return dia >= 1 && dia <= diasDelMes;
}

time_t aTimeT(const std::tm& f){
  std::tm copia = f; // mktime puede modificar el tm, trabajamos sobre una copia
  return std::mktime(&copia);
}

long diasDiferencia(const std::tm& desde, const std::tm& hasta){
  double segundos = std::difftime(aTimeT(hasta), aTimeT(desde));
  return std::lround(segundos / SEG_POR_DIA);
}

int calcularEdad(const std::tm& nacimiento, const std::tm& hoy){
  // tm_year y tm_mon vienen en la misma escala en ambas fechas,
  // asi que basta comparar los campos.
  int edad = hoy.tm_year - nacimiento.tm_year;
  if (hoy.tm_mon < nacimiento.tm_mon ||
      (hoy.tm_mon == nacimiento.tm_mon && hoy.tm_mday < nacimiento.tm_mday)) {
    edad--; // aun no cumple años este año
  }
  return edad;
}

bool esMismoMes(const std::tm& a, const std::tm& b){
  return a.tm_year == b.tm_year && a.tm_mon == b.tm_mon;
}
