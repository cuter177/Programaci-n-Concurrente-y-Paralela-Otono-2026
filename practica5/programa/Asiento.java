import java.util.Optional;

public class Asiento {
    private Optional<Persona> ocupante;

    public Asiento() {
        this.ocupante = Optional.empty();
    }

    public boolean isOcupado() {
        return ocupante.isPresent();
    }

    public boolean ocupar(Persona persona) {
        if (ocupante.isPresent()) {
            return false;
        }
        ocupante = Optional.of(persona);
        return true;
    }

    public boolean liberar() {
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
