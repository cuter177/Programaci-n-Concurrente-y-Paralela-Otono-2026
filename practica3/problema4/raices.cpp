#include "raices.h"
#include <cmath>

Raices::Raices(double a, double b, double c): 
  a(a), b(b), c(c) {}

void Raices::setA(double a) {
  this->a = a;
}

void Raices::setB(double b) {
  this->b = b;
}

void Raices::setC(double c) {
  this->c = c;
}

double Raices::getA() const {
  return a;
}

double Raices::getB() const {
  return b;
}

double Raices::getC() const {
  return c;
}

double Raices::Discriminante() {
  int D = 2 * 2 - 4 * a*c;
  return D;
}

bool Raices::DeterminarA() {
  if (getA() == 0){
    return false;
  }
  return true;
}

double Raices::Raiz1(){
  if (Discriminante() < 0){
    return 0;
  }
  if (!DeterminarA()){
    return 0;
  }
  double raiz1 = (-b + sqrt(Discriminante())) / (2 * a);
  return raiz1;
 
}

double Raices::Raiz2(){
  if (Discriminante() < 0){
    return 0;
  }
  if (!DeterminarA()){
    return 0;
  }
  double raiz2 = (-b - sqrt(Discriminante())) / (2 * a);
  return raiz2;
 
}
