import java.io.BufferedReader;
import java.io.EOFException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.time.LocalDate;

/**
 * Interfaz de puro texto para el cliente TCP: menús numerados que construyen
 * las órdenes del protocolo sin que el usuario tenga que memorizarlas.
 * Mantiene UNA sola conexión abierta durante toda la sesión.
 */
public class ClienteConsola implements AutoCloseable {

    private static final String HOST = "127.0.0.1";
    private static final int PUERTO = 5000;

    private final BufferedReader consola = new BufferedReader(
            new InputStreamReader(System.in, StandardCharsets.UTF_8));
    private final ConexionTCP conexion;

    private ClienteConsola(String host, int puerto) throws IOException {
        this.conexion = new ConexionTCP(host, puerto);
    }

    public static void main(String[] args) {
        String host = args.length > 0 ? args[0] : HOST;
        int puerto = args.length > 1 ? Protocolo.entero(args[1]) : PUERTO;
        try (ClienteConsola cliente = new ClienteConsola(host, puerto)) {
            cliente.menuPrincipal();
        } catch (EOFException e) {
            System.out.println("\nHasta luego.");
        } catch (IOException e) {
            System.out.println("Error de red: " + e.getMessage());
        }
    }

    @Override
    public void close() {
        conexion.close();
    }

    // ---------- Menús ----------

    private void menuPrincipal() throws IOException {
        while (true) {
            System.out.println();
            System.out.println("==== CLINICA DE VACUNAS ====");
            System.out.println("1) Pacientes   2) Vacunas   3) Consultas   4) Busquedas"
                    + "   5) Servidor   0) Salir");
            switch (leer("Opcion: ")) {
                case "1" -> menuPacientes();
                case "2" -> menuVacunas();
                case "3" -> menuConsultas();
                case "4" -> menuBusquedas();
                case "5" -> menuServidor();
                case "0" -> {
                    System.out.println("Hasta luego.");
                    return;
                }
                default -> System.out.println("Opcion no valida.");
            }
        }
    }

    private void menuPacientes() throws IOException {
        while (true) {
            System.out.println();
            System.out.println("-- Pacientes --");
            System.out.println("1) Alta  2) Baja  3) Cambiar nombre  4) Cambiar nacimiento");
            System.out.println("5) Informacion  6) Vacunas recibidas  7) Vacunas proximas");
            System.out.println("8) Total  0) Volver");
            switch (leer("Opcion: ")) {
                case "1" -> {
                    String nombre = leerNombre("Nombre: ");
                    LocalDate nacimiento = leerFecha("Nacimiento (dd/MM/aaaa): ");
                    String id = pedir("ALTA", nombre, Utilidades.formatearFecha(nacimiento));
                    if (id != null) {
                        System.out.println("Id asignado: " + id);
                    }
                }
                case "2" -> accion("BAJA", String.valueOf(leerEntero("Id: ")));
                case "3" -> accion("NOMBRE", String.valueOf(leerEntero("Id: ")),
                        leerNombre("Nuevo nombre: "));
                case "4" -> accion("NACIMIENTO", String.valueOf(leerEntero("Id: ")),
                        Utilidades.formatearFecha(leerFecha("Nueva fecha: ")));
                case "5" -> {
                    String texto = pedir("INFO", String.valueOf(leerEntero("Id: ")));
                    if (texto != null) {
                        System.out.println(texto.equals("0") ? "No existe ese paciente." : texto);
                    }
                }
                case "6" -> {
                    String n = pedir("CANT_RECIBIDAS", String.valueOf(leerEntero("Id: ")));
                    if (n != null) {
                        System.out.println("Recibidas: " + n);
                    }
                }
                case "7" -> {
                    String n = pedir("CANT_PROXIMAS", String.valueOf(leerEntero("Id: ")));
                    if (n != null) {
                        System.out.println("Proximas: " + n);
                    }
                }
                case "8" -> {
                    String n = pedir("TOTAL");
                    if (n != null) {
                        System.out.println("Total de pacientes: " + n);
                    }
                }
                case "0" -> {
                    return;
                }
                default -> System.out.println("Opcion no valida.");
            }
        }
    }

