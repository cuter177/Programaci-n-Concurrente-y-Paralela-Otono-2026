#include "raices.h"
#include <iostream>

int main() {
    double a, b, c;

    std::cout << "Ingrese el valor de a: ";
    std::cin >> a;
    std::cout << "Ingrese el valor de b: ";
    std::cin >> b;
    std::cout << "Ingrese el valor de c: ";
    std::cin >> c;

    if (a == 0) {
        std::cout << "No es una ecuacion cuadratica (a = 0)" << std::endl;
        return 0;
    }

    Raices raices(a, b, c);
    raices.calcular();

    if (!raices.hayRaices()) {
        std::cout << "No hay raices reales (D = " << raices.getD() << ")" << std::endl;
        return 0;
    }

    std::cout << "Discriminante D: " << raices.getD() << std::endl;
    std::cout << "Raiz 1: " << raices.getX1() << std::endl;
    std::cout << "Raiz 2: " << raices.getX2() << std::endl;

    return 0;
}
