import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.time.LocalDate;

/**
 * Servidor TCP de la fase 4: escucha conexiones y despacha cada línea a Clinica.
 * El idioma de las órdenes está en Protocolo.java; aquí solo se traduce texto
 * a llamadas al modelo.
 */
public class ServidorTCP {

    private final Clinica clinica;
    private final ServerSocket servidor;

    public ServidorTCP(int puerto, Path archivo, LocalDate fecha) throws IOException {
        this.clinica = new Clinica(archivo, fecha);
        this.servidor = new ServerSocket(puerto);
    }

    /** Un hilo por cliente; todos comparten la MISMA Clinica. */
    public void servir() throws IOException {
        try (ServerSocket socketServidor = this.servidor) {
            System.out.println("Servidor escuchando en el puerto " + socketServidor.getLocalPort());
            while (true) {
                Socket cliente = socketServidor.accept();
                Thread hilo = new Thread(new Atendedor(cliente, clinica), "cliente-tcp");
                hilo.setDaemon(true);   // un cliente colgado no impide apagar el servidor
                hilo.start();
            }
        } finally {
            clinica.cerrar();
        }
    }

    /** Cierra el socket de escucha: servir() sale del bucle y libera el pool. */
    public void cerrar() throws IOException {
        servidor.close();
        clinica.cerrar();
    }

    /** java ServidorTCP [puerto] [archivo] [dd/MM/aaaa] */
    public static void main(String[] args) throws Exception {
        int puerto = args.length > 0 ? Protocolo.entero(args[0]) : 5000;
        Path archivo = Path.of(args.length > 1 ? args[1] : "pacientes.txt");
        LocalDate fecha = args.length > 2 ? Protocolo.fecha(args[2]) : LocalDate.now();
        new ServidorTCP(puerto, archivo, fecha).servir();
    }

