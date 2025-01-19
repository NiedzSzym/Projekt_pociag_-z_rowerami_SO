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
int station_shm_id;//Pamiec dzielona przechowujaca aktualne informacje o stacji
int entries_smp_id;
int read_write_smp_id;//semafory do rozwiazania problemu pisania i czytania station_shm->passengers_waiting
#define SP 0
#define W 1

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
    //entries_smp_id = entries_semaphores(1);

    read_write_smp_id = get_read_write_stationshm_smp();

    //inkrementowanie liczby pasazerow na stacji
    semaphore_lock(read_write_smp_id, SP);
    station_shm->passengers_waiting++;
    semaphore_unlock(read_write_smp_id, SP);

    //printf("%d\n", station_shm->passengers_waiting);




    while(1) {
      sleep(1);
    }
    station_shm->passengers_waiting--;

}