    private void menuVacunas() throws IOException {
        while (true) {
            System.out.println();
            System.out.println("-- Vacunas --");
            System.out.println("1) Registrar recibida  2) Agregar proxima");
            System.out.println("3) Eliminar recibida  4) Modificar recibida  0) Volver");
            switch (leer("Opcion: ")) {
                case "1" -> accion("VACUNA_RECIBIDA", String.valueOf(leerEntero("Id: ")),
                        leerNombre("Nombre de la vacuna: "),
                        Utilidades.formatearFecha(leerFecha("Fecha (dd/MM/aaaa): ")));
                case "2" -> accion("VACUNA_PROXIMA", String.valueOf(leerEntero("Id: ")),
                        leerNombre("Nombre de la vacuna: "),
                        Utilidades.formatearFecha(leerFecha("Fecha (dd/MM/aaaa): ")));
                case "3" -> accion("ELIMINAR_VACUNA", leerNombre("Nombre del paciente: "),
                        leerNombre("Nombre de la vacuna: "));
                case "4" -> accion("MODIFICAR_VACUNA", leerNombre("Nombre del paciente: "),
                        leerNombre("Vacuna a reemplazar: "), leerNombre("Nombre nuevo: "),
                        Utilidades.formatearFecha(leerFecha("Fecha nueva: ")));
                case "0" -> {
                    return;
                }
                default -> System.out.println("Opcion no valida.");
            }
        }
    }

    private void menuConsultas() throws IOException {
        while (true) {
            System.out.println();
            System.out.println("-- Consultas --");
            System.out.println("1) Nombres  2) Mostrar paginado  3) Rango de edades");
            System.out.println("4) Proximos 7 dias  5) Aplicadas semana  6) Siguiente mes");
            System.out.println("7) Vacunados del mes  0) Volver");
            switch (leer("Opcion: ")) {
                case "1" -> mostrarLista("NOMBRES");
                case "2" -> menuMostrar();
                case "3" -> mostrarLista("RANGO_EDAD",
                        String.valueOf(leerEntero("Edad minima: ")),
                        String.valueOf(leerEntero("Edad maxima: ")));
                case "4" -> mostrarLista("PROXIMAS_7", leerNombre("Nombre de la vacuna: "));
                case "5" -> {
                    String n = pedir("CONTAR_APLICADAS");
                    if (n != null) {
                        System.out.println("Aplicadas esta semana: " + n);
                    }
                }
                case "6" -> {
                    String n = pedir("CONTAR_MES", leerNombre("Nombre de la vacuna: "));
                    if (n != null) {
                        System.out.println("En el proximo mes: " + n);
                    }
                }
                case "7" -> mostrarLista("VACUNADOS_MES");
                case "0" -> {
                    return;
                }
                default -> System.out.println("Opcion no valida.");
            }
        }
    }

    private void menuMostrar() throws IOException {
        int pagina = 0;
        while (true) {
            String[] p = Protocolo.separar(conexion.enviar("MOSTRAR", String.valueOf(pagina)));
            if (p[0].equals("ERR")) {
                System.out.println("Error: " + p[1]);
                return;
            }
            int n = Integer.parseInt(p[1]);
            if (n == 0) {
                System.out.println("(sin resultados)");
                return;
            }
            System.out.println("-- Pagina " + pagina + " --");
            for (int i = 0; i < n; ++i) {
                System.out.println((i + 1) + ". " + p[i + 2]);
            }
            if (!leer("Siguiente pagina? (s/n): ").equalsIgnoreCase("s")) {
                return;
            }
            ++pagina;
        }
    }

