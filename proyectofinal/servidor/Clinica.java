import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.file.Path;
import java.time.LocalDate;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Clínica de vacunas en memoria. Aplica las correcciones de concurrencia:
 *  - RWLock SOLO para la estructura (altas y bajas), nunca para todo;
 *  - cada Paciente se protege con sus propios synchronized;
 *  - búsqueda binaria sobre la lista por referencia (sin copiarla);
 *  - el archivo se escribe fuera de los cerrojos, con una instantánea.
 *
 * Las consultas dejan las cadenas ya formateadas: ningún método hace E/S por
 * consola ni por red. Eso será trabajo del servidor TCP (fase 4).
 */
public class Clinica {
    private static final int TAM_PAGINA = 5;
    private static final int NUM_PARTES = 5;   // la lista se divide en 5 trozos

    // Hilos daemon: si el proceso termina, no impiden que la JVM se cierre.
    // El servidor igualmente llama a cerrar() al apagarse.
    private final ExecutorService pool = Executors.newFixedThreadPool(NUM_PARTES, tarea -> {
        Thread hilo = new Thread(tarea, "consulta-" + NUM_PARTES);
        hilo.setDaemon(true);
        return hilo;
    });
    private final List<Paciente> pacientes = new ArrayList<>();  // ordenado por id
    private final AtomicInteger siguienteId = new AtomicInteger(1);
    private final AtomicReference<LocalDate> fechaActual = new AtomicReference<>();
    private final Repositorio repositorio;

    private final ReentrantReadWriteLock cerrojo = new ReentrantReadWriteLock();
    private final Lock leer = cerrojo.readLock();
    private final Lock escribir = cerrojo.writeLock();

    public Clinica(Path archivo, LocalDate fecha) throws IOException {
        setFechaActual(fecha);
        this.repositorio = new Repositorio(archivo);
        List<Paciente> cargados = repositorio.cargar(siguienteId);
        pacientes.addAll(cargados);   // ya vienen ordenados por id
    }

    public LocalDate getFechaActual() {
        return fechaActual.get();
    }

    public void setFechaActual(LocalDate fecha) {
        fechaActual.set(Objects.requireNonNull(fecha, "La fecha no puede ser null"));
    }

    public int totalPacientes() {
        leer.lock();
        try {
            return pacientes.size();
        } finally {
            leer.unlock();
        }
    }

    // ---------- 1. Alta ----------

    /** Devuelve el id asignado por el servidor. */
    public int alta(String nombre, LocalDate nacimiento) {
        Paciente nuevo;
        escribir.lock();
        try {
            nuevo = new Paciente(siguienteId.getAndIncrement(), nombre, nacimiento);
            int pos = 0;
            while (pos < pacientes.size() && pacientes.get(pos).getId() < nuevo.getId()) {
                ++pos;
            }
            pacientes.add(pos, nuevo);   // mantiene la búsqueda binaria válida
        } finally {
            escribir.unlock();
        }
        guardar();
        return nuevo.getId();
    }

    // ---------- 2 y 3. Vacunas ----------

    public boolean registrarAplicacionVacuna(int id, Vacuna vacuna) {
        Paciente p = porId(id);
        if (p == null) {
            return false;
        }
        p.agregarVacunaRecibida(vacuna);
        guardar();
        return true;
    }

    public boolean agregarVacunaProxima(int id, Vacuna vacuna) {
        Paciente p = porId(id);
        if (p == null) {
            return false;
        }
        p.agregarVacunaProxima(vacuna);
        guardar();
        return true;
    }

    // ---------- 4. Baja ----------

    public boolean eliminarPaciente(int id) {
        boolean eliminado;
        escribir.lock();
        try {
            int i = indicePorIdSinLock(id);
            if (i < 0) {
                return false;
            }
            pacientes.remove(i);
            eliminado = true;
        } finally {
            escribir.unlock();
        }
        if (eliminado) {
            guardar();
        }
        return eliminado;
    }

    // ---------- 5. Modificaciones ----------

    public boolean modificarNombre(int id, String nombre) {
        Paciente p = porId(id);
        if (p == null) {
            return false;
        }
        p.setNombre(nombre);
        guardar();
        return true;
    }

    public boolean modificarFechaNacimiento(int id, LocalDate fecha) {
        Paciente p = porId(id);
        if (p == null) {
            return false;
        }
        p.setFechaNacimiento(fecha);
        guardar();
        return true;
    }

    // ---------- 6. Información de un paciente ----------

    public String informacion(int id) {
        Paciente p = porId(id);
        return p == null ? null : p.toString();
    }

