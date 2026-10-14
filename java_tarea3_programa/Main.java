import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

public class Main {
    private static void reservar(Sala sala, int fila, int columna, String nombre, int edad) {
        Persona persona = new Persona(nombre, edad);
        boolean ok = sala.asignarAsiento(fila, columna, persona);
        System.out.print((ok ? "[OK] " : "[X]  ") + nombre
                + " -> asiento (" + fila + "," + columna + ")"
                + (ok ? " reservado\n" : " no disponible\n"));
    }

    public static void main(String[] args) throws InterruptedException {
        Sala sala = new Sala(8, 10);

        System.out.print(" Reservas de asiento \n");
        List<Thread> hilos = new ArrayList<>();
        hilos.add(new Thread(() -> reservar(sala, 0, 0, "Ana", 20)));
        hilos.add(new Thread(() -> reservar(sala, 0, 0, "Luis", 23)));
        hilos.add(new Thread(() -> reservar(sala, 0, 0, "Maria", 22)));
        hilos.add(new Thread(() -> reservar(sala, 2, 4, "Pedro", 21)));
        hilos.add(new Thread(() -> reservar(sala, 3, 7, "Sofia", 19)));
        for (Thread hilo : hilos) {
            hilo.start();
        }
        for (Thread hilo : hilos) {
            hilo.join();
        }

        System.out.print("\n Liberar asiento (2,4) \n");
        System.out.print((sala.liberarAsiento(2, 4) ? "[OK] Asiento liberado\n"
                : "[X]  No se pudo liberar\n"));

        System.out.print("\n Buscar por asiento (0,0) \n");
        Optional<Persona> persona = sala.buscarPorAsiento(0, 0);
        if (persona.isPresent()) {
            System.out.print("Asiento (0,0): " + persona.get().getNombre()
                    + ", " + persona.get().getEdad() + " anios\n");
        } else {
            System.out.print("Asiento (0,0) libre\n");
        }

        System.out.print("\n=== Buscar por nombre 'Sofia' ===\n");
        List<Sala.Posicion> ubicaciones = sala.buscarPorNombre("Sofia");
        if (ubicaciones.isEmpty()) {
            System.out.print("Sofia no esta en la sala\n");
        }
        for (Sala.Posicion ubicacion : ubicaciones) {
            System.out.print("Sofia esta en (" + ubicacion.fila() + "," + ubicacion.columna() + ")\n");
        }

        System.out.print("\n=== Estado de la sala ("
                + sala.asientosOcupados() + "/80) ===\n");
        sala.mostrarSala();
    }
}
