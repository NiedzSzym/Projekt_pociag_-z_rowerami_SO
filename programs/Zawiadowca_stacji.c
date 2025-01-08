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

#define MAX_AMOUNT_OF_PASSENGERS 15 //maksymalna liczba pasazerow w jednej fali
#define MIN_AMOUNT_OF_PASSENGERS 1 //minimalna liczba pasazerow w jednej fali

void generate_passenger() {
  pid_t pid = fork();
     if (pid < 0) {
        perror("Błąd podczas tworzenia procesu:");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Proces dziecka
        printf("Pasazer o pid: %d\n", getpid());
        execlp("./pasazer", "pasazer", NULL);
        perror("execlp");
        exit(EXIT_FAILURE);

    }
}

int main() {
  srand(time(NULL));
  int delay = rand() % 10;
  while (1) {
    sleep(delay);
    int wave;
    wave = rand() % (MAX_AMOUNT_OF_PASSENGERS - MIN_AMOUNT_OF_PASSENGERS + 1) + MIN_AMOUNT_OF_PASSENGERS;
    printf("%d\n", wave);
    for (int i = 0; i < wave; i++) {
      generate_passenger();
    }
  }
}


