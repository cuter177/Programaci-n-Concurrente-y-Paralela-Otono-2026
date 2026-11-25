# Proyecto final — Clínica de vacunas distribuida (TCP en Java)

Servidor Java con la clínica de vacunas en memoria y persistencia en `pacientes.txt`.
Clientes en otras computadoras envían peticiones por TCP para ejecutar las operaciones
de la práctica 4. El diseño aplica las correcciones de concurrencia vistas en prácticas
anteriores: cerrojo por paciente, búsqueda por referencia y consultas subdivididas.

## Estado del proyecto

- [x] **Fase 1 — Modelo Java:** `Vacuna`, `Utilidades`, `Paciente` y pruebas.
- [ ] Fase 2 — Clínica y persistencia.
- [ ] Fase 3 — Subdivisión concurrente de consultas.
- [ ] Fase 4 — Servidor, protocolo y cliente TCP.
- [ ] Fase 5 — Pruebas multi-cliente.
- [ ] Fase 6 — Documentación final.

## Cómo compilar y probar la fase 1

```bash
cd servidor
javac -d out ./*.java
java -cp out PruebaModelo
```

Salida esperada: `Todas las pruebas del modelo pasaron.`

Las pruebas cubren: validación de fechas (incluye bisiestos con `ResolverStyle.STRICT`),
nombres, normalización de vacunas, copias defensivas de las listas y 200 hilos agregando
vacunas al mismo paciente sin perder ninguna.
