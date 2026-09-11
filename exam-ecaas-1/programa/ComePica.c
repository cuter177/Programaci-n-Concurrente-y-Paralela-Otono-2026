/*
ComePica.c - Productor / Consumidor

Islas Gomez Luis Antonio - 202372860
Ramirez Candia Alfredo - 202375947
Perez Flores Julio Cesar - 202247445
Navarro Soto Mario Alberto - 202264602

Con fork() se crean dos procesos: el hijo se dedica a producir numeros y el
padre a consumirlos. Lo que comparten son dos archivos de texto: buffer.txt
guarda los elementos y candado.txt funciona como un candado que solo un
proceso puede tener a la vez. No se usan tuberias, senales, wait, exec,
semaforos, mutex, monitores, memoria compartida ni colas de mensajes; la
sincronizacion se logra con ciclos de espera y pequenos retardos.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE     10
#define ARCHIVO_BUFFER  "buffer.txt"
#define ARCHIVO_CANDADO "candado.txt"


/* Lee buffer.txt y devuelve cuantas lineas tiene, o sea, cuantos elementos
   hay guardados en este momento. */
int contarItems(){
    FILE *f = fopen(ARCHIVO_BUFFER, "r");
    if(f == NULL) return 0;

    int contador = 0;
    char linea[32];
    while(fgets(linea, sizeof(linea), f)){
        contador++;
    }
    fclose(f);
    return contador;
}

/* Agrega un numero al final de buffer.txt. Se abre en modo "a" (append)
   justamente para no borrar lo que ya estaba guardado. */
void agregarItem(int numero){
    FILE *f = fopen(ARCHIVO_BUFFER, "a");
    if(f != NULL){
        fprintf(f, "%d\n", numero);
        fclose(f);
    }
}

/* Extrae el primer numero de buffer.txt, que es el mas antiguo: por eso el
   orden de salida es FIFO. Primero lee todas las lineas, se queda con la
   primera y despues reescribe el archivo con las que sobraron. */
int quitarItem(){
    FILE *f = fopen(ARCHIVO_BUFFER, "r");
    if(f == NULL) return -1;

    char lineas[BUFFER_SIZE][32];
    int total = 0;
    while(total < BUFFER_SIZE && fgets(lineas[total], sizeof(lineas[total]), f)){
        total++;
    }
    fclose(f);

    if(total == 0) return -1;

    int item = atoi(lineas[0]);

    f = fopen(ARCHIVO_BUFFER, "w");
    for(int i = 1; i < total; i++){
        fputs(lineas[i], f);
    }
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

    /* Deja el buffer vacio y borra cualquier candado que haya quedado
       colgado de una ejecucion anterior. */
    fclose(fopen(ARCHIVO_BUFFER, "w"));
    unlink(ARCHIVO_CANDADO);

    /* Aqui nace el segundo proceso. El hijo recibe un 0, el padre recibe el
       PID del hijo, y un valor negativo significa que hubo un error. */
    int pid = fork();

    if(pid == -1){
        perror("Error al crear el proceso con fork");
        exit(1);
    }

    if(pid == 0){
        /* Rama del productor: este es el proceso hijo. */
        int numero = 1;
        while(1){
            adquirirCandado();
            /* Solo produce si todavia hay espacio en el buffer. */
            if(contarItems() < BUFFER_SIZE){
                agregarItem(numero);
                liberarCandado();
                printf("[Productor] genero: %d\n", numero);
                numero = (numero % 10) + 1;
            } else {
                /* El buffer esta lleno, suelta el candado y espera. */
                liberarCandado();
            }
            usleep(200000);
        }
    }

    if(pid > 0){
        /* Rama del consumidor: este es el proceso padre. */
        while(1){
            adquirirCandado();
            int item = -1;
            /* Solo consume si hay al menos un elemento guardado. */
            if(contarItems() > 0){
                item = quitarItem();
            }
            liberarCandado();
            if(item != -1){
                printf("\t\t[Consumidor] tomo: %d\n", item);
            }
            usleep(500000);
        }
    }

    return 0;
}
