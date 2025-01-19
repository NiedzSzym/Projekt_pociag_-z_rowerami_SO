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

#define ENTRY_BIKES_SMP 0;
#define ENTRY_BAGGAGE_SMP 1;

int train_id;
int trains_shm_id;
int station_shm_id;
int train_que_id;
int communication_que_id;
station *station_shm;
train *trains_shm;



void add_train_to_que() {
    train_msg msg;
    msg.mtype = 1;
    msg.train =  trains_shm[train_id - 1];

    if (msgsnd(train_que_id, &msg, sizeof(msg.train), 0) == -1) {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }
    printf("Pociag nr %d powrocil na stacje\n", train_id);
}

void send_message_to_station() {
    communication msg;
    msg.mtype = 1;
    msg.sender = getpid();
    if (msgsnd(communication_que_id, &msg, sizeof(msg.sender), 0) == -1) {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }
    printf("Sygnal od konduktora (PID: %d) wyslany do zawiadowcy, ODJAZD\n", msg.sender);

}

int main(int argc, char* argv[]) {
    station_shm_id = get_station_shm();
    station_shm = attach_station(station_shm_id);
    trains_shm_id = get_trains_shm();
    trains_shm = attach_trains(trains_shm_id);
    train_que_id = get_trains_que();

    train_id = atoi(argv[1]);
    communication_que_id = get_communication_que();

    communication msg;

    printf("Stworzono proces kierownika pociągu o PID = %d\n", getpid());
    while (1) {
        printf("Oczekiwanie na komunikat\n");
        if (msgrcv(communication_que_id, &msg, sizeof(msg.sender), getpid(), 0) == -1) {
            perror("msgrcv failed");
            exit(EXIT_FAILURE);
        }
        printf("Kierownik pociagu %d (PID: %d) otrzymal wiadomosc: Przyjedz na stacje\n",train_id, msg.mtype);

        sleep(STOP_TIME);

        send_message_to_station();
        printf("Pociąg o numerze %d odjeżdża ze stacji.\n", train_id);
        sleep(RETURN_TIME);
        add_train_to_que();
    }

    exit(0);
}