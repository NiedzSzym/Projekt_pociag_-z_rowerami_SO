#ifndef FUNKCJE_H
#define FUNKCJE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#define TRAIN_AMOUNT 10; //liczba pociagow na stacji
#define BIKE_SEATS 10; //liczba miejsc dla pasazerow z rowerem
#define BAGGAGE_SEATS 10; //liczba miejsc dla pasazerow z bagazem
#define STOP_TIME 10; //czas postoju w sekundach
#define RETURN_TIME 10; //czas powrotu pociagu na stacje w sekundach


typedef struct {
  int train_id;
  int train_supervisor_id;
} train;

#endif //FUNKCJE_H
