import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

/**
 * La sala NO tiene un cerrojo global. La matriz es final y se construye
 * completa antes de lanzar los hilos; cada Asiento se protege a sí mismo.
 * Así, reservar (0,0) y reservar (2,4) son operaciones concurrentes reales.
 */
public class Sala {
    private final int filas;
    private final int columnas;
    private final Asiento[][] asientos;

    public Sala(int filas, int columnas) {
        if (filas <= 0 || columnas <= 0) {
            throw new IllegalArgumentException("Las dimensiones de la sala deben ser positivas");
        }
        this.filas = filas;
        this.columnas = columnas;
        this.asientos = new Asiento[filas][columnas];
        for (int f = 0; f < filas; ++f) {
            for (int c = 0; c < columnas; ++c) {
                asientos[f][c] = new Asiento();
            }
        }
    }

    private boolean dentroDeRango(int fila, int columna) {
        return fila >= 0 && fila < filas && columna >= 0 && columna < columnas;
    }

    // Cada método solo toca UN asiento; no hay sección crítica compartida.
    public boolean asignarAsiento(int fila, int columna, Persona persona) {
        if (!dentroDeRango(fila, columna)) {
            return false;
        }
        return asientos[fila][columna].ocupar(persona);
    }

    public boolean liberarAsiento(int fila, int columna) {
        if (!dentroDeRango(fila, columna)) {
            return false;
        }
        return asientos[fila][columna].liberar();
    }

    public Optional<Persona> buscarPorAsiento(int fila, int columna) {
        if (!dentroDeRango(fila, columna)) {
            return Optional.empty();
        }
        return asientos[fila][columna].getOcupante();
    }

    // Recorrido de solo lectura: cada casilla se lee con su propio cerrojo.
    public List<Ubicacion> buscarPorNombre(String nombre) {
        List<Ubicacion> encontrados = new ArrayList<>();
        for (int f = 0; f < filas; ++f) {
            for (int c = 0; c < columnas; ++c) {
                if (asientos[f][c].perteneceA(nombre)) {
                    encontrados.add(new Ubicacion(f, c));
                }
            }
        }
        return encontrados;
    }

    public int asientosOcupados() {
        int ocupados = 0;
        for (int f = 0; f < filas; ++f) {
            for (int c = 0; c < columnas; ++c) {
                if (asientos[f][c].isOcupado()) {
                    ++ocupados;
                }
            }
        }
        return ocupados;
    }

    // filas y columnas nunca cambian: no necesitan cerrojo
    public int totalAsientos() {
        return filas * columnas;
    }

    public void mostrarSala() {
        // 1) copiar cada casilla (cada lectura ya está sincronizada en Asiento)
        boolean[][] copia = new boolean[filas][columnas];
        for (int f = 0; f < filas; ++f) {
            for (int c = 0; c < columnas; ++c) {
                copia[f][c] = asientos[f][c].isOcupado();
            }
        }

        // 2) imprimir sin cerrojos (la E/S por consola es lenta)
        System.out.print("\n     ");
        for (int c = 0; c < columnas; ++c) {
            System.out.printf("%3d", c);
        }
        System.out.println();
        for (int f = 0; f < filas; ++f) {
            System.out.print("F" + f + "  ");
            for (int c = 0; c < columnas; ++c) {
                System.out.printf("%3s", copia[f][c] ? "[X]" : "[ ]");
            }
            System.out.println();
        }
        System.out.println();
    }
}
