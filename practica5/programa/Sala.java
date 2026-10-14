import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.locks.ReentrantLock;

public class Sala {
    private final int filas;
    private final int columnas;
    private final Asiento[][] asientos;
    private final ReentrantLock mtx = new ReentrantLock();

    public Sala(int filas, int columnas) {
        if (filas <= 0 || columnas <= 0) {
            throw new IllegalArgumentException("Las dimensiones de la sala deben ser positivas");
        }
        this.filas = filas;
        this.columnas = columnas;
        this.asientos = new Asiento[filas][columnas];
        for (int f = 0; f < filas; ++f) {
            for (int c = 0; c < columnas; ++c) {
                asientos[f][c] = new Asiento(f * columnas + c + 1);
            }
        }
    }

    private boolean dentroDeRango(int fila, int columna) {
        return fila >= 0 && fila < filas && columna >= 0 && columna < columnas;
    }

    public boolean asignarAsiento(int fila, int columna, Persona persona) {
        mtx.lock();
        try {
            if (!dentroDeRango(fila, columna)) {
                return false;
            }
            return asientos[fila][columna].ocupar(persona);
        } finally {
            mtx.unlock();
        }
    }

    public boolean liberarAsiento(int fila, int columna) {
        mtx.lock();
        try {
            if (!dentroDeRango(fila, columna)) {
                return false;
            }
            return asientos[fila][columna].liberar();
        } finally {
            mtx.unlock();
        }
    }

    public Optional<Persona> buscarPorAsiento(int fila, int columna) {
        mtx.lock();
        try {
            if (!dentroDeRango(fila, columna)) {
                return Optional.empty();
            }
            return asientos[fila][columna].getOcupante();
        } finally {
            mtx.unlock();
        }
    }

    public List<Ubicacion> buscarPorNombre(String nombre) {
        mtx.lock();
        try {
            List<Ubicacion> encontrados = new ArrayList<>();
            for (int f = 0; f < filas; ++f) {
                for (int c = 0; c < columnas; ++c) {
                    if (asientos[f][c].perteneceA(nombre)) {
                        encontrados.add(new Ubicacion(f, c));
                    }
                }
            }
            return encontrados;
        } finally {
            mtx.unlock();
        }
    }

    public int asientosOcupados() {
        mtx.lock();
        try {
            int ocupados = 0;
            for (int f = 0; f < filas; ++f) {
                for (int c = 0; c < columnas; ++c) {
                    if (asientos[f][c].isOcupado()) {
                        ++ocupados;
                    }
                }
            }
            return ocupados;
        } finally {
            mtx.unlock();
        }
    }

    public void mostrarSala() {
        mtx.lock();
        try {
            System.out.print("\n     ");
            for (int c = 0; c < columnas; ++c) {
                System.out.printf("%3d", c);
            }
            System.out.println();
            for (int f = 0; f < filas; ++f) {
                System.out.print("F" + f + "  ");
                for (int c = 0; c < columnas; ++c) {
                    System.out.printf("%3s", asientos[f][c].isOcupado() ? "[X]" : "[ ]");
                }
                System.out.println();
            }
            System.out.println();
        } finally {
            mtx.unlock();
        }
    }
}
