/*
 * Practica 5 - Reserva de asientos de cine (concurrencia con hilos)
 *
 * Integrantes del equipo:
 *   Islas Gomez Luis Antonio      - 202372860
 *   Ramirez Candia Alfredo       - 202375947
 *   Perez Flores Julio Cesar     - 202247445
 *   Navarro Soto Mario Alberto   - 202264602
 */

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

public class Main {

    private static void reservar(Sala sala, int fila, int columna, String nombre, int edad) {
        Persona persona = new Persona(nombre, edad);
        boolean ok = sala.asignarAsiento(fila, columna, persona);
        System.out.println((ok ? "[OK] " : "[X]  ") + nombre
                + " -> asiento (" + fila + "," + columna + ")"
                + (ok ? " reservado" : " no disponible"));
    }

    public static void main(String[] args) throws InterruptedException {
        Sala sala = new Sala(8, 10);

        System.out.println(" Reservas de asiento ");
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

        System.out.println("\n Liberar asiento (2,4) ");
        System.out.println(sala.liberarAsiento(2, 4)
                ? "[OK] Asiento liberado"
                : "[X]  No se pudo liberar");

        System.out.println("\n Buscar por asiento (0,0) ");
        Optional<Persona> persona = sala.buscarPorAsiento(0, 0);
        if (persona.isPresent()) {
            System.out.println("Asiento (0,0): " + persona.get().getNombre()
                    + ", " + persona.get().getEdad() + " anios");
        } else {
            System.out.println("Asiento (0,0) libre");
        }

        System.out.println("\n=== Buscar por nombre 'Sofia' ===");
        List<Ubicacion> ubicaciones = sala.buscarPorNombre("Sofia");
        if (ubicaciones.isEmpty()) {
            System.out.println("Sofia no esta en la sala");
        }
        for (Ubicacion ubicacion : ubicaciones) {
            System.out.println("Sofia esta en (" + ubicacion.fila() + "," + ubicacion.columna() + ")");
        }

        System.out.println("\n=== Estado de la sala ("
                + sala.asientosOcupados() + "/80) ===");
        sala.mostrarSala();
    }
}
