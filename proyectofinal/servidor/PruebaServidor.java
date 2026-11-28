import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.time.LocalDate;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicReference;

/**
 * Pruebas de las fases 4 y 5: servidor TCP, protocolo y clientes concurrentes.
 * El servidor se arranca en el mismo proceso para no depender de terminales.
 */
public class PruebaServidor {

    private static final LocalDate HOY = Utilidades.parsearFecha("13/10/2026");

    public static void main(String[] args) throws Exception {
        int puerto = puertoLibre();
        Path directorio = Files.createTempDirectory("servidor-test-");
        Path archivo = directorio.resolve("pacientes.txt");
        ServidorTCP servidor = new ServidorTCP(puerto, archivo, HOY);
        AtomicReference<Throwable> errorServidor = new AtomicReference<>();
        Thread hiloServidor = new Thread(() -> {
            try {
                servidor.servir();
            } catch (IOException e) {
                errorServidor.compareAndSet(null, e);
            }
        }, "servidor-tcp");
        hiloServidor.setDaemon(true);
        hiloServidor.start();
        try {
            probarOrdenesBasicas(puerto);
            probarConexionReutilizable(puerto);
            probarClientesConcurrentes(puerto);
            check(new Clinica(archivo, HOY).totalPacientes() == 23,
                    "persistencia tras clientes concurrentes");
            if (errorServidor.get() != null) {
                throw new AssertionError("Falló el servidor", errorServidor.get());
            }
            System.out.println("Todas las pruebas del servidor TCP pasaron.");
        } finally {
            servidor.cerrar();
            hiloServidor.join(2000);
            borrarRecursivo(directorio);
        }
    }

    private static void probarOrdenesBasicas(int puerto) throws IOException {
        check(enviar(puerto, "ALTA", "Ana", "01/01/2015").equals("OK|1"), "ALTA id 1");
        check(enviar(puerto, "ALTA", "Luis", "02/02/2016").equals("OK|2"), "ALTA id 2");

        String info = enviar(puerto, "INFO", "1");
        check(info.startsWith("OK|") && info.contains("Ana"), "INFO de Ana");
        check(enviar(puerto, "INFO", "99").equals("OK|0"), "INFO inexistente");
        check(enviar(puerto, "VACUNA_RECIBIDA", "1", "gripe", "13/10/2026").equals("OK|1"),
                "registrar vacuna recibida");
        check(enviar(puerto, "CANT_RECIBIDAS", "1").equals("OK|1"), "contar recibidas");
        check(enviar(puerto, "CANT_PROXIMAS", "1").equals("OK|0"), "contar próximas");
        check(enviar(puerto, "NOMBRES").equals("OK|2|Ana|Luis"), "nombres alfabéticos");
        check(enviar(puerto, "MOSTRAR", "0").startsWith("OK|2|"), "mostrarTodo");
        check(enviar(puerto, "TOTAL").equals("OK|2"), "total de pacientes");
        check(enviar(puerto, "FECHA").equals("OK|13/10/2026"), "fecha actual");
        check(enviar(puerto, "INDICE_NOMBRE", "Ana").equals("OK|0"), "índice por nombre");
        check(enviar(puerto, "ID_EN_INDICE", "0").equals("OK|1"), "id en índice");
        check(enviar(puerto, "RANGO_EDAD", "10", "11").startsWith("OK|2|"), "rango de edades");
        check(enviar(puerto, "PROXIMAS_7", "gripe").startsWith("OK|0"), "próximos 7 días");
        check(enviar(puerto, "CONTAR_APLICADAS").equals("OK|1"), "aplicadas esta semana");
        check(enviar(puerto, "CONTAR_MES", "nada").equals("OK|0"), "contar siguiente mes");
        check(enviar(puerto, "VACUNADOS_MES").startsWith("OK|1|"), "vacunados del mes");
        check(enviar(puerto, "SET_FECHA", "14/10/2026").equals("OK|1"), "cambiar fecha");
        check(enviar(puerto, "FECHA").equals("OK|14/10/2026"), "fecha cambiada");
        check(enviar(puerto, "SET_FECHA", "13/10/2026").equals("OK|1"), "restaurar fecha");

        check(enviar(puerto, "ALTA", "Ana", "fecha-mal").startsWith("ERR|"), "fecha inválida");
        check(enviar(puerto, "ALTA", "Ana").startsWith("ERR|"), "faltan argumentos");
        check(enviar(puerto, "INEXISTENTE").startsWith("ERR|"), "orden desconocida");
        check(enviar(puerto, "SALIR").equals("OK|adios"), "SALIR");

        // El toString() de Paciente contiene " | " y los textos del mes contienen saltos.
        check(enviar(puerto, "VACUNA_RECIBIDA", "1", "tetanos", "01/01/2026").equals("OK|1"),
                "segunda vacuna");
        String respuestaInfo = enviar(puerto, "INFO", "1");
        check(respuestaInfo.contains("\\|"), "el '|' interno viaja escapado");
        check(Protocolo.separar(respuestaInfo)[1].contains(" | "), "el '|' interno se recupera");

        String respuestaMes = enviar(puerto, "VACUNADOS_MES");
        check(respuestaMes.contains("\\n"), "el salto de línea viaja escapado");
        // En una lista el campo 1 es el contador y el texto está en el 2.
        check(Protocolo.separar(respuestaMes)[2].contains("\n"), "el salto se recupera");
    }

