import java.time.LocalDate;
import java.util.List;
import java.util.concurrent.CountDownLatch;

/** Pruebas de la fase 1: modelo de datos, sin TCP todavía. */
public class PruebaModelo {

    public static void main(String[] args) throws InterruptedException {
        probarFechas();
        probarVacuna();
        probarPaciente();
        probarConcurrencia();
        System.out.println("Todas las pruebas del modelo pasaron.");
    }

    private static void probarFechas() {
        LocalDate fecha = Utilidades.parsearFecha("13/10/2026");
        check(fecha.getDayOfMonth() == 13 && fecha.getMonthValue() == 10
                && fecha.getYear() == 2026, "parsearFecha");

        check(Utilidades.formatearFecha(fecha).equals("13/10/2026"), "formatearFecha");

        // Año bisiesto sí, año no bisiesto no (ResolverStyle.STRICT).
        check(Utilidades.parsearFecha("29/02/2024") != null, "29/02/2024 debía ser válida");
        check(lanzaExcepcion(() -> Utilidades.parsearFecha("29/02/2023")), "29/02/2023 debía fallar");
        check(lanzaExcepcion(() -> Utilidades.parsearFecha("31/04/2026")), "31/04/2026 debía fallar");

        check(Utilidades.mismoNombre("Triple Viral", "triple viral"), "mismoNombre");
        check(!Utilidades.mismoNombre("tetanos", "difteria"), "mismoNombre distintos");
        check(Utilidades.esNombreValido("Ana Lopez"), "nombre válido");
        check(!Utilidades.esNombreValido("Ana|Lopez"), "el separador '|' debe rechazarse");

        LocalDate nacimiento = Utilidades.parsearFecha("10/05/2015");
        LocalDate hoy = Utilidades.parsearFecha("13/10/2026");
        check(Utilidades.calcularEdad(nacimiento, hoy) == 11, "calcularEdad");
        check(Utilidades.diasDiferencia(hoy, Utilidades.parsearFecha("20/10/2026")) == 7,
                "diasDiferencia");
        check(Utilidades.esMismoMes(Utilidades.parsearFecha("01/10/2026"), hoy), "esMismoMes");
    }

    private static void probarVacuna() {
        Vacuna v = new Vacuna("  triple viral ", Utilidades.parsearFecha("10/10/2026"));
        check(v.getNombre().equals("triple viral"), "Vacuna normaliza el nombre");
        check(v.toString().equals("triple viral, 10/10/2026"), "Vacuna.toString");
        check(v.equals(new Vacuna("triple viral", Utilidades.parsearFecha("10/10/2026"))),
                "Vacuna.equals");
        check(lanzaExcepcion(() -> new Vacuna("", LocalDate.now())), "vacuna sin nombre");
        check(lanzaExcepcion(() -> new Vacuna("x", null)), "vacuna sin fecha");
    }

    private static void probarPaciente() {
        Paciente p = new Paciente(7, "Ana Lopez", Utilidades.parsearFecha("10/05/2015"));
        check(p.getId() == 7 && p.getNombre().equals("Ana Lopez"), "datos del paciente");

        p.setNombre("Ana Lopez Gomez");
        p.setFechaNacimiento(Utilidades.parsearFecha("11/05/2015"));
        check(p.getNombre().equals("Ana Lopez Gomez"), "setNombre");
        check(Utilidades.formatearFecha(p.getFechaNacimiento()).equals("11/05/2015"),
                "setFechaNacimiento");

        Vacuna aplicada = new Vacuna("triple viral", Utilidades.parsearFecha("10/10/2026"));
        Vacuna proxima = new Vacuna("tetanos", Utilidades.parsearFecha("18/10/2026"));
        p.agregarVacunaRecibida(aplicada);
        p.agregarVacunaProxima(proxima);

        check(p.getVacunasRecibidas().size() == 1, "agregarVacunaRecibida");
        check(p.getVacunasProximas().size() == 1, "agregarVacunaProxima");

        // Las copias defensivas: modificar la lista devuelta no afecta al paciente.
        List<Vacuna> copia = p.getVacunasRecibidas();
        copia.clear();
        check(p.getVacunasRecibidas().size() == 1, "copia defensiva");

        check(p.modificarVacunaRecibida("TRIPLE VIRAL",
                new Vacuna("triple viral", Utilidades.parsearFecha("12/10/2026"))),
                "modificarVacunaRecibida ignora mayúsculas");
        check(Utilidades.formatearFecha(p.getVacunasRecibidas().get(0).getFecha())
                .equals("12/10/2026"), "modificación aplicada");

        check(p.eliminarVacunaProxima("tetanos"), "eliminarVacunaProxima");
        check(p.getVacunasProximas().isEmpty(), "lista próxima vacía");
        check(!p.eliminarVacunaRecibida("sarampion"), "eliminar inexistente devuelve false");

        check(p.toString().contains("Ana Lopez Gomez"), "Paciente.toString");
    }

    private static void probarConcurrencia() throws InterruptedException {
        // 200 hilos agregan vacunas distintas al MISMO paciente: no debe perderse ninguna.
        Paciente p = new Paciente(1, "Paciente Concurrente", Utilidades.parsearFecha("01/01/2000"));
        int cantidad = 200;
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
                p.agregarVacunaRecibida(new Vacuna("vacuna" + k, LocalDate.of(2026, 10, 10)));
                p.getNombre();                 // lecturas simultáneas con escrituras
                p.getVacunasRecibidas().size();
            });
            hilos[i].start();
        }
        inicio.countDown();
        for (Thread hilo : hilos) {
            hilo.join();
        }
        check(p.getVacunasRecibidas().size() == cantidad,
                "concurrencia sobre el mismo paciente");
    }

    private static void check(boolean condicion, String mensaje) {
        if (!condicion) {
            throw new AssertionError("Fallo: " + mensaje);
        }
    }

    private static boolean lanzaExcepcion(Runnable accion) {
        try {
            accion.run();
            return false;
        } catch (IllegalArgumentException e) {
            return true;
        }
    }
}
