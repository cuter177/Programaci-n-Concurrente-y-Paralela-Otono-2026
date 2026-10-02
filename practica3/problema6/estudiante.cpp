#include "estudiante.h"
#include <string>
#include <iostream>
#include <thread>

Estudiante::Estudiante(std::string nombre, int edad){
  this -> nombre = nombre;
  this -> edad = edad;
  this -> calificacion_final = 0.0;
  this -> promedio_parciales = 0.0;
  this -> promedio_tareas = 0.0;
  this -> aprobado = false;
}

void Estudiante::setNombre(std::string nombre){
  this -> nombre = nombre;
}

void Estudiante::setEdad(int edad){
  this -> edad = edad;
}

void Estudiante::setCalificacionParcial1(double calificacion){
  calificacion_parcial1 = calificacion;
}

void Estudiante::setCalificacionParcial2(double calificacion){
  calificacion_parcial2 = calificacion;
}

void Estudiante::setCalificacionParcial3(double calificacion){
  calificacion_parcial3 = calificacion;
}

void Estudiante::setCalificacionTarea1(double calificacion){
  calificacion_tarea1 = calificacion;
}

void Estudiante::setCalificacionTarea2(double calificacion){
  calificacion_tarea2 = calificacion;
}

void Estudiante::setCalificacionTarea3(double calificacion){
  calificacion_tarea3 = calificacion;
}

void Estudiante::setCalificacionTarea4(double calificacion){
  calificacion_tarea4 = calificacion;
}

std::string Estudiante::getNombre() const {
  return nombre;
}

int Estudiante::getEdad() const {
  return edad;
}

double Estudiante::getCalificacionParcial1() const {
  return calificacion_parcial1;
}

double Estudiante::getCalificacionParcial2() const {
  return calificacion_parcial2;
}

double Estudiante::getCalificacionParcial3() const {
  return calificacion_parcial3;
}

double Estudiante::getCalificacionTarea1() const {
  return calificacion_tarea1;
}

double Estudiante::getCalificacionTarea2() const {
  return calificacion_tarea2;
}

double Estudiante::getCalificacionTarea3() const {
  return calificacion_tarea3;
}

double Estudiante::getCalificacionTarea4() const {
  return calificacion_tarea4;
}



void Estudiante::calcularPromedioParciales(){
  promedio_parciales = (calificacion_parcial1 + calificacion_parcial2 + calificacion_parcial3) / 3;
}

void Estudiante::calcularPromedioTareas(){
  promedio_tareas = (calificacion_tarea1 + calificacion_tarea2 + calificacion_tarea3 + calificacion_tarea4) / 4;
}


double Estudiante::calcularCalificacionFinal(){
  calificacion_final = promedio_parciales * 0.5 + promedio_tareas * 0.5;
  return calificacion_final;
}

void Estudiante::procesar(){
  // COBEGIN-COEND: T1 (pp) y T2 (pa) escriben variables distintas y leen
  // datos distintos, por lo que son independientes.
  std::thread t1([this]() { calcularPromedioParciales(); });
  std::thread t2([this]() { calcularPromedioTareas(); });
  t1.join();
  t2.join();

  // Punto de union: cf depende de pp y pa.
  calcularCalificacionFinal();

  // T4: aprobado depende de los tres parciales y de cf.
  aprobado = (calificacion_parcial1 >= 6.0 &&
              calificacion_parcial2 >= 6.0 &&
              calificacion_parcial3 >= 6.0 &&
              calificacion_final >= 6.0);
}

bool Estudiante::aprobo() const {
  return aprobado;
}

void Estudiante::mostrarInformacion(){
  std::cout << "[Nombre: " << nombre << ", Edad: " << edad
            << ", Calificacion Final: " << calificacion_final << "]" << std::endl;
}
