# Proyecto final — Clínica de vacunas distribuida (TCP en Java)

Servidor Java con la clínica de vacunas en memoria y persistencia en `pacientes.txt`.
Clientes en otras computadoras envían peticiones por TCP para ejecutar las operaciones
de la práctica 4. El diseño aplica las correcciones de concurrencia vistas en prácticas
anteriores: cerrojo por paciente, búsqueda por referencia y consultas subdivididas.

## Estado del proyecto

- [x] **Fase 1 — Modelo Java:** `Vacuna`, `Utilidades`, `Paciente` y pruebas.
- [x] **Fase 2 — Clínica y persistencia:** `Clinica`, `Repositorio` y las 14 operaciones.

- [x] **Fase 3 — Subdivisión concurrente de consultas:** 5 trozos por consulta con `ExecutorService`.

- [x] **Fase 4 — Servidor, protocolo y clientes TCP:** `Protocolo`, `ServidorTCP`, `ConexionTCP`, `ClienteTCP` y `ClienteConsola` (menú de texto).
- [x] **Fase 5 — Pruebas multi-cliente:** `PruebaServidor` (20 altas y 20 vacunas concurrentes por red, conexión reutilizable y escape del protocolo).
- [x] **Fase 6 — Documentación final:** este README.

## Cómo compilar y probar

```bash
cd servidor
javac -d out ./*.java
java -cp out PruebaModelo
java -cp out PruebaClinica
java -cp out PruebaConsultas
java -cp out PruebaServidor
```

Salida esperada:
```
Todas las pruebas del modelo pasaron.
Todas las pruebas de la clínica pasaron.
Todas las pruebas de consultas divididas pasaron.
Todas las pruebas del servidor TCP pasaron.
```

Las pruebas cubren: validación de fechas (incluye bisiestos con `ResolverStyle.STRICT`),
nombres, normalización de vacunas, copias defensivas, las 14 operaciones, persistencia
(reabrir el archivo), 80 altas concurrentes, 200 vacunas en el mismo paciente y 40
pacientes distintos vacunados a la vez. `PruebaConsultas` verifica que dividir una consulta
en 5 hilos dé exactamente el mismo resultado que hacerla en uno solo, incluso con 20 hilos
pidiendo la misma consulta simultáneamente. `PruebaServidor` añade las órdenes por red,
el escape de `|` y de saltos de línea, una conexión con varias órdenes seguidas y 20
clientes concurrentes.

## Cómo divide las consultas la fase 3

`Clinica.enCincoPartes()` toma una instantánea de la lista, la parte en 5 rangos y lanza
una tarea por rango en un `ExecutorService` de 5 hilos (`COBEGIN`). Cada tarea lee solo su
trozo y escribe en su propio resultado local, así que son independientes por Bernstein.
Después se juntan los 5 resultados (`COEND`) y se ordena una sola vez. Aplica a:
búsquedas por nombre/fecha, nombres alfabéticos, listado paginado, rango de edades,
próximos 7 días, los dos conteos y vacunados del mes. La búsqueda por id no se divide
(ya es `O(log n)`).

## Fase 4: protocolo TCP

Petición y respuesta son **una línea cada una**, terminada en `\n`:

```
ORDEN|arg1|arg2...   ->   OK|campo1|campo2...   o   ERR|mensaje
```

- `|` separa campos y las fechas van en `dd/MM/aaaa`.
- Una vacuna son dos campos: `nombre|dd/MM/aaaa`.
- Booleanos: `OK|1` / `OK|0`. `ERR` queda para argumentos inválidos u órdenes desconocidas.
- Listas: `OK|<n>|elem1|elem2|...` (el contador va primero).
- Los textos que contienen `|` o saltos de línea se escapan en `Protocolo.codificar`
  (`\|`, `\n`, `\\`). `Protocolo.separar` los descodifica. Servidor y cliente usan
  **los mismos métodos**, así que el formato vive en un solo archivo.

