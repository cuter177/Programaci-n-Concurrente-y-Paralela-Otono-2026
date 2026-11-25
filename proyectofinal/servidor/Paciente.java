import java.time.LocalDate;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * Paciente con sincronización fina, igual que Asiento en la práctica 5:
 * cada paciente tiene su propio monitor. Dos clientes que trabajan con
 * pacientes distintos no se bloquean entre sí.
 *
 * Las listas se devuelven COPIADAS para que nadie modifique el estado interno
 * sin pasar por los métodos synchronized.
 */
public class Paciente {
    private final int id;
    private String nombre;
    private LocalDate fechaNacimiento;
    private final List<Vacuna> recibidas = new ArrayList<>();
    private final List<Vacuna> proximas = new ArrayList<>();

    public Paciente(int id, String nombre, LocalDate fechaNacimiento) {
        if (!Utilidades.esNombreValido(nombre)) {
            throw new IllegalArgumentException("El nombre del paciente no es valido");
        }
        if (fechaNacimiento == null) {
            throw new IllegalArgumentException("La fecha de nacimiento no puede ser null");
        }
        this.id = id;
        this.nombre = Utilidades.normalizar(nombre);
        this.fechaNacimiento = fechaNacimiento;
    }

    // El id nunca cambia: no necesita sincronización.
    public int getId() {
        return id;
    }

    public synchronized String getNombre() {
        return nombre;
    }

    public synchronized void setNombre(String nombre) {
        if (!Utilidades.esNombreValido(nombre)) {
            throw new IllegalArgumentException("El nombre del paciente no es valido");
        }
        this.nombre = Utilidades.normalizar(nombre);
    }

    public synchronized LocalDate getFechaNacimiento() {
        return fechaNacimiento;
    }

    public synchronized void setFechaNacimiento(LocalDate fechaNacimiento) {
        if (fechaNacimiento == null) {
            throw new IllegalArgumentException("La fecha de nacimiento no puede ser null");
        }
        this.fechaNacimiento = fechaNacimiento;
    }

    // ---------- Vacunas recibidas ----------

    public synchronized void agregarVacunaRecibida(Vacuna vacuna) {
        recibidas.add(Objects.requireNonNull(vacuna));
    }

    public synchronized boolean eliminarVacunaRecibida(String nombreVacuna) {
        return eliminar(recibidas, nombreVacuna);
    }

    public synchronized boolean modificarVacunaRecibida(String nombreBuscado, Vacuna nueva) {
        return modificar(recibidas, nombreBuscado, nueva);
    }

    public synchronized List<Vacuna> getVacunasRecibidas() {
        return new ArrayList<>(recibidas);
    }

    // ---------- Vacunas próximas ----------

    public synchronized void agregarVacunaProxima(Vacuna vacuna) {
        proximas.add(Objects.requireNonNull(vacuna));
    }

    public synchronized boolean eliminarVacunaProxima(String nombreVacuna) {
        return eliminar(proximas, nombreVacuna);
    }

    public synchronized boolean modificarVacunaProxima(String nombreBuscado, Vacuna nueva) {
        return modificar(proximas, nombreBuscado, nueva);
    }

    public synchronized List<Vacuna> getVacunasProximas() {
        return new ArrayList<>(proximas);
    }

    // ---------- Auxiliares privados ----------

    private static boolean eliminar(List<Vacuna> lista, String nombreVacuna) {
        for (int i = 0; i < lista.size(); ++i) {
            if (Utilidades.mismoNombre(lista.get(i).getNombre(), nombreVacuna)) {
                lista.remove(i);
                return true;
            }
        }
        return false;
    }

    private static boolean modificar(List<Vacuna> lista, String nombreBuscado, Vacuna nueva) {
        for (int i = 0; i < lista.size(); ++i) {
            if (Utilidades.mismoNombre(lista.get(i).getNombre(), nombreBuscado)) {
                lista.set(i, Objects.requireNonNull(nueva));
                return true;
            }
        }
        return false;
    }

    /** Línea para pacientes.txt (sin id; se asigna al cargar). */
    public synchronized String lineaArchivo() {
        return nombre + "; " + Utilidades.formatearFecha(fechaNacimiento)
                + "; " + formatoArchivo(recibidas) + "; " + formatoArchivo(proximas);
    }

    @Override
    public synchronized String toString() {
        return id + "; " + nombre + "; " + Utilidades.formatearFecha(fechaNacimiento)
                + "; " + formatoLista(recibidas) + "; " + formatoLista(proximas);
    }

    private static String formatoArchivo(List<Vacuna> lista) {
        StringBuilder texto = new StringBuilder();
        for (int i = 0; i < lista.size(); ++i) {
            if (i > 0) {
                texto.append(", ");
            }
            texto.append(lista.get(i).toString());
        }
        return texto.toString();
    }

    private static String formatoLista(List<Vacuna> lista) {
        StringBuilder texto = new StringBuilder();
        for (int i = 0; i < lista.size(); ++i) {
            if (i > 0) {
                texto.append(" | ");
            }
            texto.append(lista.get(i).toString());
        }
        return texto.toString();
    }
}
