import java.time.LocalDate;
import java.util.Objects;

/** Vacuna inmutable: nombre y fecha. Se puede compartir entre hilos sin cerrojos. */
public final class Vacuna {
    private final String nombre;
    private final LocalDate fecha;

    public Vacuna(String nombre, LocalDate fecha) {
        if (!Utilidades.esNombreValido(nombre)) {
            throw new IllegalArgumentException("El nombre de la vacuna no es valido");
        }
        if (fecha == null) {
            throw new IllegalArgumentException("La fecha de la vacuna no puede ser null");
        }
        this.nombre = Utilidades.normalizar(nombre);
        this.fecha = fecha;
    }

    public String getNombre() {
        return nombre;
    }

    public LocalDate getFecha() {
        return fecha;
    }

    @Override
    public boolean equals(Object otro) {
        if (this == otro) {
            return true;
        }
        if (!(otro instanceof Vacuna)) {
            return false;
        }
        Vacuna v = (Vacuna) otro;
        return nombre.equals(v.nombre) && fecha.equals(v.fecha);
    }

    @Override
    public int hashCode() {
        return Objects.hash(nombre, fecha);
    }

    @Override
    public String toString() {
        return nombre + ", " + Utilidades.formatearFecha(fecha);
    }
}
