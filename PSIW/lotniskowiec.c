#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define N 15 // sapaces for planes on carrier
#define K 9 // a number less than n, and if there are currently fewer than K aircraft on the carrier, landing aircraft have priority
#define S 4


pthread_mutex_t mutex;
pthread_cond_t landing_cond;
pthread_cond_t takeoff_cond;

int planes_on_deck = 10;

void* landing() {
    while(1) {
        pthread_mutex_lock(&mutex);

        while (planes_on_deck>=N) {
            pthread_cond_wait(&landing_cond, &mutex);
        }
        planes_on_deck++;
        printf("Samolot laduje.\n");

        printf("Samolot wyladowal\nliczba samolotow na lotniskowcu: %d\n\n", planes_on_deck);
        pthread_mutex_unlock(&mutex);
        usleep(1000*1000);
        pthread_cond_broadcast(&takeoff_cond);
    }
    return 0;
}

void* takeoff() {
    while(1) {
        pthread_mutex_lock(&mutex);
        while (planes_on_deck < K) {
           pthread_cond_wait(&takeoff_cond, &mutex);
        }
        planes_on_deck--;
        printf("Samolot startuje.\n");

        printf("Samolot wystartowal\nliczba samolotow na lotniskowcu: %d\n\n", planes_on_deck);

        pthread_mutex_unlock(&mutex);
        usleep(1000*1000);
        pthread_cond_broadcast(&landing_cond);
    }
    return 0;
}

int main() {
    printf("\nliczba samolotow na lotniskowcu: %d\n", planes_on_deck);
    pthread_t landing_threads[S];
    pthread_t takeoff_threads[S];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&landing_cond, NULL);
    pthread_cond_init(&takeoff_cond, NULL);

    for (int i = 0; i < S; i++){
        if (pthread_create(&landing_threads[i], NULL, &landing, NULL) != 0){
            perror("Nie udału się utworzyc wątku ladowanie");
        }
        if (pthread_create(&takeoff_threads[i], NULL, &takeoff, NULL) != 0){
            perror("Nie udało sie utworzyc watku startowanie");
        }
    }


    for (int i = 0; i < S; i++){
        if (pthread_join(takeoff_threads[i], NULL) != 0){
            return 2;
        }
        if (pthread_join(landing_threads[i], NULL) != 0){
            return 2;
        }
    }




    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&landing_cond);
    pthread_cond_destroy(&takeoff_cond);

    return 0;
}