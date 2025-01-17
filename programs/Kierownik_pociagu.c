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
#define ENTRY_SUITCASE_SMP 1;

int semaphor;

void handle_sigusr1(int sig) {
    printf("Kierownik_pociągu otrzymał sygnał: SIGUSR1\n");
}

int main() {
    semaphor = entries_semaphors();
    printf("Stworzono proces kierownika pociągu o PID = %d\n",getpid());
    signal(SIGUSR1, handle_sigusr1);
    pause();
    printf("Pociag podejezdza na stacje\n");
    exit(0);
}