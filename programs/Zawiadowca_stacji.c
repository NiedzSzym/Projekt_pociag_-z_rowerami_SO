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

#define MAX_AMOUNT_OF_PASSENGERS 15 //maksymalna liczba pasazerow w jednej fali
#define MIN_AMOUNT_OF_PASSENGERS 1 //minimalna liczba pasazerow w jednej fali


int train_que_id;
int trains_shm_id;
int station_shm_id;


int communication_que_id;
int station_smp_id;

train *trains_shm;
station *station_shm;

void generate_passenger() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Błąd podczas tworzenia procesu:");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Proces dziecka
        execlp("./Pasazer", "Pasazer", NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }

}

void * waves_generator() {
    while (1) {
        int delay = rand() % 10;
        while (1) {
            sleep(delay);
            int wave;
            wave = rand() % (MAX_AMOUNT_OF_PASSENGERS - MIN_AMOUNT_OF_PASSENGERS + 1) + MIN_AMOUNT_OF_PASSENGERS;
            //printf("%d\n", wave);
            for (int i = 0; i < wave; i++) {
                usleep(50000);
                generate_passenger();
            }
        }
    }

}

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
    remove_semaphore(station_smp_id);
	detach_trains_shm(trains_shm_id, trains_shm);
    detach_station_shm(station_shm_id, station_shm);

    exit(0);
}

void rcv_message_from_conductor() {
    communication msg;
    if (msgrcv(communication_que_id, &msg, sizeof(msg.sender) + sizeof(msg.signal), 1, 0) == -1) {
        perror("msgrcv failed_zawiadowca");
        exit(EXIT_FAILURE);
    }

    if (msg.signal == 2) {
    	printf("Zawiadowca stacji otrzymal wiadomosc zwrotna od konduktora o numerze PID: %d\n",msg.sender);
    }
    else if (msg.signal == 1) {
    	printf("Zawiadowca stacji otrzymal wiadomosc o odjezdzie od konduktora o numerze PID: %d\n",msg.sender);
    }
    else {
    	printf("Nastapil blad komunikacji");
        cleanup(0);
        exit(EXIT_FAILURE);
    }
}


void send_message_to_conductor(int signal) {
    communication msg;
    msg.mtype = station_shm->current_train.train_conductor_PID;
    msg.sender = getpid();
    msg.signal = signal;
    if (msgsnd(communication_que_id, &msg, sizeof(msg.sender) + sizeof(msg.signal), 0) == -1) {
        perror("msgsnd failed_zawiadowca");
        exit(EXIT_FAILURE);
    }
	if (signal == 1) {
    	printf("Zawiadowca wysłał wiadomość do kierownika pociągu %d (PID: %d), komunikat: ODJAZD\n", station_shm->current_train.train_id, station_shm->current_train.train_conductor_PID);
    }
    if (signal == 0) {
      	printf("Zawiadowca wysłał wiadomość do kierownika pociągu %d (PID: %d), komunikat: PODJAZD\n", station_shm->current_train.train_id, station_shm->current_train.train_conductor_PID);
    }
}

void call_train() {
    train_msg msg;
    if (msgrcv(train_que_id, &msg, sizeof(msg.train), msg.mtype, 0) == -1) {
            perror("Blad odbioru komunikatu z kolejki");
            exit(EXIT_FAILURE);
    }

    station_shm->current_train = msg.train;
    printf("%d\n", station_shm->current_train.train_id);
    send_message_to_conductor(0);
    rcv_message_from_conductor();
    printf("KOMUNIKAT: Na peron wjedzie pociag numer %d\n", msg.train.train_id);
    station_shm->track_status = 1;

}

void wait_for_train_departure() {
    rcv_message_from_conductor();
    printf("KOMUNIKAT: Pociag nr %d odjechal ze stacji\n", station_shm->current_train.train_id);
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
        //printf("Wygenerowano pociag o id %d oraz kierownika %d\n",new_train.train_id, new_train.train_conductor_PID);

    }
}

void setup_station() {
    setup_trains(TRAIN_AMOUNT);


}

void IPC_setup() {
	train_que_id = get_trains_que();
    communication_que_id = get_communication_que();

    trains_shm_id = get_trains_shm();
    trains_shm = attach_trains(trains_shm_id);

    station_shm_id = get_station_shm();
    station_shm = attach_station(station_shm_id);

    station_smp_id = station_semaphore(1);
}

void SIGUSR1_handler() {
	printf("SIGUSR1\n");
}

void SIGUSR2_handler() {
	printf("SIGUSR2\n");
}

int main() {
    signal(SIGINT, cleanup);
    signal(SIGUSR1, SIGUSR1_handler);
    signal(SIGUSR2, SIGUSR2_handler);
    IPC_setup();

    pthread_t generate_passenger_thread;
    srand(time(NULL));
    if (pthread_create(&generate_passenger_thread, NULL, waves_generator, NULL) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    setup_station();


//    while(1) {
//        sleep(4);
//
//    }
    station_shm->track_status = 0;

    while(1) {
        sleep(1);
        if (station_shm->passengers_waiting > 0 && station_shm->track_status == 0) {
            call_train();
            wait_for_train_departure();

        }
    }

    pthread_join(generate_passenger_thread, NULL);
    cleanup(0);



}


