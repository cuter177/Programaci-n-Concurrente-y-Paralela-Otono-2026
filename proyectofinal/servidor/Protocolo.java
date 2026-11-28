import java.time.LocalDate;
import java.util.ArrayList;
import java.util.List;

/**
 * Lenguaje compartido por servidor y cliente TCP.
 *
 * Una petición y una respuesta son cada una UNA línea terminada en '\n':
 *   ORDEN|arg1|arg2...   ->   OK|campo1|campo2...   o   ERR|mensaje
 *
 * El '|' separa campos, así que los textos que lo contienen se codifican
 * ('|' -> '\|', '\' -> '\\', salto de línea -> '\n'). Todo el escaping vive
 * aquí: servidor y cliente llaman exactamente a los mismos métodos.
 */
public final class Protocolo {

    // '|' que NO esté precedido de una barra: los '\|' son datos, no separadores.
    private static final String SEPARADOR = "(?<!\\\\)\\|";

    private Protocolo() {
    }

    /** Error de formato en la petición; el servidor lo convierte en ERR|... */
    public static class ProtocoloException extends RuntimeException {
        public ProtocoloException(String mensaje) {
            super(mensaje);
        }
    }

    /** Parte una línea por los '|' no escapados y descodifica cada campo. */
    public static String[] separar(String linea) {
        String[] partes = linea.split(SEPARADOR, -1);   // -1 conserva campos vacíos
        for (int i = 0; i < partes.length; ++i) {
            partes[i] = descodificar(partes[i]);
        }
        return partes;
    }

    /** Prepara un texto para viajar dentro de un campo. */
    public static String codificar(String texto) {
        if (texto == null) {
            return "";
        }
        return texto.replace("\\", "\\\\")
                .replace("|", "\\|")
                .replace("\r\n", "\\n")
                .replace("\n", "\\n")
                .replace("\r", "\\n");
    }

    /** Reconstruye el texto original, carácter a carácter. */
    public static String descodificar(String texto) {
        StringBuilder salida = new StringBuilder(texto.length());
        for (int i = 0; i < texto.length(); ++i) {
            char c = texto.charAt(i);
            if (c == '\\' && i + 1 < texto.length()) {
                char siguiente = texto.charAt(++i);
                switch (siguiente) {
                    case 'n':
                        salida.append('\n');
                        break;
                    case '|':
                        salida.append('|');
                        break;
                    case '\\':
                        salida.append('\\');
                        break;
                    default:
                        salida.append('\\').append(siguiente);
                }
            } else {
                salida.append(c);
            }
        }
        return salida.toString();
    }

    /** Respuesta correcta. Los campos ya deben venir codificados (o ser números). */
    public static String ok(String... campos) {
        if (campos.length == 0) {
            return "OK";
        }
        return "OK|" + String.join("|", campos);
    }

    public static String err(String mensaje) {
        return "ERR|" + codificar(mensaje);
    }

    /** Respuesta con lista: el contador va primero y luego los elementos codificados. */
    public static String lista(List<String> elementos) {
        List<String> campos = new ArrayList<>(elementos.size() + 1);
        campos.add(String.valueOf(elementos.size()));
        for (String elemento : elementos) {
            campos.add(codificar(elemento));
        }
        return ok(campos.toArray(new String[0]));
    }

    public static LocalDate fecha(String campo) {
        try {
            return Utilidades.parsearFecha(campo);
        } catch (IllegalArgumentException e) {
            throw new ProtocoloException(e.getMessage());
        }
    }

    public static int entero(String campo) {
        try {
            return Integer.parseInt(Utilidades.normalizar(campo));
        } catch (NumberFormatException e) {
            throw new ProtocoloException("Numero invalido: " + campo);
        }
    }

    public static void requerir(String[] partes, int cantidad) {
        if (partes.length < cantidad) {
            throw new ProtocoloException("Faltan argumentos para " + partes[0]);
        }
    }

    /** Los booleanos de la clínica viajan como 1/0. */
    public static String siNo(boolean valor) {
        return valor ? "1" : "0";
    }
}
