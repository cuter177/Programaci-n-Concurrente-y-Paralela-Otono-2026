#include "estudiantes.h"
#include "estudiante.h"
#include <vector>
#include <thread>
#include <random>

int main() {
  Estudiante Juan("Juan Ramirez Bedolla", 20);
  Estudiante Maria("Maria Lopez Castro", 22);
  Estudiante Pedro("Pedro Martinez Sanchez", 21);
  Estudiante Ana("Ana Gonzalez Torres", 19);
  Estudiante Luis("Luis Hernandez Ramirez", 23);
  Estudiante Carlos("Carlos Ramirez Torres", 20);
  Estudiante Sofia("Sofia Martinez Lopez", 22);
  Estudiante Diego("Diego Gonzalez Hernandez", 21);
  Estudiante Valeria("Valeria Torres Ramirez", 19);
  Estudiante Javier("Javier Lopez Martinez", 23);
  Estudiante Fernanda("Fernanda Ramirez Gonzalez", 20);

  std::vector<Estudiante> estudiantes = {Juan, Maria, Pedro, Ana, Luis, Carlos, Sofia, Diego, Valeria, Javier, Fernanda};

  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int> notaAlta(6, 10);
  std::uniform_int_distribution<int> notaBaja(1, 5);

  for (size_t i = 0; i < estudiantes.size(); ++i) {
    Estudiante& estudiante = estudiantes[i];
    bool pasa = (i % 3 != 0);
    auto nota = [&]() { return pasa ? notaAlta(rng) : notaBaja(rng); };

    estudiante.setCalificacionParcial1(nota());
    estudiante.setCalificacionParcial2(nota());
    estudiante.setCalificacionParcial3(nota());
    estudiante.setCalificacionTarea1(nota());
    estudiante.setCalificacionTarea2(nota());
    estudiante.setCalificacionTarea3(nota());
    estudiante.setCalificacionTarea4(nota());
  }

  for (Estudiante& estudiante : estudiantes) {
    std::thread t1([&estudiante]() {
      estudiante.calcularPromedioParciales();
    });
    t1.join();

    std::thread t2([&estudiante]() {
      estudiante.calcularPromedioTareas();
    });
    t2.join();

    estudiante.calcularCalificacionFinal();

    if(estudiante.aprobo()){
      estudiante.mostrarInformacion();
    }
  }

  return 0;
}
