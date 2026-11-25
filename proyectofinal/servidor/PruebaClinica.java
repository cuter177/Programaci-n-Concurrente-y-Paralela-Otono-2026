import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.time.LocalDate;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CountDownLatch;

/** Pruebas de la fase 2: clínica, persistencia y concurrencia fina. */
public class PruebaClinica {

    private static final LocalDate HOY = Utilidades.parsearFecha("13/10/2026");

    public static void main(String[] args) throws Exception {
        Path directorio = Files.createTempDirectory("clinica-test-");
        Path archivo = directorio.resolve("pacientes.txt");
        try {
            escribirArchivoInicial(archivo);

            Clinica clinica = new Clinica(archivo, HOY);
            probarCarga(clinica);
            probarConsultas(clinica);
            probarModificaciones(clinica);

            // Reabrir: la persistencia debe conservar los cambios.
            Clinica reabierta = new Clinica(archivo, HOY);
            check(reabierta.buscarPorNombre("Maria Cazares") == 2, "persistencia de modificación");
            check(reabierta.cantidadVacunasRecibidas(1) == 1, "persistencia de vacunas");

            probarAltasConcurrentes(reabierta, archivo);
            probarMismoPaciente(reabierta);
            probarPacientesDistintos(reabierta);

            Clinica finalReabierta = new Clinica(archivo, HOY);
            check(finalReabierta.totalPacientes() == 83, "persistencia después de concurrencia");
            check(Files.readAllLines(archivo, StandardCharsets.UTF_8).size() == 83,
                    "el archivo tiene una línea por paciente");

            check(finalReabierta.eliminarPaciente(3), "eliminarPaciente");
            check(new Clinica(archivo, HOY).totalPacientes() == 82, "persistencia de la baja");

            System.out.println("Todas las pruebas de la clínica pasaron.");
        } finally {
            borrarRecursivo(directorio);
        }
    }

    private static void escribirArchivoInicial(Path archivo) throws IOException {
        List<String> lineas = List.of(
                "Nieves; 27/09/2020; hepatitis B, 25/05/2023, varicela, 22/01/2021; "
                        + "tetanos, 22/10/2027, difteria, 07/11/2027",
                "Juan; 14/04/2019; sarampion, 12/07/2020; ",
                "Maria; 27/09/2026; ; sarampion, 25/05/2027"
        );
        Files.write(archivo, lineas, StandardCharsets.UTF_8);
    }

    private static void probarCarga(Clinica c) {
        check(c.totalPacientes() == 3, "carga de 3 pacientes");
        check(c.buscarPorId(1) == 0 && c.buscarPorId(2) == 1 && c.buscarPorId(3) == 2,
                "ids asignados en orden");
        check(c.buscarPorId(99) == -1, "id inexistente");
        check(c.idEnIndice(1) == 2, "idEnIndice");
        check(c.buscarPorNombre("juan") == 1, "búsqueda por nombre sin mayúsculas");
        check(c.buscarPorFechaNacimiento(Utilidades.parsearFecha("27/09/2020")) == 0,
                "búsqueda por fecha");
        check(c.cantidadVacunasRecibidas(1) == 2 && c.cantidadVacunasProximas(1) == 2,
                "carga de vacunas");
    }

    private static void probarConsultas(Clinica c) {
        check(c.nombresAlfabetico().equals(List.of("Juan", "Maria", "Nieves")),
                "nombres en orden alfabético");

        check(c.mostrarTodo(0).size() == 3, "página 0 del listado");
        check(c.mostrarTodo(1).isEmpty(), "página 1 vacía");

        List<String> porEdad = c.rangoEdad(0, 120);
        check(porEdad.size() == 3 && porEdad.get(0).contains("Juan"),
                "rango de edades ordenado por edad descendente");

        check(c.conVacunaProxima7Dias("tetanos").isEmpty(), "tetanos lejano");
        c.agregarVacunaProxima(1, new Vacuna("tetanos", Utilidades.parsearFecha("18/10/2026")));
        check(c.conVacunaProxima7Dias("TETANOS").equals(List.of("Nieves")),
                "vacuna próxima en 7 días");

        check(c.contarVacunasAplicadaSemana() == 0, "sin vacunas en la última semana");
        c.registrarAplicacionVacuna(2,
                new Vacuna("triple viral", Utilidades.parsearFecha("10/10/2026")));
        check(c.contarVacunasAplicadaSemana() == 1, "una vacuna en la última semana");

        check(c.contarVacunasSiguienteMes("difteria") == 0, "difteria es en 2027");
        c.agregarVacunaProxima(2, new Vacuna("difteria", Utilidades.parsearFecha("05/11/2026")));
        check(c.contarVacunasSiguienteMes("DIFTERIA") == 1, "difteria el siguiente mes");

        c.registrarAplicacionVacuna(2,
                new Vacuna("influenza", Utilidades.parsearFecha("13/10/2026")));
        List<String> vacunados = c.vacunadosDelMes();
        check(vacunados.size() == 1 && vacunados.get(0).contains("Juan (7 anios)")
                && vacunados.get(0).contains("influenza"), "pacientes vacunados en el mes");

        check(c.informacion(99) == null, "información de id inexistente");
    }

