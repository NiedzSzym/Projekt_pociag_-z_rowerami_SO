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
#include "Funkcje.h"

int sem_psg_num_id; //semafor określa aktualną liczbe pasażerów na peronie

struct passenger {
    int place; // 1 - peron, 0 - pociag

};



int main() {
    struct passenger p;
    p.place = 1;
    sem_psg_num_id = passenger_num_semaphor();
    semaphor_up(sem_psg_num_id, 0);
    printf("Pasazer o numerze PID = %d wszedl na peron\n", getpid());
    sleep(4);
    while(1) {
      pause();
    }
    semaphor_down(sem_psg_num_id, 0);
}