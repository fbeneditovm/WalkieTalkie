#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
/*
union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};
*/

// inicializa o valor do semáforo
int setSemValue(int wait2write1);
// remove o semáforo
void delSemValue(int wait2write1);
// operação P
int semaforoDown(int wait2write1);
//operação V
int semaforoUp(int wait2write1);


int main(){

    int pid, counter, segmento;
    char *msg, buffer[300];
    int wait2write1, wait2write2;

    pid = getpid();
    counter = 0;

    wait2write1 = semget (8752, 1, 0666 | IPC_CREAT);
    wait2write2 = semget (8753, 1, 0666 | IPC_CREAT);

    setSemValue(wait2write2);//Lembrando que essa funcao seta o semaforo em 1
    semaforoDown(wait2write2);//Seta o semaforo em 0

    //Recupera o segmento de memoria compartilhado
    segmento = shmget(8752, 500*sizeof(char) , IPC_EXCL | S_IRUSR |  S_IWUSR);
    if(segmento == -1){
        printf("Erro na alocacao da Pagina\n");
        exit(-1);
    }

    //attach ao segmento
    msg = (char*) shmat(segmento, 0, 0);

    printf("Hello, my pid is: %d \nYou can't talkie now wait for the first message:\n", pid);
    fflush(stdin); //Limpeza do buffer do teclado por precaucao

    semaforoDown(wait2write2);//Espera a primeira mensagem

    while(1){
        puts(msg);//Imprime a msg da memoria compartilhada
        printf("You: ");
        gets(buffer);//O usuario escreve

        /*Guarda na memoria compartilhada o pid do processo, um contador e a mensagem do usuario*/
        sprintf(msg, "\nPid: %d, counter: %d\nMessage:%s\n", pid, counter, buffer);
        counter++;

        /*Nao necessario pois espera entrada do usuario. Uma opcao seria enviar apenas o pid e o contador 
        entao faria sentido espera 1 segundo para evitar uma quantidade muito grande de mensagens*/
        //sleep(1);
        
        semaforoUp(wait2write1);//Libera o outro processo
        semaforoDown(wait2write2);//Espera
    }
}

int setSemValue(int wait2write){
   union semun semUnion;
   semUnion.val = 1;
   return semctl(wait2write, 0, SETVAL, semUnion);
}

void delSemValue(int wait2write){
   union semun semUnion;
   semctl(wait2write, 0, IPC_RMID, semUnion);
}

int semaforoDown(int wait2write){
   struct sembuf semB;
   semB.sem_num = 0;
   semB.sem_op = -1;
   semB.sem_flg = SEM_UNDO;
   semop(wait2write, &semB, 1);
   return 0;
}

int semaforoUp(int wait2write){
   struct sembuf semB;
   semB.sem_num = 0;
   semB.sem_op = 1;
   semB.sem_flg = SEM_UNDO;
   semop(wait2write, &semB, 1);
   return 0;
}