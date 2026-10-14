import java.util.Optional;

public class Asiento {
    private final int numero;
    private Persona ocupante;

    public Asiento(int numero) {
        this.numero = numero;
        this.ocupante = null;
    }

    public int getNumero() {
        return numero;
    }

    public boolean isOcupado() {
        return ocupante != null;
    }

    public boolean ocupar(Persona persona) {
        if (ocupante != null) {
            return false;
        }
        ocupante = persona;
        return true;
    }

    public boolean liberar() {
        if (ocupante == null) {
            return false;
        }
        ocupante = null;
        return true;
    }

    public boolean perteneceA(String nombre) {
        return ocupante != null && ocupante.getNombre().equals(nombre);
    }

    public Optional<Persona> getOcupante() {
        return Optional.ofNullable(ocupante);
    }
}