    /** Una sola conexión sirve para muchas órdenes: base del menú interactivo. */
    private static void probarConexionReutilizable(int puerto) throws IOException {
        try (ConexionTCP conexion = new ConexionTCP("127.0.0.1", puerto)) {
            check(conexion.enviar("ALTA", "Reutilizable", "01/01/2015").equals("OK|3"),
                    "ALTA en conexión reutilizable");
            check(conexion.enviar("TOTAL").equals("OK|3"), "TOTAL en la misma conexión");
            check(conexion.enviar("NOMBRES").endsWith("|Reutilizable"),
                    "NOMBRES en la misma conexión");
        }
    }

    private static void probarClientesConcurrentes(int puerto)
            throws InterruptedException, IOException {
        int cantidad = 20;
        CountDownLatch inicio = new CountDownLatch(1);
        Set<Integer> ids = Collections.synchronizedSet(new HashSet<>());
        AtomicReference<Throwable> error = new AtomicReference<>();
        Thread[] hilos = new Thread[cantidad];
        for (int i = 0; i < cantidad; ++i) {
            final int k = i;
            hilos[i] = new Thread(() -> {
                try {
                    inicio.await();
                    String[] campos = Protocolo.separar(
                            enviar(puerto, "ALTA", "C" + k, "01/01/2015"));
                    if (!campos[0].equals("OK")) {
                        throw new AssertionError("ALTA falló: " + campos[0]);
                    }
                    ids.add(Integer.parseInt(campos[1]));
                } catch (Throwable t) {
                    error.compareAndSet(null, t);
                }
            }, "cliente-alta-" + i);
            hilos[i].start();
        }
        inicio.countDown();
        for (Thread hilo : hilos) {
            hilo.join();
        }
        if (error.get() != null) {
            throw new AssertionError("Falló un alta concurrente", error.get());
        }
        check(ids.size() == cantidad, "20 ids únicos por TCP");
        check(enviar(puerto, "TOTAL").equals("OK|23"), "23 pacientes tras las altas");

        // 20 clientes modificando el MISMO paciente: su monitor los serializa.
        AtomicReference<Throwable> errorVacunas = new AtomicReference<>();
        Thread[] vacunas = new Thread[cantidad];
        for (int i = 0; i < cantidad; ++i) {
            vacunas[i] = new Thread(() -> {
                try {
                    String respuesta = enviar(puerto, "VACUNA_RECIBIDA", "1", "gripe",
                            "13/10/2026");
                    if (!respuesta.equals("OK|1")) {
                        throw new AssertionError("Vacuna falló: " + respuesta);
                    }
                } catch (Throwable t) {
                    errorVacunas.compareAndSet(null, t);
                }
            }, "cliente-vacuna-" + i);
            vacunas[i].start();
        }
        for (Thread hilo : vacunas) {
            hilo.join();
        }
        if (errorVacunas.get() != null) {
            throw new AssertionError("Falló una vacuna concurrente", errorVacunas.get());
        }
        check(enviar(puerto, "CANT_RECIBIDAS", "1").equals("OK|22"),
                "22 vacunas en el paciente 1");
    }

    private static int puertoLibre() throws IOException {
        try (ServerSocket s = new ServerSocket(0)) {
            return s.getLocalPort();
        }
    }

    private static String enviar(int puerto, String... campos) throws IOException {
        try (Socket socket = new Socket("127.0.0.1", puerto);
             PrintWriter salida = new PrintWriter(new OutputStreamWriter(
                     socket.getOutputStream(), StandardCharsets.UTF_8), true);
             BufferedReader entrada = new BufferedReader(new InputStreamReader(
                     socket.getInputStream(), StandardCharsets.UTF_8))) {
            socket.setSoTimeout(15_000);
            salida.println(String.join("|", campos));
            String respuesta = entrada.readLine();
            if (respuesta == null) {
                throw new IOException("El servidor cerró sin responder");
            }
            return respuesta;
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
