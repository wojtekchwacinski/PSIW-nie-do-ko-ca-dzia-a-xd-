#include <stdio.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


#define N 5 // liczba głodomorów
// semafory
static struct sembuf buf;
void podnies(int semid, int semnum){
    buf.sem_num = semnum;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1){
        perror("Podnoszenie semafora");
        exit(1);
    }
}
void opusc(int semid, int semnum){
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1){
        perror("Opuszczanie semafora");
        exit(1);
    }
}

int los_czasu(){
    time_t czas;
    srand((unsigned int)time(&czas));
    return (rand()%10)+1;

}

int najchudszy(int suma[]){
    int i = 0;
    for (int j = i + 1; j < N; j++){
        if (suma[j] < suma[i] ){
            i = j;
        }
    }
    return i;
}

void godomor(int nr_glodomora, int suma[], int talerz, int paleczka, int pozwolenie){

    while(1){
        printf("Filozof o numerku %d duma i jego suma to: %d\n", nr_glodomora, suma[nr_glodomora]);
        usleep(1000*1000*los_czasu()); //duma sb
        printf("Filozof o numerku %d zglodnial i jego suma to: %d\n",nr_glodomora, suma[nr_glodomora]);
        if (nr_glodomora != najchudszy(suma)){
            opusc(pozwolenie, nr_glodomora);
        }
        opusc(talerz, 0); //zabiera talerzyk
        opusc(paleczka, nr_glodomora);
        opusc(paleczka, (nr_glodomora + N + 1)%N);
        printf("Filozof o numerku %d je i jego suma to: %d\n", nr_glodomora, suma[nr_glodomora]);
        suma[nr_glodomora] = suma[nr_glodomora] + 1;
        podnies(pozwolenie, najchudszy(suma));
        usleep(1000*1000*los_czasu()); //je sb
        podnies(paleczka, nr_glodomora);
        podnies(paleczka, (nr_glodomora + N + 1)%N);
        podnies(talerz, 0); //oddaje talerzyk
    }


}




int main(){
    // w pamiei współdzielonej będzie przechowywana ile zjadł każdy filozof
    int shmid;
    int *buf;

    //semafory których bedziemy potrzebować
    int paleczka; // potrzeba dwie pałeczki żeby filozfo mógł zjeść potrzeba ich N
    int pozwolenie; // będzie pilnował priorytetu każdego z filozofóów potrzeba ich N
    int talerz;  // jeden talerz nie wystarcza potrzeba więcej



    shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT|0600);
    if (shmid == -1){
        perror("Utworzenie segmentu pamieci wspoldzielonej");
        exit(1);
    }
    buf = (int*)shmat(shmid, NULL, 0);
    if (buf == NULL){
        perror("Przyłączenie segmentu pamieci wspoldzielonej");
        exit(1);
    }

    //semnafory
    paleczka = semget(IPC_PRIVATE, N, IPC_CREAT|0600);
    if (paleczka == -1){
        perror("Semafor pałeczkowy");
        exit(1);
    }
    pozwolenie = semget(IPC_PRIVATE, N, IPC_CREAT|0600);
    if (pozwolenie == -1){
        perror("semafor pozwoleniowy");
        exit(1);
    }
    talerz = semget(IPC_PRIVATE, 1, IPC_CREAT|0600);
    if (talerz == -1){
        perror("Semafor talerzowy");
        exit(1);
    }


    //przypisanie wartości do pamieci wspołdzielonej
    for (int i = 0; i < N; i++){
        buf[i] = 0;
    }

    //przypisanie wartości do semaforów
    if (semctl(talerz, 0, SETVAL, (int)4) == -1){
        perror("Nadawanie wartości dla semafora talerzowego");
        exit(1);
    }
    for (int i = 0; i < N; i++){
            if (semctl(paleczka, i, SETVAL, (int)1) == -1){
                perror("Nadawanie wartości dla semafora paleczkowego");
                exit(1);
            }
            if (semctl(pozwolenie, i, SETVAL, (int)0) == -1){
                perror("Nadawanie wartości dla semafora pozwoleniowego");
                exit(1);
            }
    }


    //stworzebnie N procesów
    for (int i = 0; i < N; i++){
        pid_t pid = fork();
        if (pid == 0){
            godomor(i, buf,talerz, paleczka, pozwolenie);
            exit(0);
        }
    }


    return 0;
}