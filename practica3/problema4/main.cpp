#include "raices.h"
#include <iostream>
#include <thread>
#include <functional>

int main(){
  double a, b, c;

  std::cout << "Ingrese el valor de a: ";
  std::cin >> a;
  std::cout << "Ingrese el valor de b: ";
  std::cin >> b;
  std::cout << "Ingrese el valor de c: ";
  std::cin >> c;

  Raices raices(a, b, c);

  std::thread t1([&raices](){
    double raiz1 = raices.Raiz1();
    if (raiz1 == 0){
      std::cout << "No hay raiz 1" << std::endl;
    } else {
      std::cout << "Raiz 1: " << raiz1 << std::endl;
    }
  });
  t1.join();
  
  std::thread t2([&raices](){
    double raiz2 = raices.Raiz2();
    if (raiz2 == 0){
      std::cout << "No hay raiz 2" << std::endl;
    } else {
      std::cout << "Raiz 2: " << raiz2 << std::endl;
    }
  });

  t2.join();

  return 0;
}
