#include "estudiantes.h"
#include "estudiante.h"
#include <iostream>

void Estudiantes::agregarEstudiante(const Estudiante& estudiante){
  estudiantes.push_back(estudiante);
}

void Estudiantes::mostrarEstudiantesAprobados(){
  std::cout << "Estudiantes aprobadosL" << std::endl;
  for(auto& estudiante : estudiantes){
    if(estudiante.aprobo()){
      estudiante.mostrarInformacion();
    }
  }
}
