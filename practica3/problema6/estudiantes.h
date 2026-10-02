#ifndef ESTUDIANTES_H
#define ESTUDIANTES_H
#include "estudiante.h"
#include <vector>

class Estudiantes{
 private:
   std::vector<Estudiante> estudiantes;

  public:
  
   void agregarEstudiante(const Estudiante& estudiante);
   void mostrarEstudiantesAprobados();
};

#endif // !ESTUDIANTES_H

