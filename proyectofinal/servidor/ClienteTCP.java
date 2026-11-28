import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

/**
 * Cliente TCP por línea de comandos.
 *   java ClienteTCP [host] [puerto] ORDEN arg1 arg2...   -> una orden y sale
 *   java ClienteTCP [host] [puerto]                     -> órdenes a mano
 * Para un menú numerado, usa ClienteConsola.
 */
public class ClienteTCP {

    private static final String HOST = "127.0.0.1";
    private static final int PUERTO = 5000;

    public static void main(String[] args) {
        String host = args.length > 0 ? args[0] : HOST;
        int puerto = args.length > 1 ? Protocolo.entero(args[1]) : PUERTO;

        try (ConexionTCP conexion = new ConexionTCP(host, puerto)) {
            // Modo de una sola orden: ideal para scripts y pruebas.
            if (args.length > 2) {
                String[] orden = Arrays.copyOfRange(args, 2, args.length);
                System.out.println(respuesta(conexion.enviar(orden)));
                return;
            }

            // Modo interactivo de órdenes escritas a mano.
            BufferedReader consola = new BufferedReader(
                    new InputStreamReader(System.in, StandardCharsets.UTF_8));
            System.out.println("Escribe ordenes como: ALTA|Ana|01/01/2015");
            System.out.println("'SALIR' termina la conexion.");
            String linea;
            while ((linea = consola.readLine()) != null) {
                System.out.println(respuesta(conexion.enviar(linea)));
                if (linea.equalsIgnoreCase("SALIR")) {
                    break;
                }
            }
        } catch (IOException e) {
            System.out.println("Error de red: " + e.getMessage());
        }
    }

    private static String respuesta(String linea) {
        return linea == null ? "El servidor cerro la conexion." : linea;
    }
}