    /** Útil para pruebas y para el protocolo: tamaño de las listas de un paciente. */
    public int cantidadVacunasRecibidas(int id) {
        Paciente p = porId(id);
        return p == null ? -1 : p.getVacunasRecibidas().size();
    }

    public int cantidadVacunasProximas(int id) {
        Paciente p = porId(id);
        return p == null ? -1 : p.getVacunasProximas().size();
    }

    // ---------- 7. Modificar / eliminar vacuna por nombre de paciente ----------

    public boolean eliminarVacunaDePaciente(String nombrePaciente, String nombreVacuna) {
        Paciente p = porNombre(nombrePaciente);
        if (p == null) {
            return false;
        }
        boolean ok = p.eliminarVacunaRecibida(nombreVacuna);
        if (ok) {
            guardar();
        }
        return ok;
    }

    public boolean modificarVacunaDePaciente(String nombrePaciente, String nombreVacuna,
                                             Vacuna nueva) {
        Paciente p = porNombre(nombrePaciente);
        if (p == null) {
            return false;
        }
        boolean ok = p.modificarVacunaRecibida(nombreVacuna, nueva);
        if (ok) {
            guardar();
        }
        return ok;
    }

    // ---------- 8. Nombres en orden alfabético ----------

    public List<String> nombresAlfabetico() {
        List<Paciente> fuente = instantanea();
        List<List<String>> partes = enCincoPartes(fuente, (inicio, fin) -> {
            List<String> locales = new ArrayList<>();
            for (int i = inicio; i < fin; ++i) {
                locales.add(fuente.get(i).getNombre());
            }
            return locales;
        });
        List<String> nombres = new ArrayList<>();
        for (List<String> parte : partes) {
            nombres.addAll(parte);
        }
        nombres.sort(Comparator.comparing(Utilidades::claveOrden));
        return nombres;
    }

    // ---------- 9. Mostrar de 5 en 5 (adaptado a paginación TCP) ----------

    public List<String> mostrarTodo(int pagina) {
        if (pagina < 0) {
            return List.of();
        }
        List<Paciente> fuente = instantanea();
        int desde = pagina * TAM_PAGINA;
        int hasta = Math.min(desde + TAM_PAGINA, fuente.size());
        List<List<String>> partes = enCincoPartes(fuente, (inicio, fin) -> {
            List<String> locales = new ArrayList<>();
            for (int i = Math.max(inicio, desde); i < Math.min(fin, hasta); ++i) {
                locales.add(fuente.get(i).toString());
            }
            return locales;
        });
        List<String> fila = new ArrayList<>();
        for (List<String> parte : partes) {
            fila.addAll(parte);
        }
        return fila;
    }

    // ---------- 10. Rango de edades ----------

    private record ConEdad(Paciente paciente, int edad) {
    }

    private record Vacunado(String nombre, String texto) {
    }

    public List<String> rangoEdad(int min, int max) {
        if (min > max) {
            return List.of();
        }
        LocalDate hoy = getFechaActual();
        List<Paciente> fuente = instantanea();

        // COBEGIN: 5 hilos, cada uno revisa su quinto parte de la lista.
        List<List<ConEdad>> partes = enCincoPartes(fuente, (inicio, fin) -> {
            List<ConEdad> locales = new ArrayList<>();
            for (int i = inicio; i < fin; ++i) {
                Paciente p = fuente.get(i);
                int edad = Utilidades.calcularEdad(p.getFechaNacimiento(), hoy);
                if (edad >= min && edad <= max) {
                    locales.add(new ConEdad(p, edad));
                }
            }
            return locales;
        });

        // COEND: se juntan los 5 resultados y se ordena una sola vez.
        List<ConEdad> seleccionados = new ArrayList<>();
        for (List<ConEdad> parte : partes) {
            seleccionados.addAll(parte);
        }
        seleccionados.sort(Comparator.comparingInt(ConEdad::edad).reversed()
                .thenComparing(c -> Utilidades.claveOrden(c.paciente().getNombre())));
        List<String> resultado = new ArrayList<>();
        for (ConEdad c : seleccionados) {
            resultado.add(c.paciente().toString());
        }
        return resultado;
    }

    // ---------- 11. Vacuna en los próximos 7 días ----------

