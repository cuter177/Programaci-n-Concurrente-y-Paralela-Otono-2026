import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.Socket;
import java.nio.charset.StandardCharsets;

/**
 * Una conexión TCP reutilizable: una línea de petición, una de respuesta.
 * La usan ClienteTCP (una orden) y ClienteConsola (toda una sesión), para no
 * duplicar el código de red en dos sitios.
 */
public class ConexionTCP implements AutoCloseable {

    private static final int TIMEOUT_MS = 15_000;

    private final Socket socket;
    private final PrintWriter salida;
    private final BufferedReader entrada;

    public ConexionTCP(String host, int puerto) throws IOException {
        this.socket = new Socket(host, puerto);
        this.socket.setSoTimeout(TIMEOUT_MS);
        this.salida = new PrintWriter(new OutputStreamWriter(
                socket.getOutputStream(), StandardCharsets.UTF_8), true);
        this.entrada = new BufferedReader(new InputStreamReader(
                socket.getInputStream(), StandardCharsets.UTF_8));
    }

    /** Envía una orden ya formada y devuelve la línea de respuesta. */
    public String enviar(String orden) throws IOException {
        salida.println(orden);
        String respuesta = entrada.readLine();
        if (respuesta == null) {
            throw new IOException("El servidor cerró la conexión");
        }
        return respuesta;
    }

    /** Envía una orden formada por campos, uniéndolos con '|'. */
    public String enviar(String... campos) throws IOException {
        return enviar(String.join("|", campos));
    }

    /** Despide al servidor y suelta el socket. */
    @Override
    public void close() {
        try {
            salida.println("SALIR");
        } catch (Exception ignorado) {
            // si la conexión ya está rota, no hay nada que enviar
        }
        try {
            socket.close();
        } catch (IOException ignorado) {
            // el socket ya estaba cerrado
        }
    }
}
