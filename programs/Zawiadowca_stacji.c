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

#define FILE_ID 1;
#define TRAINS_AMOUNT 5;

int sem_psg_num_id; //semafor określa aktualną liczbe pasażerów na peronie
int track_status; //1 - pusty, 0 - zajety
int train_que_id;

void cleanup(int signum) {
    semctl(sem_psg_num_id, 0, IPC_RMID);
    msgctl(train_que_id, IPC_RMID, NULL);
    if ( msgctl(train_que_id,IPC_RMID,0)==-1 )
      		{
        	perror("train_que");
        	exit(EXIT_FAILURE);
      		}

    exit(0);
}

void call_train() {
    train_msg msg;

    if (msgrcv(train_que_id, &msg, sizeof(msg.train), msg.mtype, 0) == -1) {
            perror("Blad odbioru komunikatu z kolejki");
            exit(EXIT_FAILURE);
    }
    printf("Odebrano wiadomość:\n");
    printf("  Train ID: %d\n", msg.train.train_id);
    printf("  Train Status: %d\n", msg.train.train_status);
    printf("  Train Conductor PID: %d\n", msg.train.train_conductor_PID);
    kill(msg.train.train_conductor_PID, SIGUSR1);

}

void setup_trains(int number) {
    int i;
    for (i = 0; i < number; i++) {
    	pid_t pid_conductor;
		train new_train;
		pid_conductor = fork();
		if (pid_conductor == 0) {
        	execlp("./Kierownik_pociagu", "Kierownik_pociagu", NULL);
       		perror("execlp");
        	exit(EXIT_FAILURE);
   		}
   		else if (pid_conductor < 0) {
        	perror("fork");
        	exit(EXIT_FAILURE);
    	}

        new_train.train_conductor_PID = pid_conductor;
        new_train.train_id = i + 1;
        new_train.train_status = 0;

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

void station_setup() {
    setup_trains(5);
}

int main() {
    int sem_num;
    signal(SIGINT, cleanup);
    sem_psg_num_id = passenger_num_semaphor();
    train_que_id = get_trains_que();
    setup_trains(5);
    call_train();
    sleep(20);
    cleanup(0);

//    pthread_t check_for_passengers_thread;
//    if (pthread_create(&check_for_passengers_thread, NULL, check_for_passengers, NULL) != 0) {
//        perror("pthread_create");
//        exit(EXIT_FAILURE);
//    }
}


