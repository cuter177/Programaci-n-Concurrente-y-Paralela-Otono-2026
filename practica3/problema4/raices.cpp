#include "raices.h"
#include <cmath>
#include <thread>

Raices::Raices(double a, double b, double c)
    : a(a), b(b), c(c), b2(0), ac4(0), D(0), r(0), x1(0), x2(0),
      hay_raices(false) {}

void Raices::calcular() {
    // COBEGIN-COEND: T1 (b2 = b*b) y T2 (ac4 = 4*a*c) son independientes.
    std::thread t1([this]() { b2 = b * b; });
    std::thread t2([this]() { ac4 = 4 * a * c; });
    t1.join();
    t2.join();

    // Punto de union obligatorio: D depende de b2 y ac4.
    D = b2 - ac4;

    if (D < 0)
        return;

    r = std::sqrt(D);

    // COBEGIN-COEND: T5 (x1) y T6 (x2) solo leen a, b, r y escriben variables
    // distintas, por lo que son independientes.
    std::thread t3([this]() { x1 = (-b + r) / (2 * a); });
    std::thread t4([this]() { x2 = (-b - r) / (2 * a); });
    t3.join();
    t4.join();

    hay_raices = true;
}

bool Raices::hayRaices() const {
    return hay_raices;
}

double Raices::getD() const {
    return D;
}

double Raices::getX1() const {
    return x1;
}

double Raices::getX2() const {
    return x2;
}
