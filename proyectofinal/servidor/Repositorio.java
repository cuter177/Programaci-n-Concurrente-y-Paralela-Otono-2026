import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.AtomicMoveNotSupportedException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.time.LocalDate;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Persistencia en patients.txt, nuestra "base de datos".
 * Formato de cada línea: nombre; dd/MM/aaaa; recibidas; próximas
 * donde cada lista es: "nombre, dd/MM/aaaa, nombre2, dd/MM/aaaa".
 */
public class Repositorio {
    private final Path ruta;

    public Repositorio(Path ruta) {
        this.ruta = ruta;
    }

    /** Carga los registros válidos y les asigna id en orden de lectura. */
    public List<Paciente> cargar(AtomicInteger siguienteId) throws IOException {
        List<Paciente> pacientes = new ArrayList<>();
        if (!Files.exists(ruta)) {
            return pacientes;
        }
        for (String linea : Files.readAllLines(ruta, StandardCharsets.UTF_8)) {
            Paciente p = parsearLinea(linea, siguienteId);
            if (p != null) {
                pacientes.add(p);
            }
        }
        return pacientes;
    }

    private static Paciente parsearLinea(String linea, AtomicInteger siguienteId) {
        String[] campos = linea.split(";", -1);   // -1 conserva campos vacíos
        if (campos.length < 4) {
            return null;
        }
        String nombre = Utilidades.normalizar(campos[0]);
        if (!Utilidades.esNombreValido(nombre)) {
            return null;
        }
        LocalDate nacimiento;
        try {
            nacimiento = Utilidades.parsearFecha(campos[1]);
        } catch (IllegalArgumentException e) {
            return null;
        }

        Paciente p = new Paciente(siguienteId.getAndIncrement(), nombre, nacimiento);
        for (Vacuna v : parsearVacunas(campos[2])) {
            p.agregarVacunaRecibida(v);
        }
        for (Vacuna v : parsearVacunas(campos[3])) {
            p.agregarVacunaProxima(v);
        }
        return p;
    }

    /** Los pares nombre/fecha vienen separados por comas, de dos en dos. */
    private static List<Vacuna> parsearVacunas(String texto) {
        List<Vacuna> vacunas = new ArrayList<>();
        String[] partes = texto.split(",", -1);
        for (int i = 0; i + 1 < partes.length; i += 2) {
            String nombre = Utilidades.normalizar(partes[i]);
            try {
                vacunas.add(new Vacuna(nombre, Utilidades.parsearFecha(partes[i + 1])));
            } catch (IllegalArgumentException e) {
                // par inválido: se omite sin descartar el paciente
            }
        }
        return vacunas;
    }

    /**
     * Escribe una instantánea completa. Sincronizado para que dos guardados
     * concurrentes no se mezclen ni se hagan en orden equivocado.
     * Se escribe a un archivo temporal y se renombra: si el proceso muere a
     * mitad, el archivo original queda intacto.
     */
    public synchronized void guardar(List<String> lineas) throws IOException {
        Path temporal = ruta.resolveSibling(ruta.getFileName() + ".tmp");
        Files.write(temporal, lineas, StandardCharsets.UTF_8);
        try {
            Files.move(temporal, ruta, StandardCopyOption.REPLACE_EXISTING,
                    StandardCopyOption.ATOMIC_MOVE);
        } catch (AtomicMoveNotSupportedException e) {
            Files.move(temporal, ruta, StandardCopyOption.REPLACE_EXISTING);
        }
    }
}
