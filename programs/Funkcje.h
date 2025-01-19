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

#define TRAIN_AMOUNT 3 //liczba pociagow na stacji
#define BIKE_SEATS 10 //liczba miejsc dla pasazerow z rowerem
#define BAGGAGE_SEATS 10 //liczba miejsc dla pasazerow z bagazem
#define STOP_TIME 10 //czas postoju w sekundach
#define RETURN_TIME 10 //czas powrotu pociagu na stacje w sekundach
#define SMP_STATION_ID 11
#define SMP_TRAIN_ENTRIES_ID 12
#define SMP_TRACK_STATUS_ID 13
#define MSG_QUE_TRAINS_ID 21
#define MSG_QUE_COMMUNICATION_ID 22
#define SHM_TRAINS_ID 31
#define SHM_STATION_ID 32



typedef struct {
    int train_id;
    int train_status; // 1 - na peronie, 0 - czeka na stacji, 2 - w drodze
    int seats_left;
    int bike_space_left;
    int normal_entry; // 1 - otwarte, 0 - zamkniete
    int bike_entry; // 1 - otwarte, 0 - zamkniete
    pid_t train_conductor_PID;
} train;

typedef struct {
	long mtype;
    train train;
} train_msg;

typedef struct {
	long mtype;
    int track_status; // 1 - zajety, 0 - wolny
    int passengers_waiting;
    train current_train;
} station;

typedef struct {
    long mtype;
    long sender;
    int signal; //1 - odjazd, 0 - podjazd, 2 - otrzymano wiadomosc
} communication;

void remove_semaphore(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("semctl IPC_RMID failed");
        exit(EXIT_FAILURE);
    }
}

int station_semaphore(int initial_value) {
   key_t key = ftok("/tmp", SMP_STATION_ID);
	if (key == -1) {
	    perror("ftok");
		exit(EXIT_FAILURE);
  	}
    int sem_id = semget(key, 1, IPC_CREAT | 0622);
  	if (sem_id == -1) {
	    perror("semget_train_entries");
    	exit(EXIT_FAILURE);
  	}
    if (semctl(sem_id, 0, SETVAL, initial_value) == -1) {
        perror("semctl SETVAL failed");
        exit(EXIT_FAILURE);
    }
    return sem_id;

}



int entries_semaphores(int initial_value) {
	key_t key = ftok("/tmp", SMP_TRAIN_ENTRIES_ID);
	if (key == -1) {
	    perror("ftok");
		exit(EXIT_FAILURE);
  	}

	int sem_id = semget(key, 2, IPC_CREAT | 0622);
  	if (sem_id == -1) {
	    perror("semget_train_entries");
    	exit(EXIT_FAILURE);
  	}
    if (semctl(sem_id, 0, SETVAL, initial_value) == -1) {
        perror("semctl SETVAL failed");
        exit(EXIT_FAILURE);
    }
    if (semctl(sem_id, 1, SETVAL, initial_value) == -1) {
        perror("semctl SETVAL failed");
        exit(EXIT_FAILURE);
    }
  	return sem_id;
}

int track_status_semaphor() {
	key_t key = ftok("/tmp", SMP_TRACK_STATUS_ID);
	if (key == -1) {
	    perror("ftok");
		exit(EXIT_FAILURE);
  	}

	int sem_id = semget(key, 1, IPC_CREAT | 0622);
  	if (sem_id == -1) {
	    perror("semget_train_entries");
    	exit(EXIT_FAILURE);
  	}

  	return sem_id;
}

void semaphor_lock(int sem_id, int sem_num) {
  	struct sembuf buffer = {sem_num, -1, 0};
  	if (semop(sem_id, &buffer, 1) == -1) {
	    perror("semop");
	    exit(EXIT_FAILURE);
  	}
}

void semaphor_unlock(int sem_id, int sem_num) {
  	struct sembuf buffer = {sem_num, 1, 0};
  	if (semop(sem_id, &buffer, 1) == -1) {
	    perror("semop");
	    exit(EXIT_FAILURE);
  	}
}

int get_trains_que() {
	key_t key = ftok("/tmp", MSG_QUE_TRAINS_ID);
	if (key == -1) {
		perror("ftok");
		exit(EXIT_FAILURE);
	}
	int que_id;
    que_id = msgget(key, IPC_CREAT | 0777);
    if (que_id == -1) {
    	perror("msg_que_trains");
        exit(EXIT_FAILURE);
    }
    return que_id;
}

int get_communication_que() {
	key_t key = ftok("/tmp", MSG_QUE_COMMUNICATION_ID);
	if (key == -1) {
		perror("ftok");
		exit(EXIT_FAILURE);
	}
	int que_id;
    que_id = msgget(key, IPC_CREAT | 0777);
    if (que_id == -1) {
    	perror("msg_que_communication");
        exit(EXIT_FAILURE);
    }
    return que_id;
}

int get_trains_shm() {
	key_t key = ftok("/tmp", SHM_TRAINS_ID);
	if (key == -1) {
		perror("ftok");
		exit(EXIT_FAILURE);
	}
    int shm_id;
	shm_id = shmget(key,TRAIN_AMOUNT * sizeof(train), 0622|IPC_CREAT);
	if (shm_id == -1) {
		perror("Shared memory");
		exit(EXIT_FAILURE);
	}
    return shm_id;
}

train * attach_trains(int shm_id) {
	train * train = shmat(shm_id, NULL, 0);
    if (train ==  -1) {
		perror("attach_train");
        exit(EXIT_FAILURE);
    }
    return train;

}


void detach_trains_shm(int shm_id, train * trains_shm) {
	if (shmdt(trains_shm) == -1) {
        perror("detach_train");
        exit(EXIT_FAILURE);
	}
    if (shmctl(shm_id, IPC_RMID, 0) == -1) {
        perror("detach_shm");
        exit(EXIT_FAILURE);
    }
}

int get_station_shm() {
	key_t key = ftok("/tmp", SHM_STATION_ID);
	if (key == -1) {
		perror("ftok");
		exit(EXIT_FAILURE);
	}
    int shm_id;
	shm_id = shmget(key,sizeof(station), 0622|IPC_CREAT);
	if (shm_id == -1) {
		perror("Shared memory");
		exit(EXIT_FAILURE);
	}
    return shm_id;
}

station * attach_station(int shm_id) {
	station * station =  shmat(shm_id, NULL, 0);
    if (station ==  -1) {
		perror("attach_train");
        exit(EXIT_FAILURE);
    }
    return station;

}

void detach_station_shm(int shm_id, station * station_shm) {
	if (shmdt(station_shm) == -1) {
        perror("detach_train");
        exit(EXIT_FAILURE);
	}
    if (shmctl(shm_id, IPC_RMID, 0) == -1) {
        perror("detach_shm");
        exit(EXIT_FAILURE);
    }
}

#endif //FUNKCJE_H
