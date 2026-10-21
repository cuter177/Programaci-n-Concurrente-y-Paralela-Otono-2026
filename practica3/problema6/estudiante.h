#ifndef ESTUDIANTE_H
#define ESTUDIANTE_H
#include <string>

class Estudiante {
  private:
    std::string nombre;
    int edad;
    double calificacion_parcial1;
    double calificacion_parcial2;
    double calificacion_parcial3;
    double promedio_parciales;

    double calificacion_tarea1;
    double calificacion_tarea2;
    double calificacion_tarea3;
    double calificacion_tarea4;
    double promedio_tareas;

    double calificacion_final;
    bool aprobado;

  public:
    Estudiante(std::string nombre, int edad);

    void setCalificacionParcial1(double calificacion);
    void setCalificacionParcial2(double calificacion);
    void setCalificacionParcial3(double calificacion);
    void setCalificacionTarea1(double calificacion);
    void setCalificacionTarea2(double calificacion);
    void setCalificacionTarea3(double calificacion);
    void setCalificacionTarea4(double calificacion);
    void setEdad(int edad);
    void setNombre(std::string nombre);

    double getCalificacionParcial1() const;
    double getCalificacionParcial2() const;
    double getCalificacionParcial3() const;
    double getCalificacionTarea1() const;
    double getCalificacionTarea2() const;
    double getCalificacionTarea3() const;
    double getCalificacionTarea4() const;
    int getEdad() const;
    std::string getNombre() const;

    void calcularPromedioParciales();
    void calcularPromedioTareas();
    double calcularCalificacionFinal();

    // Ejecuta el COBEGIN-COEND interno (pp y pa en paralelo), calcula cf y
    // determina si el estudiante aprobo.
    void procesar();

    bool aprobo() const;

    void mostrarInformacion();
};

#endif // !ESTUDIANTE_H