    private static void probarModificaciones(Clinica c) {
        check(c.eliminarVacunaDePaciente("nieves", "VARICELA"), "eliminar vacuna por nombre");
        check(c.cantidadVacunasRecibidas(1) == 1, "vacuna eliminada");
        check(c.modificarVacunaDePaciente("nieves", "hepatitis B",
                new Vacuna("hepatitis B", Utilidades.parsearFecha("26/05/2023"))),
                "modificar vacuna");
        check(c.modificarNombre(3, "Maria Cazares"), "modificar nombre");
        check(c.modificarFechaNacimiento(3, Utilidades.parsearFecha("28/09/2026")),
                "modificar fecha de nacimiento");
        check(c.modificarNombre(99, "Nadie") == false, "modificar id inexistente");
    }

    private static void probarAltasConcurrentes(Clinica c, Path archivo) throws InterruptedException {
        int cantidad = 80;
        CountDownLatch inicio = new CountDownLatch(1);
        Thread[] hilos = new Thread[cantidad];
        Set<Integer> ids = java.util.Collections.synchronizedSet(new HashSet<>());

        for (int i = 0; i < cantidad; ++i) {
            final int k = i;
            hilos[i] = new Thread(() -> {
                try {
                    inicio.await();
                } catch (InterruptedException e) {
                    return;
                }
                ids.add(c.alta("Paciente " + k, Utilidades.parsearFecha("01/01/2015")));
            });
            hilos[i].start();
        }
        inicio.countDown();
        for (Thread hilo : hilos) {
            hilo.join();
        }
        check(ids.size() == cantidad, "80 ids únicos");
        check(c.totalPacientes() == 83, "80 altas concurrentes");
    }

    private static void probarMismoPaciente(Clinica c) throws InterruptedException {
        int cantidad = 200;
        int antes = c.cantidadVacunasRecibidas(1);
        CountDownLatch inicio = new CountDownLatch(1);
        Thread[] hilos = new Thread[cantidad];

        for (int i = 0; i < cantidad; ++i) {
            final int k = i;
            hilos[i] = new Thread(() -> {
                try {
                    inicio.await();
                } catch (InterruptedException e) {
                    return;
                }
                c.registrarAplicacionVacuna(1,
                        new Vacuna("concurrente" + k, Utilidades.parsearFecha("10/10/2026")));
            });
            hilos[i].start();
        }
        inicio.countDown();
        for (Thread hilo : hilos) {
            hilo.join();
        }
        check(c.cantidadVacunasRecibidas(1) == antes + cantidad,
                "200 vacunas en el mismo paciente sin pérdidas");
    }

    private static void probarPacientesDistintos(Clinica c) throws InterruptedException {
        int cantidad = 40;
        CountDownLatch inicio = new CountDownLatch(1);
        Thread[] hilos = new Thread[cantidad];

        for (int i = 0; i < cantidad; ++i) {
            final int id = 4 + i;   // patients creados en probarAltasConcurrentes
            hilos[i] = new Thread(() -> {
                try {
                    inicio.await();
                } catch (InterruptedException e) {
                    return;
                }
                if (!c.registrarAplicacionVacuna(id,
                        new Vacuna("unica", Utilidades.parsearFecha("10/10/2026")))) {
                    throw new AssertionError("No se pudo vacunar al paciente " + id);
                }
            });
            hilos[i].start();
        }
        inicio.countDown();
        for (Thread hilo : hilos) {
            hilo.join();
        }
        for (int i = 0; i < cantidad; ++i) {
            check(c.cantidadVacunasRecibidas(4 + i) == 1, "paciente " + (4 + i));
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
