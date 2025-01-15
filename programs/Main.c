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
        srand(time(NULL));
        int delay = rand() % 10;
        while (1) {
            sleep(delay);
            int wave;
            wave = rand() % (MAX_AMOUNT_OF_PASSENGERS - MIN_AMOUNT_OF_PASSENGERS + 1) + MIN_AMOUNT_OF_PASSENGERS;
            printf("%d\n", wave);
            for (int i = 0; i < wave; i++) {
                usleep(50000);
                generate_passenger();
            }
        }
    }

}

void cleanup(int signum) {
    pid_t pgid = getpgid(0); // Pobiera PGID bieżącego procesu
    if (pgid == -1) {
        perror("getpgid");
        return 1;
    }

    // Wysyłanie sygnału SIGTERM do całej grupy procesów
    if (kill(-pgid, SIGINT) == -1) {
        perror("kill");
        return 1;
    }
}

int main() {
    pthread_t generate_passenger_thread;
    pid_t pid_zawiadowca;
    signal(SIGINT, cleanup);

    if (pthread_create(&generate_passenger_thread, NULL, waves_generator, NULL) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    pid_zawiadowca = fork();
    if (pid_zawiadowca == 0) {
        execlp("./Zawiadowca_stacji", "Zawiadowca_stacji", NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }
    else if (pid_zawiadowca < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }





    waitpid(pid_zawiadowca, NULL, 0);
    pthread_join(generate_passenger_thread, NULL);
}