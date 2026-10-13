#ifndef UTILS_H
#define UTILS_H

#include <string>

// Clase de utilidades reutilizables en todo el programa.
// Los metodos son estaticos: se llaman Utils::metodo(...) sin crear objetos.
class Utils {
  public:
    // Devuelve una copia del texto en MAYUSCULAS (solo ASCII).
    static std::string aMayusculas(const std::string& texto);

    // Devuelve una copia del texto en minusculas (solo ASCII).
    static std::string aMinusculas(const std::string& texto);

    // Compara dos textos ignorando mayusculas/minusculas.
    static bool sonIguales(const std::string& a, const std::string& b);
};

#endif // !UTILS_H
