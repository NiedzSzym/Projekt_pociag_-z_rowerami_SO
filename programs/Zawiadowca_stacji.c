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
#include <pthread.h>
#include "Funkcje.h"

int train_que_id;
int trains_shm_id;
int station_shm_id;
int train_id_on_station;
int communication_que_id;

train *trains_shm;
station *station_shm;
void cleanup(int signum) {
    if ( msgctl(train_que_id,IPC_RMID,0)==-1 )
    {
        perror("train_que");
        exit(EXIT_FAILURE);
    }
    if ( msgctl(communication_que_id,IPC_RMID,0)==-1 )
    {
        perror("communication_que");
        exit(EXIT_FAILURE);
    }
	detach_trains_shm(trains_shm_id, trains_shm);
    detach_station_shm(station_shm_id, station_shm);

    exit(0);
}

void rcv_message_from_conductor() {
    communication msg;
    if (msgrcv(communication_que_id, &msg, sizeof(msg.sender), 1, 0) == -1) {
        perror("msgrcv failed");
        exit(EXIT_FAILURE);
    }
    printf("Zawiadowca stacji otrzymal wiadomosc o odjezdzie od konduktora o numerze PID: %d\n",msg.sender);
}


void send_message_to_conductor(int pid_conductor) {
    communication msg;
    msg.mtype = pid_conductor;
    msg.sender = getpid();
    printf("Wysyłam wiadomość do kierownika pociągu o PID: %d\n", msg.mtype);
    if (msgsnd(communication_que_id, &msg, sizeof(msg.sender), 0) == -1) {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }

    printf("Zawiadowca wysłał wiadomość do kierownika pociągu %d (PID: %d)\n", train_id_on_station, pid_conductor);
}

void call_train() {
    train_msg msg;

    if (msgrcv(train_que_id, &msg, sizeof(msg.train), msg.mtype, 0) == -1) {
            perror("Blad odbioru komunikatu z kolejki");
            exit(EXIT_FAILURE);
    }
    train_id_on_station = msg.train.train_id;
    send_message_to_conductor(msg.train.train_conductor_PID);
    printf("KOMUNIKAT: Na peron wjedzie pociag numer %d\n", msg.train.train_id);

}

void train_departure() {
    printf("Pociag odjezdza ze stacji\n");
    station_shm->track_status = 0;
}

void setup_trains(int number) {
    int i;
    for (i = 0; i < number; i++) {
    	pid_t pid_conductor;
		train new_train;
        new_train.train_id = i + 1;
        new_train.train_status = 0;

        char train_id_str[10];
        sprintf(train_id_str, "%d", new_train.train_id);

		pid_conductor = fork();
		if (pid_conductor == 0) {
        	execlp("./Kierownik_pociagu", "Kierownik_pociagu", train_id_str,NULL);
       		perror("execlp");
        	exit(EXIT_FAILURE);
   		}
   		else if (pid_conductor < 0) {
        	perror("fork");
        	exit(EXIT_FAILURE);
    	}

        new_train.train_conductor_PID = pid_conductor;



        trains_shm[i].train_id = new_train.train_id;
        trains_shm[i].train_status = new_train.train_status;
        trains_shm[i].train_conductor_PID = new_train.train_conductor_PID;

        train_msg msg;
        msg.mtype = 1;
        msg.train = new_train;

        if (msgsnd(train_que_id, &msg, sizeof(msg.train), 0) == -1) {
            perror("msgsnd failed");
            exit(EXIT_FAILURE);
        }
        printf("Wygenerowano pociag o id %d oraz kierownika %d\n",new_train.train_id, new_train.train_conductor_PID);

    }
}

void setup_station() {
    setup_trains(TRAIN_AMOUNT);


}



int main() {
    int sem_num;
    signal(SIGINT, cleanup);


    train_que_id = get_trains_que();
    communication_que_id = get_communication_que();

    trains_shm_id = get_trains_shm();
    station_shm_id = get_station_shm();


    trains_shm = attach_trains(trains_shm_id);
    station_shm = attach_station(station_shm_id);

    setup_station();
    sleep(1);


//    while(1) {
//        sleep(4);
//        printf("Liczba pasazerow na stacji: %d\n", station_shm->passengers_waiting);
//    }

    while(1) {
        if (station_shm->passengers_waiting > 0 && station_shm->track_status == 0) {
            call_train();
            rcv_message_from_conductor();
            train_departure();
        }
    }
//    printf("Trains initialized in shared memory:\n");
//    for (int i = 0; i < TRAIN_AMOUNT; i++) {
//        printf("Pociag %d: ID=%d, Status=%d, PID kierownika=%d\n",
//               i, trains_shm[i].train_id, trains_shm[i].train_status, trains_shm[i].train_conductor_PID);
//    }
    for (int i = 0; i < TRAIN_AMOUNT; i++) {
    	waitpid(trains_shm[i].train_conductor_PID, NULL, 0);
	}
    sleep(5);
    cleanup(0);
    sleep(1);



}


