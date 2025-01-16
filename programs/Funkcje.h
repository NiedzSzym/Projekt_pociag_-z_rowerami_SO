#ifndef FUNKCJE_H
#define FUNKCJE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <time.h>
#include <sys/time.h>

#define TRAIN_AMOUNT 10 //liczba pociagow na stacji
#define BIKE_SEATS 10 //liczba miejsc dla pasazerow z rowerem
#define BAGGAGE_SEATS 10 //liczba miejsc dla pasazerow z bagazem
#define STOP_TIME 10 //czas postoju w sekundach
#define RETURN_TIME 10 //czas powrotu pociagu na stacje w sekundach
#define SMP_PASSENGERS_NUMBER_ID 11
#define MSG_QUE_TRAINS 21



typedef struct {
    int train_status;
    int train_id;
    pid_t train_conductor_PID;
} train;

typedef struct {
	long mtype;
    train train;
} train_msg;

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

int get_trains_que() {
	key_t key = ftok("/tmp", SMP_PASSENGERS_NUMBER_ID);
	if (key == -1) {
		perror("ftok");
		exit(EXIT_FAILURE);
	}
	int que_id;
    que_id = msgget(key, IPC_CREAT | 0777);
    if (que_id == -1) {
    	perror("msg_que");
        exit(EXIT_FAILURE);
    }
    return que_id;
}



//int get_shared_memory() {
//	key_t key = ftok("/tmp", SHARED_MEMORY_TRACK_STATUS_ID);
//	if (key == -1) {
//		perror("ftok");
//		exit(EXIT_FAILURE);
//	}
//    int shm_id;
//	shm_id = shmget(key, sizeof(train_status), 0622|IPC_CREAT);
//	if (shm_id == -1) {
//		perror("Shared memory");
//		exit(EXIT_FAILURE);
//	}
//    return shm_id;
//}



#endif //FUNKCJE_H
