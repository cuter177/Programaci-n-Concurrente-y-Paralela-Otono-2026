Dra. Hilda Castillo Zacatelco
 PROGRAMACIÓN CONCURRENTE Y PARALELA

Práctica 4. Elaborar un programa que dé solución a lo siguiente:

La  información  de  los  pacientes  pediátricos  de  una  clínica  de  medicina  general,  se
encuentran en un archivo de texto. La información de cada paciente es la siguiente:
nombre, fecha de nacimiento, lista de vacunas recibidas, lista de vacunas próximas a
realizarse.

Cuando la clínica abre, se cargan los datos a memoria y se leerá la fecha en la cual se
está laborando (actual) y el programa permite realizar las siguientes operaciones:

1.  Registrar información de nuevo paciente
2.  Registrar aplicación de vacuna
3.  Agregar fecha y nombre de vacuna próxima a realizarse
4.   Eliminar información de paciente
5.  Modificar  nombre  o  fecha  de  nacimiento  de  paciente  (buscar  por  nombre  o

fecha de nacimiento)

6.  Mostrar información de paciente
7.  Modificar  o  eliminar  vacuna,  buscar  por  nombre  de  paciente    (nombre  de

vacuna o fecha)

8.  Mostrar en orden alfabético los nombres de todos los pacientes
9.  Mostrar  en  orden  alfabético  los  nombres  de  todos  los  pacientes  y  toda  su

información, mostrarla de 5 en 5.

10. Buscar pacientes de determinado intervalo de edades
11. Mostrar por orden alfabético pacientes a los que se les aplicarán determinada

vacuna en los próximos 7 días

12. Mostrar el número de vacunas aplicadas en la última semana
13. Mostrar el número de determinada vacuna próxima a aplicarse (siguiente mes)
14. Mostrar  el  nombre  de  los  pacientes  en  orden  alfabético  que  acudieron  a
aplicarse vacunas en el mes actual (mostrar nombre de paciente, edad, nombre
de vacunas aplicadas y fecha de aplicación).

Cuando la clínica cierra, se almacenan los datos nuevos al final del archivo de texto
original  y  los  que  se  modificaron  se  actualizan,  y  quedan  en  el  orden  en  el  cual  se
encontraban originalmente.

Un ejemplo del contenido del archivo de texto es el siguiente:

Otoño 2026                                                                                                       FCC-BUAP

Dra. Hilda Castillo Zacatelco
 PROGRAMACIÓN CONCURRENTE Y PARALELA

“Nieves, Casco, Ramírez; 27/09/2020; hepatitis B, 25/05/2023, varicela, 22/01/2021,
triple viral, 14/12/2022;   tétanos, 22/10/2027, difteria, 07/11/2027.

Juan de Dios, HERnández ,    hernandez ; 14/04/2019; SaraMpión, 12/07/2020,
variCela, 22/01/2021, triple viral, 22/01/2021;  .

María de Jesús, De la Torre, CázaREs; 27/09/2026; ; sarampion, 25/05/2027, tetaNos,
22/01/2027, triple viral, 14/12/2027.”

La información del paciente se almacena en una sola línea de texto.

Cuando se lee nueva información es necesario validar entradas (fechas y nombre de
vacunas).

El archivo original (más los nuevos registros) debe ser siempre el punto de partida.

Entregar grafos de dependencia y bloques COBEGIN – COEND, y archivos fuente.

Otoño 2026                                                                                                       FCC-BUAP

