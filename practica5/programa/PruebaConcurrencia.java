import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Pruebas para verificar la sincronización de la práctica 5.
 * Ejecutar: javac -d out ./*.java && java -cp out PruebaConcurrencia
 */
public class PruebaConcurrencia {

    public static void main(String[] args) throws InterruptedException {
        mismoAsiento();
        asientosDistintos();
    }

    // 200 hilos pelean por el mismo asiento: exactamente uno debe ganar.
    private static void mismoAsiento() throws InterruptedException {
        for (int ronda = 0; ronda < 50; ++ronda) {
            Sala sala = new Sala(1, 1);
            AtomicInteger ganadores = new AtomicInteger();
            CountDownLatch inicio = new CountDownLatch(1);
            Thread[] hilos = new Thread[200];

            for (int i = 0; i < hilos.length; ++i) {
                final int k = i;
                hilos[i] = new Thread(() -> {
                    try {
                        inicio.await();
                    } catch (InterruptedException e) {
                        return;
                    }
                    if (sala.asignarAsiento(0, 0, new Persona("P" + k, 20))) {
                        ganadores.incrementAndGet();
                    }
                });
                hilos[i].start();
            }
            inicio.countDown();
            for (Thread hilo : hilos) {
                hilo.join();
            }

            if (ganadores.get() != 1
                    || !sala.liberarAsiento(0, 0)
                    || sala.liberarAsiento(0, 0)) {
                throw new AssertionError("Ronda " + ronda + ": ganadores = " + ganadores.get());
            }
        }
        System.out.println("OK 1: 50 rondas x 200 hilos en el MISMO asiento, un solo ganador");
    }

    // 80 hilos reservan 80 asientos distintos: todos deben lograrlo.
    private static void asientosDistintos() throws InterruptedException {
        Sala sala = new Sala(8, 10);
        AtomicInteger exitosos = new AtomicInteger();
        CountDownLatch inicio = new CountDownLatch(1);
        Thread[] hilos = new Thread[80];

        for (int f = 0; f < 8; ++f) {
            for (int c = 0; c < 10; ++c) {
                final int fila = f, columna = c;
                hilos[f * 10 + c] = new Thread(() -> {
                    try {
                        inicio.await();
                    } catch (InterruptedException e) {
                        return;
                    }
                    if (sala.asignarAsiento(fila, columna,
                            new Persona("P" + fila + "_" + columna, 20))) {
                        exitosos.incrementAndGet();
                    }
                });
                hilos[f * 10 + c].start();
            }
        }
        inicio.countDown();
        for (Thread hilo : hilos) {
            hilo.join();
        }

        if (exitosos.get() != 80 || sala.asientosOcupados() != 80) {
            throw new AssertionError("Exitosos = " + exitosos.get()
                    + ", ocupados = " + sala.asientosOcupados());
        }
        System.out.println("OK 2: 80 hilos en asientos DISTINTOS, 80 reservas simultaneas");
    }
}
