#ifndef FECHA_H
#define FECHA_H

#include <ctime>

bool esFechaValida(int dia, int mes, int anio);

// Convierte una fecha std::tm a time_t (segundos desde el epoch).
time_t aTimeT(const std::tm& f);

// Diferencia en dias: hasta - desde. Negativa si "hasta" es anterior.
long diasDiferencia(const std::tm& desde, const std::tm& hasta);

// Edad en años cumplidos del nacimiento respecto a una fecha de referencia.
int calcularEdad(const std::tm& nacimiento, const std::tm& hoy);

// true si ambas fechas caen en el mismo mes y el mismo año.
bool esMismoMes(const std::tm& a, const std::tm& b);

#endif // !FECHA_H