    /** Traduce una línea del protocolo a una respuesta. Paquete privado para las pruebas. */
    static String despachar(String linea, Clinica clinica) {
        try {
            String[] p = Protocolo.separar(linea);
            if (p.length == 0 || p[0].isEmpty()) {
                return Protocolo.err("Orden vacia");
            }
            switch (p[0]) {
                case "ALTA":
                    Protocolo.requerir(p, 3);
                    return Protocolo.ok(String.valueOf(
                            clinica.alta(p[1], Protocolo.fecha(p[2]))));
                case "BAJA":
                    Protocolo.requerir(p, 2);
                    return Protocolo.ok(Protocolo.siNo(
                            clinica.eliminarPaciente(Protocolo.entero(p[1]))));
                case "NOMBRE":
                    Protocolo.requerir(p, 3);
                    return Protocolo.ok(Protocolo.siNo(
                            clinica.modificarNombre(Protocolo.entero(p[1]), p[2])));
                case "NACIMIENTO":
                    Protocolo.requerir(p, 3);
                    return Protocolo.ok(Protocolo.siNo(clinica.modificarFechaNacimiento(
                            Protocolo.entero(p[1]), Protocolo.fecha(p[2]))));
                case "VACUNA_RECIBIDA":
                    Protocolo.requerir(p, 4);
                    return Protocolo.ok(Protocolo.siNo(clinica.registrarAplicacionVacuna(
                            Protocolo.entero(p[1]), new Vacuna(p[2], Protocolo.fecha(p[3])))));
                case "VACUNA_PROXIMA":
                    Protocolo.requerir(p, 4);
                    return Protocolo.ok(Protocolo.siNo(clinica.agregarVacunaProxima(
                            Protocolo.entero(p[1]), new Vacuna(p[2], Protocolo.fecha(p[3])))));
                case "ELIMINAR_VACUNA":
                    Protocolo.requerir(p, 3);
                    return Protocolo.ok(Protocolo.siNo(
                            clinica.eliminarVacunaDePaciente(p[1], p[2])));
                case "MODIFICAR_VACUNA":
                    Protocolo.requerir(p, 5);
                    return Protocolo.ok(Protocolo.siNo(clinica.modificarVacunaDePaciente(
                            p[1], p[2], new Vacuna(p[3], Protocolo.fecha(p[4])))));
                case "INFO": {
                    Protocolo.requerir(p, 2);
                    String texto = clinica.informacion(Protocolo.entero(p[1]));
                    return texto == null ? Protocolo.ok("0")
                                         : Protocolo.ok(Protocolo.codificar(texto));
                }
                case "CANT_RECIBIDAS":
                    Protocolo.requerir(p, 2);
                    return Protocolo.ok(String.valueOf(
                            clinica.cantidadVacunasRecibidas(Protocolo.entero(p[1]))));
                case "CANT_PROXIMAS":
                    Protocolo.requerir(p, 2);
                    return Protocolo.ok(String.valueOf(
                            clinica.cantidadVacunasProximas(Protocolo.entero(p[1]))));
                case "NOMBRES":
                    return Protocolo.lista(clinica.nombresAlfabetico());
                case "MOSTRAR":
                    Protocolo.requerir(p, 2);
                    return Protocolo.lista(clinica.mostrarTodo(Protocolo.entero(p[1])));
                case "RANGO_EDAD":
                    Protocolo.requerir(p, 3);
                    return Protocolo.lista(clinica.rangoEdad(
                            Protocolo.entero(p[1]), Protocolo.entero(p[2])));
                case "PROXIMAS_7":
                    Protocolo.requerir(p, 2);
                    return Protocolo.lista(clinica.conVacunaProxima7Dias(p[1]));
                case "CONTAR_APLICADAS":
                    return Protocolo.ok(String.valueOf(clinica.contarVacunasAplicadaSemana()));
                case "CONTAR_MES":
                    Protocolo.requerir(p, 2);
                    return Protocolo.ok(String.valueOf(
                            clinica.contarVacunasSiguienteMes(p[1])));
                case "VACUNADOS_MES":
                    return Protocolo.lista(clinica.vacunadosDelMes());
                case "INDICE_ID":
                    Protocolo.requerir(p, 2);
                    return Protocolo.ok(String.valueOf(
                            clinica.buscarPorId(Protocolo.entero(p[1]))));
                case "INDICE_NOMBRE":
                    Protocolo.requerir(p, 2);
                    return Protocolo.ok(String.valueOf(clinica.buscarPorNombre(p[1])));
                case "INDICE_NACIMIENTO":
                    Protocolo.requerir(p, 2);
                    return Protocolo.ok(String.valueOf(
                            clinica.buscarPorFechaNacimiento(Protocolo.fecha(p[1]))));
                case "ID_EN_INDICE":
                    Protocolo.requerir(p, 2);
                    return Protocolo.ok(String.valueOf(
                            clinica.idEnIndice(Protocolo.entero(p[1]))));
                case "TOTAL":
                    return Protocolo.ok(String.valueOf(clinica.totalPacientes()));
                case "FECHA":
                    return Protocolo.ok(Utilidades.formatearFecha(clinica.getFechaActual()));
                case "SET_FECHA":
                    Protocolo.requerir(p, 2);
                    clinica.setFechaActual(Protocolo.fecha(p[1]));
                    return Protocolo.ok("1");
                case "SALIR":
                    return "OK|adios";
                default:
                    return Protocolo.err("Orden desconocida: " + p[0]);
            }
        } catch (Protocolo.ProtocoloException | IllegalArgumentException e) {
            return Protocolo.err(e.getMessage());
        }
    }

    /** Atiende a un cliente hasta que cierra o envía SALIR. */
    private static final class Atendedor implements Runnable {
        private final Socket cliente;
        private final Clinica clinica;

        Atendedor(Socket cliente, Clinica clinica) {
            this.cliente = cliente;
            this.clinica = clinica;
        }

        @Override
        public void run() {
            try (Socket socket = this.cliente;
                 BufferedReader entrada = new BufferedReader(new InputStreamReader(
                         socket.getInputStream(), StandardCharsets.UTF_8));
                 PrintWriter salida = new PrintWriter(new OutputStreamWriter(
                         socket.getOutputStream(), StandardCharsets.UTF_8), true)) {
                String linea;
                while ((linea = entrada.readLine()) != null) {
                    salida.println(despachar(linea, clinica));
                    if (linea.startsWith("SALIR")) {
                        break;
                    }
                }
            } catch (IOException e) {
                System.out.println("Cliente desconectado: " + e.getMessage());
            }
        }
    }
}
