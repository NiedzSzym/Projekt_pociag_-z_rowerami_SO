#ifndef FUNKCJE_H
#define FUNKCJE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#define TRAIN_AMOUNT 10 //liczba pociagow na stacji
#define BIKE_SEATS 10 //liczba miejsc dla pasazerow z rowerem
#define BAGGAGE_SEATS 10 //liczba miejsc dla pasazerow z bagazem
#define STOP_TIME 10 //czas postoju w sekundach
#define RETURN_TIME 10 //czas powrotu pociagu na stacje w sekundach
#define SMP_PASSENGERS_NUMBER_ID 1

typedef struct {
	int train_id;
	int train_supervisor_id;
} train;

int passenger_num_semaphor() {
	key_t key = ftok("/tmp", SMP_PASSENGERS_NUMBER_ID);
	if (key == -1) {
	    perror("ftok");
		exit(EXIT_FAILURE);
  	}

	int sem_id = semget(key, 1, IPC_CREAT | 0622);
  	if (sem_id == -1) {
	    perror("semget_passenger_number");
    	exit(EXIT_FAILURE);
  	}

  	return sem_id;
}

void semaphor_down(int sem_id, int sem_num) {
  	struct sembuf buffer = {sem_num, -1, 0};
  	if (semop(sem_id, &buffer, 1) == -1) {
	    perror("semop");
	    exit(EXIT_FAILURE);
  	}
}

void semaphor_up(int sem_id, int sem_num) {
  	struct sembuf buffer = {sem_num, 1, 0};
  	if (semop(sem_id, &buffer, 1) == -1) {
	    perror("semop");
	    exit(EXIT_FAILURE);
  	}
}



#endif //FUNKCJE_H
