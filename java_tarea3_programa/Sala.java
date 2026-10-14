import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.locks.ReentrantLock;

public class Sala {
    private final int filas;
    private final int columnas;
    private final List<List<Asiento>> asientos;
    private final ReentrantLock mtx = new ReentrantLock();

    public Sala(int filas, int columnas) {
        if (filas <= 0 || columnas <= 0) {
            throw new IllegalArgumentException("Las dimensiones de la sala deben ser positivas");
        }
        this.filas = filas;
        this.columnas = columnas;
        this.asientos = new ArrayList<>();
        for (int f = 0; f < filas; ++f) {
            List<Asiento> fila = new ArrayList<>();
            for (int c = 0; c < columnas; ++c) {
                fila.add(new Asiento(f * columnas + c + 1));
            }
            asientos.add(fila);
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
            return asientos.get(fila).get(columna).ocupar(persona);
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
            return asientos.get(fila).get(columna).liberar();
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
            return asientos.get(fila).get(columna).getOcupante();
        } finally {
            mtx.unlock();
        }
    }

    public record Posicion(int fila, int columna) { }

    public List<Posicion> buscarPorNombre(String nombre) {
        mtx.lock();
        try {
            List<Posicion> encontrados = new ArrayList<>();
            for (int f = 0; f < filas; ++f) {
                for (int c = 0; c < columnas; ++c) {
                    if (asientos.get(f).get(c).perteneceA(nombre)) {
                        encontrados.add(new Posicion(f, c));
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
                    if (asientos.get(f).get(c).isOcupado()) {
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
            System.out.print("\n");
            for (int f = 0; f < filas; ++f) {
                System.out.printf("F%d  ", f);
                for (int c = 0; c < columnas; ++c) {
                    System.out.printf("%3s", asientos.get(f).get(c).isOcupado() ? "[X]" : "[ ]");
                }
                System.out.print("\n");
            }
            System.out.print("\n");
        } finally {
            mtx.unlock();
        }
    }
}
