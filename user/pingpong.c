#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Error: invalid number of arguments\n");
        exit(1);
    }

    int semaphore = sem_open(0,1); // Abro semaforo con id 0(ping) y valor inicial 1
    if (semaphore == 0){
        printf("Error opening semaphore");
        exit(1);
    }
    semaphore = sem_open(1,0); // Abro semaforo con id 1(pong) y valor inicial 0, para evitar que el proceso padre imprima antes que el hijo
    if (semaphore == 0){
        printf("Error opening semaphore");
        exit(1);
    }

    int pid;
    int rally = atoi(argv[1]); // Para pasar de string a int el argumento de cantidad de veces que se va a ejecutar el pingpong

    if((pid = fork()) < 0){
        printf("Error creating fork");
    }
    else if(pid == 0){ // proceso hijo
        for(unsigned int i = 0; i < rally; i++){
            semaphore = sem_down(0); // Decremento el valor del semaforo, si es 0 bloquea pings sucesivos
            if (semaphore == 0){
                printf("Error decreasing semaphore value");
                exit(1);
            }
            printf("ping\n");
            semaphore = sem_up(1); // Incremento el valor del semaforo con id 1, para despertar semaforo pong luego de pings
            if (semaphore == 0){
                printf("Error increasing semaphore value");
                exit(1);
            }
        }
    } 
    else { // proceso padre
        for(unsigned int i = 0; i < rally; i++){
            semaphore = sem_down(1); // Decremento el valor del semaforo con id 1, si es 0 bloquea pongs sucesivos
            if (semaphore == 0){
                printf("Error decreasing semaphore value");
                exit(1);
            }
            printf("    pong\n");
            semaphore = sem_up(0); // Incremento el valor del semaforo con id 0, para despertar semaforo ping luego de pongs
            if (semaphore == 0){
                printf("Error increasing semaphore value");
                exit(1);
            }
        }

        semaphore = sem_close(0); // Cierro semaforos para evitar errores
        if (semaphore == 0){
                printf("Error closing semaphore");
                exit(1);
        }
        semaphore = sem_close(1); // Cierro semaforos para evitar errores
        if (semaphore == 0){
                printf("Error closing semaphore");
                exit(1);
        }
    }

    return 0;
}