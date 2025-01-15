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

#define FILE_ID 1;

int sem_psg_num_id; //semafor określa aktualną liczbe pasażerów na peronie

void cleanup(int signum) {
    semctl(sem_psg_num_id, 0, IPC_RMID);
    exit(0);
}

int main() {
    int sem_num;
    signal(SIGINT, cleanup);
    sem_psg_num_id = passenger_num_semaphor();

    while(1) {
        sleep(3);
        sem_num = semctl(sem_psg_num_id, 0, GETVAL, 0);
        printf("%d\n", sem_num);
    }
}


