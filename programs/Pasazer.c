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

#define BIKE_ENTRY 1
#define NORMAL_ENTRY 0

typedef struct{
    int place; // 1 - peron, 0 - pociag
    int bike; // 1 pasazer z rowerem, 0 - pasazer bez rowera
} passenger;

station *station_shm;
int station_shm_id;
int station_smp_id;
int entries_smp_id;

int random_type() {
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
    int random_number = rand() % 100;
    return (random_number < 25) ? 1 : 0;
}
int main() {
    passenger p;
    p.place = 1;
    p.bike = random_type();//25% szansy na to ze pasazer bedzie mial rower

    station_shm_id = get_station_shm();
    station_shm = attach_station(station_shm_id);
    station_smp_id = station_semaphore(1);
    entries_smp_id = entries_semaphores(1);

    //inkrementowanie liczby pasazerow na stacji
    semaphor_lock(station_smp_id, 0);
    station_shm->passengers_waiting++;
    semaphor_unlock(station_smp_id, 0);
    //printf("%d\n", station_shm->passengers_waiting);




    while(1) {
      sleep(1);
    }
    station_shm->passengers_waiting--;

}