    private void menuBusquedas() throws IOException {
        while (true) {
            System.out.println();
            System.out.println("-- Busquedas --");
            System.out.println("1) Indice por id  2) Indice por nombre  3) Indice por fecha");
            System.out.println("4) Id en indice  0) Volver");
            switch (leer("Opcion: ")) {
                case "1" -> indice("INDICE_ID", String.valueOf(leerEntero("Id: ")));
                case "2" -> indice("INDICE_NOMBRE", leerNombre("Nombre: "));
                case "3" -> indice("INDICE_NACIMIENTO",
                        Utilidades.formatearFecha(leerFecha("Fecha (dd/MM/aaaa): ")));
                case "4" -> {
                    String id = pedir("ID_EN_INDICE", String.valueOf(leerEntero("Indice: ")));
                    if (id != null) {
                        System.out.println("Id: " + id);
                    }
                }
                case "0" -> {
                    return;
                }
                default -> System.out.println("Opcion no valida.");
            }
        }
    }

    private void menuServidor() throws IOException {
        while (true) {
            System.out.println();
            System.out.println("-- Servidor --");
            System.out.println("1) Ver fecha  2) Cambiar fecha  0) Volver");
            switch (leer("Opcion: ")) {
                case "1" -> {
                    String fecha = pedir("FECHA");
                    if (fecha != null) {
                        System.out.println("Fecha actual: " + fecha);
                    }
                }
                case "2" -> accion("SET_FECHA",
                        Utilidades.formatearFecha(leerFecha("Nueva fecha (dd/MM/aaaa): ")));
                case "0" -> {
                    return;
                }
                default -> System.out.println("Opcion no valida.");
            }
        }
    }

    // ---------- Auxiliares ----------

    private void indice(String orden, String argumento) throws IOException {
        String i = pedir(orden, argumento);
        if (i != null) {
            System.out.println("Indice: " + i);
        }
    }

    /** Ejecuta una orden booleana y lo dice en lenguaje natural. */
    private void accion(String... campos) throws IOException {
        String valor = pedir(campos);
        if (valor != null) {
            System.out.println(valor.equals("1") ? "Hecho."
                    : valor.equals("0") ? "No se encontro o no aplico." : valor);
        }
    }

    /** Devuelve el primer campo de la respuesta, o null si fue ERR (ya impreso). */
    private String pedir(String... campos) throws IOException {
        String[] p = Protocolo.separar(conexion.enviar(campos));
        if (p[0].equals("ERR")) {
            System.out.println("Error: " + (p.length > 1 ? p[1] : ""));
            return null;
        }
        return p.length > 1 ? p[1] : "";
    }

    /** Imprime una respuesta de lista: contador primero y elementos después. */
    private void mostrarLista(String... campos) throws IOException {
        String[] p = Protocolo.separar(conexion.enviar(campos));
        if (p[0].equals("ERR")) {
            System.out.println("Error: " + p[1]);
            return;
        }
        int n = Integer.parseInt(p[1]);
        if (n == 0) {
            System.out.println("(sin resultados)");
            return;
        }
        for (int i = 0; i < n; ++i) {
            System.out.println((i + 1) + ". " + p[i + 2]);
        }
    }

    private String leer(String mensaje) throws IOException {
        System.out.print(mensaje);
        System.out.flush();
        String linea = consola.readLine();
        if (linea == null) {
            throw new EOFException();
        }
        return Utilidades.normalizar(linea);
    }

    private int leerEntero(String mensaje) throws IOException {
        while (true) {
            try {
                return Protocolo.entero(leer(mensaje));
            } catch (Protocolo.ProtocoloException e) {
                System.out.println("Escribe un numero entero.");
            }
        }
    }

    private LocalDate leerFecha(String mensaje) throws IOException {
        while (true) {
            try {
                return Utilidades.parsearFecha(leer(mensaje));
            } catch (IllegalArgumentException e) {
                System.out.println("Fecha invalida. Usa dd/MM/aaaa.");
            }
        }
    }

    private String leerNombre(String mensaje) throws IOException {
        while (true) {
            String texto = leer(mensaje);
            if (Utilidades.esNombreValido(texto)) {
                return texto;
            }
            System.out.println("El nombre no puede estar vacio ni contener '|'.");
        }
    }
}
