import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.time.format.DateTimeParseException;
import java.time.format.ResolverStyle;
import java.time.temporal.ChronoUnit;
import java.util.Locale;

/** Utilidades de fechas y texto, compartidas por todo el proyecto. */
public final class Utilidades {

    // "uuuu" + STRICT es lo correcto para LocalDate (con "yyyy" STRICT pide era).
    private static final DateTimeFormatter FORMATO =
            DateTimeFormatter.ofPattern("dd/MM/uuuu").withResolverStyle(ResolverStyle.STRICT);

    private Utilidades() {
    }

    /** El protocolo usa '|' como separador, así que no puede aparecer en un nombre. */
    public static boolean esNombreValido(String nombre) {
        return nombre != null && !nombre.isBlank() && !nombre.contains("|");
    }

    public static String normalizar(String texto) {
        return texto == null ? "" : texto.trim();
    }

    /** Convierte dd/MM/aaaa a LocalDate. Lanza IllegalArgumentException si es inválida. */
    public static LocalDate parsearFecha(String texto) {
        if (texto == null) {
            throw new IllegalArgumentException("Fecha vacia");
        }
        try {
            return LocalDate.parse(normalizar(texto), FORMATO);
        } catch (DateTimeParseException e) {
            throw new IllegalArgumentException("Fecha invalida: " + texto);
        }
    }

    public static String formatearFecha(LocalDate fecha) {
        return fecha.format(FORMATO);
    }

    /** Comparación de nombres sin importar mayúsculas (reemplaza a Utils::sonIguales). */
    public static boolean mismoNombre(String a, String b) {
        return a != null && b != null && a.equalsIgnoreCase(b);
    }

    /** Clave de orden alfabético estable, sin importar mayúsculas. */
    public static String claveOrden(String texto) {
        return normalizar(texto).toUpperCase(Locale.ROOT);
    }

    /** Edad en años cumplidos, igual que en la práctica 4. */
    public static int calcularEdad(LocalDate nacimiento, LocalDate hoy) {
        int edad = hoy.getYear() - nacimiento.getYear();
        if (hoy.getMonthValue() < nacimiento.getMonthValue()
                || (hoy.getMonthValue() == nacimiento.getMonthValue()
                    && hoy.getDayOfMonth() < nacimiento.getDayOfMonth())) {
            --edad;
        }
        return edad;
    }

    /** Días entre dos fechas: positivo si 'hasta' es posterior a 'desde'. */
    public static long diasDiferencia(LocalDate desde, LocalDate hasta) {
        return ChronoUnit.DAYS.between(desde, hasta);
    }

    public static boolean esMismoMes(LocalDate a, LocalDate b) {
        return a.getYear() == b.getYear() && a.getMonthValue() == b.getMonthValue();
    }
}
