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

#define ENTRY_BIKES_SMP 0
#define ENTRY_BAGGAGE_SMP 1

int train_id;
int trains_shm_id;
int station_shm_id;
int train_que_id;
int communication_que_id;
train *conductor_train;
station *station_shm;
train *trains_shm;

train* find_train_by_id(int count, int id) {
    for (int i = 0; i < count; i++) {
        if (trains_shm[i].train_id == id) {
            return &trains_shm[i]; // Zwraca wskaźnik do znalezionej struktury
        }
    }
    return NULL; // Jeśli nie znaleziono
}

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

void send_message_to_station(int signal) {
    communication msg;
    msg.mtype = 1;
    msg.sender = getpid();
    msg.signal = signal;
    if (msgsnd(communication_que_id, &msg, sizeof(msg.sender) + sizeof(msg.signal), 0) == -1) {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }
    if (msg.signal == 0) {
        printf("Sygnal od konduktora (PID: %d) wyslany do zawiadowcy, wiadomosc zwrotna\n", msg.sender);
    }
    if (msg.signal == 1) {
        printf("Sygnal od konduktora (PID: %d) wyslany do zawiadowcy, odjazd ze stacji\n", msg.sender);
    }

}

void rcv_message_from_station() {
	communication msg;
    if (msgrcv(communication_que_id, &msg, sizeof(msg.sender) + sizeof(msg.signal), getpid(), 0) == -1) {
            perror("msgrcv failed_kierownik");
            exit(EXIT_FAILURE);
    }
    if (msg.signal == 1) {
    	printf("Kierownik (PID: %d) otrzymal wiadomosc do Zawiadowcy stacji, komunikat: ODJAZD\n", getpid());
    }
    if (msg.signal == 0) {
      	printf("Kierownik (PID: %d) otrzymal wiadomosc do Zawiadowcy stacji, komunikat: PODJAZD\n", getpid());
    }
}




void IPC_setup() {
	station_shm_id = get_station_shm();
    station_shm = attach_station(station_shm_id);
    trains_shm_id = get_trains_shm();
    trains_shm = attach_trains(trains_shm_id);
    train_que_id = get_trains_que();
    communication_que_id = get_communication_que();
}

int main(int argc, char* argv[]) {
	IPC_setup();
    train_id = atoi(argv[1]);
    conductor_train = find_train_by_id(TRAIN_AMOUNT, train_id);
    printf("Stworzono proces kierownika pociagu o PID = %d, przypisany do pociagu nr %d\n", getpid(), train_id);

    pthread_t check_seats_thread;

    while (1) {
        rcv_message_from_station();//Kierownik czeka na komunikat aby mogl podjechac
        send_message_to_station(2);//Kierownik wysyla wiadomosc zwrotna ze otrzymal komunikat
        sleep(STOP_TIME);//Kierownik czeka okreslony czas przed planowanym odjazdem
		send_message_to_station(1);//Kierownik wysyla wiadomosc ze pociag odjedzie ze stacji

        sleep(RETURN_TIME);
        add_train_to_que();
    }

    exit(0);
}