    public List<String> conVacunaProxima7Dias(String nombreVacuna) {
        LocalDate hoy = getFechaActual();
        List<Paciente> fuente = instantanea();
        List<List<String>> partes = enCincoPartes(fuente, (inicio, fin) -> {
            List<String> locales = new ArrayList<>();
            for (int i = inicio; i < fin; ++i) {
                Paciente p = fuente.get(i);
                for (Vacuna v : p.getVacunasProximas()) {
                    long dias = Utilidades.diasDiferencia(hoy, v.getFecha());
                    if (Utilidades.mismoNombre(v.getNombre(), nombreVacuna)
                            && dias >= 0 && dias <= 7) {
                        locales.add(p.getNombre());
                        break;
                    }
                }
            }
            return locales;
        });
        List<String> nombres = new ArrayList<>();
        for (List<String> parte : partes) {
            nombres.addAll(parte);
        }
        nombres.sort(Comparator.comparing(Utilidades::claveOrden));
        return nombres;
    }

    // ---------- 12. Vacunas aplicadas en la última semana ----------

    public int contarVacunasAplicadaSemana() {
        LocalDate hoy = getFechaActual();
        List<Paciente> fuente = instantanea();
        List<Integer> partes = enCincoPartes(fuente, (inicio, fin) -> {
            int local = 0;
            for (int i = inicio; i < fin; ++i) {
                for (Vacuna v : fuente.get(i).getVacunasRecibidas()) {
                    long dias = Utilidades.diasDiferencia(v.getFecha(), hoy);
                    if (dias >= 0 && dias <= 7) {
                        ++local;
                    }
                }
            }
            return local;
        });
        int total = 0;
        for (Integer parte : partes) {
            total += parte;
        }
        return total;
    }

    // ---------- 13. Vacuna próxima el siguiente mes ----------

    public int contarVacunasSiguienteMes(String nombreVacuna) {
        LocalDate siguiente = getFechaActual().withDayOfMonth(1).plusMonths(1);
        List<Paciente> fuente = instantanea();
        List<Integer> partes = enCincoPartes(fuente, (inicio, fin) -> {
            int local = 0;
            for (int i = inicio; i < fin; ++i) {
                for (Vacuna v : fuente.get(i).getVacunasProximas()) {
                    if (Utilidades.mismoNombre(v.getNombre(), nombreVacuna)
                            && Utilidades.esMismoMes(v.getFecha(), siguiente)) {
                        ++local;
                    }
                }
            }
            return local;
        });
        int total = 0;
        for (Integer parte : partes) {
            total += parte;
        }
        return total;
    }

    // ---------- 14. Pacientes vacunados en el mes actual ----------

    public List<String> vacunadosDelMes() {
        LocalDate hoy = getFechaActual();
        List<Paciente> fuente = instantanea();
        List<List<Vacunado>> partes = enCincoPartes(fuente, (inicio, fin) -> {
            List<Vacunado> locales = new ArrayList<>();
            for (int i = inicio; i < fin; ++i) {
                Paciente p = fuente.get(i);
                List<Vacuna> recibidas = p.getVacunasRecibidas();
                boolean aplica = false;
                for (Vacuna v : recibidas) {
                    if (Utilidades.esMismoMes(v.getFecha(), hoy)) {
                        aplica = true;
                        break;
                    }
                }
                if (aplica) {
                    StringBuilder texto = new StringBuilder();
                    texto.append(p.getNombre()).append(" (")
                            .append(Utilidades.calcularEdad(p.getFechaNacimiento(), hoy))
                            .append(" anios)\n");
                    for (Vacuna v : recibidas) {
                        if (Utilidades.esMismoMes(v.getFecha(), hoy)) {
                            texto.append("  ").append(v.toString()).append("\n");
                        }
                    }
                    locales.add(new Vacunado(p.getNombre(), texto.toString()));
                }
            }
            return locales;
        });
        List<Vacunado> todos = new ArrayList<>();
        for (List<Vacunado> parte : partes) {
            todos.addAll(parte);
        }
        todos.sort(Comparator.comparing(v -> Utilidades.claveOrden(v.nombre())));
        List<String> resultado = new ArrayList<>();
        for (Vacunado v : todos) {
            resultado.add(v.texto());
        }
        return resultado;
    }

    // ---------- Búsquedas ----------

    /** Devuelve el índice en la lista ordenada por id, o -1. */
    public int buscarPorId(int id) {
        leer.lock();
        try {
            return indicePorIdSinLock(id);
        } finally {
            leer.unlock();
        }
    }

    public int buscarPorNombre(String nombre) {
        List<Paciente> fuente = instantanea();
        List<Integer> partes = enCincoPartes(fuente, (inicio, fin) -> {
            for (int i = inicio; i < fin; ++i) {
                if (Utilidades.mismoNombre(fuente.get(i).getNombre(), nombre)) {
                    return i;   // índice global, no local al trozo
                }
            }
            return -1;
        });
        return primerResultado(partes);
    }

