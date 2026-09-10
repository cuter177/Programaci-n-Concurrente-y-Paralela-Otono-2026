#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE     10
#define ARCHIVO_BUFFER  "buffer.txt"
#define ARCHIVO_CANDADO "candado.txt"


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

void agregarItem(int numero){
    FILE *f = fopen(ARCHIVO_BUFFER, "a");
    if(f != NULL){
        fprintf(f, "%d\n", numero);
        fclose(f);
    }
}

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


void adquirirCandado(){
    while(1){
        FILE *f = fopen(ARCHIVO_CANDADO, "r");
        char estado = 'L';
        if(f != NULL){
            fscanf(f, " %c", &estado);
            fclose(f);
        }
        if(estado == 'U'){
            f = fopen(ARCHIVO_CANDADO, "w");
            fprintf(f, "L");
            fclose(f);
            return;
        }
    }
}

void liberarCandado(){
    FILE *f = fopen(ARCHIVO_CANDADO, "w");
    fprintf(f, "U");
    fclose(f);
}


int main(){
    setbuf(stdout, NULL);

    fclose(fopen(ARCHIVO_BUFFER, "w"));
    FILE *fi = fopen(ARCHIVO_CANDADO, "w");
    fprintf(fi, "U");
    fclose(fi);

    int pid = fork();

    if(pid == -1){
        perror("Error al crear el proceso con fork");
        exit(1);
    }

    if(pid == 0){
        int numero = 1;
        while(1){
            while(contarItems() >= BUFFER_SIZE);

            adquirirCandado();
            agregarItem(numero);
            liberarCandado();

            printf("[Productor] genero: %d\n", numero);

            numero = (numero % 10) + 1;
            usleep(200000);
        }
    }

    if(pid > 0){
        while(1){
            while(contarItems() <= 0);

            adquirirCandado();
            int item = quitarItem();
            liberarCandado();

            if(item != -1){
                printf("\t\t[Consumidor] tomo: %d\n", item);
            }

            usleep(500000);
        }
    }

    return 0;
}
