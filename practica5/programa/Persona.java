// Inmutable: una vez creada, sus datos nunca cambian.
// Así es seguro compartir una Persona entre hilos sin cerrojos adicionales.
public final class Persona {
    private final String nombre;
    private final int edad;

    public Persona(String nombre, int edad) {
        if (nombre == null || nombre.isEmpty()) {
            throw new IllegalArgumentException("El nombre no puede estar vacio");
        }
        if (edad < 0) {
            throw new IllegalArgumentException("La edad no puede ser negativa");
        }
        this.nombre = nombre;
        this.edad = edad;
    }

    public String getNombre() {
        return nombre;
    }

    public int getEdad() {
        return edad;
    }
}
