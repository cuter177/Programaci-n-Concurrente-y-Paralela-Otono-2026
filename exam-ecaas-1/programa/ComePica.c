/*
ComePica.c - Productor / Consumidor con buffers privados

Islas Gomez Luis Antonio - 202372860
Ramirez Candia Alfredo - 202375947
Perez Flores Julio Cesar - 202247445
Navarro Soto Mario Alberto - 202264602

Con fork() se crean dos procesos: el hijo se dedica a producir numeros y el
padre a consumirlos. Cada proceso tiene su PROPIO buffer en memoria (un arreglo
privado): el productor guarda ahi lo que va generando y el consumidor guarda
ahi lo que va recibiendo. La comunicacion entre ambos se hace con el archivo de
texto memoria.txt, que se emplea UNICAMENTE como el intermediario de la
comunicacion: guarda un solo elemento el tiempo justo para que el otro proceso
lo recoja, sin acumular nada. candado.txt funciona como un candado que solo un
proceso puede tener a la vez. No se usan tuberias, senales, wait, exec, semaforos, mutex, monitores,
memoria compartida real ni colas de mensajes; la sincronizacion se logra con
ciclos de espera y pequenos retardos.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE     10
#define ARCHIVO_MEMORIA "memoria.txt"
#define ARCHIVO_CANDADO "candado.txt"


/* Revisa si hay un elemento esperando en el intermediario de comunicacion.
   Devuelve 1 si hay uno y 0 si esta vacio. */
int contarItems(){
    FILE *f = fopen(ARCHIVO_MEMORIA, "r");
    if(f == NULL) return 0;

    char linea[32];
    int hay = (fgets(linea, sizeof(linea), f) != NULL);
    fclose(f);
    return hay;
}

/* Deposita un numero en el intermediario. Solo se llama cuando esta vacio, de
   modo que el archivo no almacena: guarda el dato el tiempo justo para que el
   otro proceso lo recoja. */
void agregarItem(int numero){
    FILE *f = fopen(ARCHIVO_MEMORIA, "w");
    if(f != NULL){
        fprintf(f, "%d\n", numero);
        fclose(f);
    }
}

/* Recoge el numero que el productor dejo en el intermediario y deja el archivo
   vacio, listo para el siguiente intercambio. */
int quitarItem(){
    FILE *f = fopen(ARCHIVO_MEMORIA, "r");
    if(f == NULL) 
      return -1;

    char linea[32];
    if(fgets(linea, sizeof(linea), f) == NULL){
        fclose(f);
        return -1;
    }
    fclose(f);

    int item = atoi(linea);

    f = fopen(ARCHIVO_MEMORIA, "w");
    fclose(f);

    return item;
}


/*
Toma el candado antes de entrar a la region critica. Todo el truco esta en
la llamada open(): al combinar O_CREAT con O_EXCL, el sistema operativo crea
el archivo de manera exclusiva; si el archivo ya existe, la llamada falla y
devuelve -1. Como el kernel hace la comprobacion en un solo paso, dos
procesos no pueden crear el candado al mismo tiempo. Mientras siga ocupado,
el ciclo espera un milisegundo y lo vuelve a intentar.

Las banderas que se unen con | son:
O_WRONLY, que abre el archivo solo para escritura;
O_CREAT, que crea el archivo si todavia no existe;
O_EXCL, que junto con O_CREAT hace que la creacion falle si el archivo ya
existe.
El numero 0600 son los permisos del archivo nuevo: el dueno puede leerlo y
escribirlo, y los demas no tienen ningun permiso.
*/
void adquirirCandado(){
    while(open(ARCHIVO_CANDADO, O_CREAT | O_EXCL | O_WRONLY, 0600) == -1){
        usleep(1000);
    }
}

/* Suelta el candado borrando el archivo, para que el otro proceso pueda
   tomarlo y entrar a la region critica. */
void liberarCandado(){
    unlink(ARCHIVO_CANDADO);
}


int main(){
    /* Sin buffer, cada printf sale de inmediato en pantalla. */
    setbuf(stdout, NULL);

    /* Deja el intermediario vacio y borra cualquier candado que haya quedado
       colgado de una ejecucion anterior. */
    fclose(fopen(ARCHIVO_MEMORIA, "w"));
    unlink(ARCHIVO_CANDADO);

    int pid = fork();

    if(pid == -1){
        perror("Error al crear el proceso con fork");
        exit(1);
    }

    if(pid == 0){
        int bufferPropio[BUFFER_SIZE];
        int n = 0;
        int numero = 1;

        while(1){
            if(n < BUFFER_SIZE){
                bufferPropio[n] = numero;
                n++;
                numero = (numero % 10) + 1;
            }

            /* Envia al intermediario solo si esta vacio y hay algo pendiente
               en su buffer privado. Todo el acceso va dentro de la region
               critica. */
            adquirirCandado();
            if(contarItems() == 0 && n > 0){
                int enviado = bufferPropio[0];
                agregarItem(enviado);
                for(int i = 1; i < n; i++){
                    bufferPropio[i-1] = bufferPropio[i];
                }
                n--;
                liberarCandado();
                printf("[Productor] envio: %d\n", enviado);
            } else {
                /* El intermediario sigue ocupado o no hay nada pendiente:
                   suelta el candado y espera. */
                liberarCandado();
            }
            usleep(200000);
        }
    }

    if(pid > 0){
        int bufferPropio[BUFFER_SIZE];
        int n = 0;

        while(1){
            adquirirCandado();
            if(contarItems() > 0 && n < BUFFER_SIZE){
                bufferPropio[n] = quitarItem();
                n++;
            }
            liberarCandado();

            /* Consume de su buffer privado, que es independiente del
               intermediario. */
            if(n > 0){
                int item = bufferPropio[0];
                for(int i = 1; i < n; i++){
                    bufferPropio[i-1] = bufferPropio[i];
                }
                n--;
                printf("\t\t[Consumidor] tomo: %d\n", item);
            }
            usleep(500000);
        }
    }

    return 0;
}
