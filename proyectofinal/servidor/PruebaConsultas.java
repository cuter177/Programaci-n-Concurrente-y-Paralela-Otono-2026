import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.time.LocalDate;
import java.util.Comparator;
import java.util.List;
import java.util.concurrent.CountDownLatch;

/**
 * Pruebas de la fase 3: cada consulta se divide en 5 trozos resueltos por
 * hilos distintos. Se comprueba que el resultado sea IDÉNTICO al esperado,
 * incluso cuando varias consultas se piden al mismo tiempo.
 */
public class PruebaConsultas {

    private static final LocalDate HOY = Utilidades.parsearFecha("13/10/2026");

    public static void main(String[] args) throws Exception {
        Path directorio = Files.createTempDirectory("consultas-test-");
        Path archivo = directorio.resolve("pacientes.txt");
        Clinica clinica = new Clinica(archivo, HOY);
        try {
            prepararDatos(clinica);

            // Resultado de referencia (se calcula una vez, en un solo hilo).
            List<String> referencia = clinica.rangoEdad(3, 5);
            check(referencia.size() == 30, "30 pacientes entre 3 y 5 años");
            check(referencia.get(0).contains("P05"), "primero es el mayor de los de 5 años");
            check(referencia.get(29).contains("P93"), "último es el menor de los de 3 años");
            check(clinica.nombresAlfabetico().size() == 100, "100 nombres");
            check(clinica.contarVacunasAplicadaSemana() == 20, "20 vacunas esta semana");
            check(clinica.buscarPorNombre("P50") == 50, "búsqueda por nombre en el trozo 3");

            // 20 hilos piden la MISMA consulta 5 veces cada uno: siempre igual.
            int cantidad = 20;
            CountDownLatch inicio = new CountDownLatch(1);
            Thread[] hilos = new Thread[cantidad];
            for (int i = 0; i < cantidad; ++i) {
                hilos[i] = new Thread(() -> {
                    try {
                        inicio.await();
                    } catch (InterruptedException e) {
                        return;
                    }
                    for (int rep = 0; rep < 5; ++rep) {
                        if (!clinica.rangoEdad(3, 5).equals(referencia)) {
                            throw new AssertionError("La consulta dividida dio otro resultado");
                        }
                    }
                });
                hilos[i].start();
            }
            inicio.countDown();
            for (Thread hilo : hilos) {
                hilo.join();
            }

            // 5 consultas DISTINTAS en paralelo, todas correctas.
            Thread[] consultas = new Thread[5];
            consultas[0] = new Thread(() -> check(clinica.nombresAlfabetico().size() == 100,
                    "nombres en paralelo"));
            consultas[1] = new Thread(() -> check(clinica.contarVacunasAplicadaSemana() == 20,
                    "conteo en paralelo"));
            consultas[2] = new Thread(() -> check(clinica.buscarPorNombre("P50") == 50,
                    "búsqueda en paralelo"));
            consultas[3] = new Thread(() -> check(clinica.mostrarTodo(0).size() == 5,
                    "paginación en paralelo"));
            consultas[4] = new Thread(() -> check(clinica.contarVacunasSiguienteMes("nada") == 0,
                    "conteo de mes en paralelo"));
            for (Thread consulta : consultas) {
                consulta.start();
            }
            for (Thread consulta : consultas) {
                consulta.join();
            }

            System.out.println("Todas las pruebas de consultas divididas pasaron.");
        } finally {
            clinica.cerrar();
            borrarRecursivo(directorio);
        }
    }

    private static void prepararDatos(Clinica c) {
        LocalDate fechaVacuna = Utilidades.parsearFecha("10/10/2026");
        for (int i = 0; i < 100; ++i) {
            String nombre = String.format("P%02d", i);
            LocalDate nacimiento = HOY.minusYears(i % 10);
            int id = c.alta(nombre, nacimiento);
            if (i % 5 == 0) {
                c.registrarAplicacionVacuna(id, new Vacuna("gripe", fechaVacuna));
            }
        }
    }

    private static void check(boolean condicion, String mensaje) {
        if (!condicion) {
            throw new AssertionError("Fallo: " + mensaje);
        }
    }

    private static void borrarRecursivo(Path ruta) throws IOException {
        if (!Files.exists(ruta)) {
            return;
        }
        try (var caminos = Files.walk(ruta)) {
            caminos.sorted(Comparator.reverseOrder()).forEach(p -> {
                try {
                    Files.deleteIfExists(p);
                } catch (IOException e) {
                    // ignorar limpieza
                }
            });
        }
    }
}