    public int buscarPorFechaNacimiento(LocalDate fecha) {
        List<Paciente> fuente = instantanea();
        List<Integer> partes = enCincoPartes(fuente, (inicio, fin) -> {
            for (int i = inicio; i < fin; ++i) {
                if (fuente.get(i).getFechaNacimiento().equals(fecha)) {
                    return i;
                }
            }
            return -1;
        });
        return primerResultado(partes);
    }

    /** Los trozos vuelven en orden, así que el primero que encontró es el menor índice. */
    private static int primerResultado(List<Integer> partes) {
        for (Integer indice : partes) {
            if (indice >= 0) {
                return indice;
            }
        }
        return -1;
    }

    public int idEnIndice(int indice) {
        leer.lock();
        try {
            if (indice < 0 || indice >= pacientes.size()) {
                return -1;
            }
            return pacientes.get(indice).getId();
        } finally {
            leer.unlock();
        }
    }

    /** Se llama al apagar el servidor para liberar los hilos del pool. */
    public void cerrar() {
        pool.shutdown();
    }

    // ---------- Auxiliares ----------

    private Paciente porId(int id) {
        leer.lock();
        try {
            int i = indicePorIdSinLock(id);
            return i < 0 ? null : pacientes.get(i);
        } finally {
            leer.unlock();
        }
    }

    private Paciente porNombre(String nombre) {
        leer.lock();
        try {
            return porNombreSinLock(nombre);
        } finally {
            leer.unlock();
        }
    }

    /**
     * Copia superficial de los pacientes bajo readLock. Luego se suelta el
     * cerrojo: los Paciente siguen protegidos por sus propios synchronized.
     */
    private List<Paciente> instantanea() {
        leer.lock();
        try {
            return new ArrayList<>(pacientes);
        } finally {
            leer.unlock();
        }
    }

    /** Un trozo de trabajo: recibe el rango [inicio, fin) de la lista. */
    @FunctionalInterface
    private interface Parte<T> {
        T ejecutar(int inicio, int fin);
    }

    /**
     * Divide la lista en 5 trozos, lanza una tarea por trozo (COBEGIN) y espera
     * a las 5 (COEND). Los resultados vuelven en orden de trozo.
     * Cada tarea lee pacientes distintos y escribe solo en su resultado local:
     * por las condiciones de Bernstein son independientes.
     */
    private <T> List<T> enCincoPartes(List<Paciente> fuente, Parte<T> tarea) {
        int n = fuente.size();
        List<Future<T>> futuros = new ArrayList<>(NUM_PARTES);
        for (int i = 0; i < NUM_PARTES; ++i) {
            int inicio = n * i / NUM_PARTES;
            int fin = n * (i + 1) / NUM_PARTES;
            futuros.add(pool.submit(() -> tarea.ejecutar(inicio, fin)));
        }
        List<T> resultados = new ArrayList<>(NUM_PARTES);
        for (Future<T> futuro : futuros) {
            try {
                resultados.add(futuro.get());
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                throw new IllegalStateException("Consulta interrumpida", e);
            } catch (ExecutionException e) {
                throw new IllegalStateException("Falló una parte de la consulta", e.getCause());
            }
        }
        return resultados;
    }

    /** Debe llamarse con algún cerrojo de la estructura ya tomado. */
    private Paciente porNombreSinLock(String nombre) {
        for (Paciente p : pacientes) {
            if (Utilidades.mismoNombre(p.getNombre(), nombre)) {
                return p;
            }
        }
        return null;
    }

    /** Búsqueda binaria sobre la lista, por referencia (nunca por valor). */
    private int indicePorIdSinLock(int id) {
        int inicio = 0;
        int fin = pacientes.size() - 1;
        while (inicio <= fin) {
            int medio = inicio + (fin - inicio) / 2;
            int idMedio = pacientes.get(medio).getId();
            if (id == idMedio) {
                return medio;
            }
            if (idMedio < id) {
                inicio = medio + 1;
            } else {
                fin = medio - 1;
            }
        }
        return -1;
    }

    /**
     * Snapshot + escritura, sin cerrojos de estructura durante la E/S.
     * synchronized evita dos guardados simultáneos o desordenados.
     */
    private synchronized void guardar() {
        List<String> lineas;
        leer.lock();
        try {
            lineas = new ArrayList<>(pacientes.size());
            for (Paciente p : pacientes) {
                lineas.add(p.lineaArchivo());   // cada lectura es synchronized en el paciente
            }
        } finally {
            leer.unlock();
        }
        try {
            repositorio.guardar(lineas);
        } catch (IOException e) {
            throw new UncheckedIOException("No se pudo guardar pacientes.txt", e);
        }
    }
}