| Orden | Argumentos | Respuesta |
|---|---|---|
| `ALTA` | `nombre`, `nacimiento` | `OK\|id` |
| `BAJA` | `id` | `OK\|0/1` |
| `NOMBRE` | `id`, `nuevoNombre` | `OK\|0/1` |
| `NACIMIENTO` | `id`, `nuevaFecha` | `OK\|0/1` |
| `VACUNA_RECIBIDA` | `id`, `nombreVacuna`, `fecha` | `OK\|0/1` |
| `VACUNA_PROXIMA` | `id`, `nombreVacuna`, `fecha` | `OK\|0/1` |
| `ELIMINAR_VACUNA` | `nombrePaciente`, `nombreVacuna` | `OK\|0/1` |
| `MODIFICAR_VACUNA` | `nombrePaciente`, `vieja`, `nueva`, `fecha` | `OK\|0/1` |
| `INFO` | `id` | `OK\|texto` o `OK\|0` |
| `CANT_RECIBIDAS` / `CANT_PROXIMAS` | `id` | `OK\|n` (`-1` si no existe) |
| `NOMBRES` | — | `OK\|n\|nom1\|...` |
| `MOSTRAR` | `pagina` | `OK\|n\|línea1\|...` |
| `RANGO_EDAD` | `min`, `max` | `OK\|n\|línea1\|...` |
| `PROXIMAS_7` | `nombreVacuna` | `OK\|n\|nom1\|...` |
| `CONTAR_APLICADAS` | — | `OK\|n` |
| `CONTAR_MES` | `nombreVacuna` | `OK\|n` |
| `VACUNADOS_MES` | — | `OK\|n\|texto1\|...` |
| `INDICE_ID` / `INDICE_NOMBRE` / `INDICE_NACIMIENTO` | `id` / `nombre` / `fecha` | `OK\|índice` |
| `ID_EN_INDICE` | `índice` | `OK\|id` |
| `TOTAL` | — | `OK\|n` |
| `FECHA` / `SET_FECHA` | — / `fecha` | `OK\|dd/MM/aaaa` / `OK\|1` |
| `SALIR` | — | `OK\|adios` (cierra esa conexión) |

## Cómo arrancar servidor y cliente

Servidor, en la máquina que tiene el archivo:

```bash
cd servidor
java -cp out ServidorTCP 5000 pacientes.txt 13/10/2026
```

Los tres argumentos son opcionales: puerto (5000), archivo (`pacientes.txt`) y fecha
inicial (la de hoy). El socket escucha en todas las interfaces, así que los clientes
de otras máquinas solo necesitan alcanzar la IP del servidor.

### Conectarse desde otra computadora

1. Descubre la IP del servidor: `hostname -I` o `ip addr` en Linux, `ipconfig` en Windows.
2. Abre el puerto en el firewall: `sudo ufw allow 5000/tcp` en Linux, o una regla de
   entrada en Windows Defender.
3. Copia la carpeta `out` a la otra máquina (el cliente no necesita `patients.txt`).
   También puedes compilar allí `ClienteTCP.java`, `ClienteConsola.java`,
   `ConexionTCP.java`, `Protocolo.java` y `Utilidades.java`.
4. Comprueba la red con `ping <IP>` y `nc -vz <IP> 5000`.
5. Ejecuta el cliente con la IP real:

```bash
java -cp out ClienteTCP 192.168.1.24 5000 ALTA Ana 01/01/2015
java -cp out ClienteConsola 192.168.1.24 5000     # menú de puro texto
```

Desde internet haría falta redirección de puerto o una VPN; como el protocolo no tiene
autenticación, mejor quedarse en la red local.

### Cliente por comandos

```bash
java -cp out ClienteTCP 127.0.0.1 5000 ALTA Ana 01/01/2015
java -cp out ClienteTCP 127.0.0.1 5000 MOSTRAR 0
java -cp out ClienteTCP 127.0.0.1 5000     # escribe órdenes a mano
```

El cliente une los argumentos con `|`: `ALTA Ana 01/01/2015` se envía como
`ALTA|Ana|01/01/2015`.

### Menú de texto

`ClienteConsola` muestra menús numerados (Pacientes, Vacunas, Consultas, Búsquedas y
Servidor), pide cada campo por separado, valida fechas y números antes de enviar y
mantiene **una sola conexión** durante toda la sesión. El usuario no necesita conocer
el protocolo. `ConexionTCP` es la clase compartida por ambos clientes.

## Fase 5: pruebas multi-cliente

`PruebaServidor` levanta el servidor dentro del mismo proceso y lanza 20 clientes que
hacen `ALTA` simultáneamente (comprueba 20 ids únicos), otros 20 que agregan vacunas al
mismo paciente (comprueba que no se pierde ninguna) y finalmente reabre el archivo para
verificar la persistencia. También cubre el escape de `|` y de saltos de línea y las
respuestas `ERR`.

## Resumen de concurrencia

- `ReentrantReadWriteLock` protege **solo la estructura** (altas y bajas).
- Cada `Paciente` tiene sus propios métodos `synchronized`: dos clientes que trabajan
  con pacientes distintos no se bloquean.
- `siguienteId` es `AtomicInteger` y `fechaActual` es `AtomicReference`: contadores y
  estado compartidos sin depender del cerrojo.
- Las consultas se dividen en 5 trozos con un `ExecutorService` de hilos daemon.
- Un hilo daemon por cliente atiende la conexión; un cliente colgado no impide apagar
  el servidor, y `cerrar()` libera el pool al apagarse.
- `Repositorio.guardar()` escribe en un archivo temporal y renombra con `ATOMIC_MOVE`.
