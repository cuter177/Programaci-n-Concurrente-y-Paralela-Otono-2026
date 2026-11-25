import java.util.Optional;

// Cada asiento tiene su propio cerrojo (el monitor del objeto).
// synchronized bloquea "this": el asiento (0,0) y el (2,4) son cerrojos
// distintos, así que se pueden reservar al mismo tiempo.
// Solo se esperan los hilos que quieren el MISMO asiento.
public class Asiento {
    private Optional<Persona> ocupante = Optional.empty();

    public boolean isOcupado() {
        return ocupante.isPresent();
    }

    // Atómico: revisar si está libre y ocuparlo ocurre sin interrupciones.
    public synchronized boolean ocupar(Persona persona) {
        if (ocupante.isPresent()) {
            return false;
        }
        ocupante = Optional.of(persona);
        return true;
    }

    public synchronized boolean liberar() {
        if (ocupante.isEmpty()) {
            return false;
        }
        ocupante = Optional.empty();
        return true;
    }

    public boolean perteneceA(String nombre) {
        return ocupante.isPresent() && ocupante.get().getNombre().equals(nombre);
    }

    public Optional<Persona> getOcupante() {
        return ocupante;
    }
}
