#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "Funkcje.h"

int sem_psg_num_id; //semafor określa aktualną liczbe pasażerów na peronie


typedef struct{
    int place; // 1 - peron, 0 - pociag
    int type; // 1 pasazer z bagazem, 0 - pasazer z rowerem
} passenger;

int random_type() {
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
    int random_number = rand() % 100;
    return (random_number < 75) ? 1 : 0;
}
int main() {
    passenger p;
    p.place = 1;
    p.type = random_type();
    sem_psg_num_id = passenger_num_semaphor();
    semaphor_up(sem_psg_num_id, 0);
    printf("Pasazer o numerze PID = %d wszedl na peron\n", getpid());
    sleep(4);
    while(1) {
      pause();
    }
    semaphor_down(sem_psg_num_id, 0);
}