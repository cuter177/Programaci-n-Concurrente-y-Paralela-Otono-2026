# Proyecto final — Clínica de vacunas distribuida (TCP en Java)

Servidor Java con la clínica de vacunas en memoria y persistencia en `pacientes.txt`.
Clientes en otras computadoras envían peticiones por TCP para ejecutar las operaciones
de la práctica 4. El diseño aplica las correcciones de concurrencia vistas en prácticas
anteriores: cerrojo por paciente, búsqueda por referencia y consultas subdivididas.

## Estado del proyecto

- [x] **Fase 1 — Modelo Java:** `Vacuna`, `Utilidades`, `Paciente` y pruebas.
- [x] **Fase 2 — Clínica y persistencia:** `Clinica`, `Repositorio` y las 14 operaciones.

- [x] **Fase 3 — Subdivisión concurrente de consultas:** 5 trozos por consulta con `ExecutorService`.

- [ ] Fase 4 — Servidor, protocolo y cliente TCP.
- [ ] Fase 5 — Pruebas multi-cliente.
- [ ] Fase 6 — Documentación final.

## Cómo compilar y probar la fase 1

```bash
cd servidor
javac -d out ./*.java
java -cp out PruebaModelo
```

Salida esperada:
```
Todas las pruebas del modelo pasaron.
Todas las pruebas de la clínica pasaron.
Todas las pruebas de consultas divididas pasaron.
```

Las pruebas cubren: validación de fechas (incluye bisiestos con `ResolverStyle.STRICT`),
nombres, normalización de vacunas, copias defensivas, las 14 operaciones, persistencia
(reabrir el archivo), 80 altas concurrentes, 200 vacunas en el mismo paciente y 40
pacientes distintos vacunados a la vez. `PruebaConsultas` verifica que dividir una consulta
en 5 hilos dé exactamente el mismo resultado que hacerla en uno solo, incluso con 20 hilos
pidiendo la misma consulta simultáneamente.

## Cómo divide las consultas la fase 3

`Clinica.enCincoPartes()` toma una instantánea de la lista, la parte en 5 rangos y lanza
una tarea por rango en un `ExecutorService` de 5 hilos (`COBEGIN`). Cada tarea lee solo su
trozo y escribe en su propio resultado local, así que son independientes por Bernstein.
Después se juntan los 5 resultados (`COEND`) y se ordena una sola vez. Aplica a:
búsquedas por nombre/fecha, nombres alfabéticos, listado paginado, rango de edades,
próximos 7 días, los dos conteos y vacunados del mes. La búsqueda por id no se divide
(ya es `O(log n)`).
