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

// inicializa o valor do semaforo
int setSemValue(int wait2write);
// remove o semaaforo
void delSemValue(int wait2write);
// operacao P
int semaforoDown(int wait2write);
//operacao V
int semaforoUp(int wait2write);


int main(){

    int pid, counter, segmento;
    char * msg, buffer[300];
    int wait2write1, wait2write2;

    pid = getpid();
    counter = 0;

    wait2write1 = semget (8752, 1, 0666 | IPC_CREAT);
    wait2write2 = semget (8753, 1, 0666 | IPC_CREAT);

    setSemValue(wait2write1);//Lembrando que essa funcao seta o semaforo em 1
    semaforoDown(wait2write1);//Seta o semaforo em 0

    //Cria um segmento de memoria compartilhada de 500 chars
    segmento = shmget(8752, 500*sizeof(char) , IPC_CREAT | S_IRUSR |  S_IWUSR);
    if(segmento == -1){
        printf("Erro na alocacao da Pagina\n");
        exit(-1);
    }

    //attach ao segmento
    msg = (char*) shmat(segmento, 0, 0);

    //O processo first escreve primeiro
    printf("Hello, my pid is: %d \nYou can talkie now:\n", pid);
    fflush(stdin); //Limpeza do buffer do teclado por precaucao
    printf("You: ");
    gets(buffer);
    sprintf(msg, "Pid: %d, counter: %d\nMessage:%s\n\n", pid, counter, buffer);
    counter++;
    semaforoUp(wait2write2);//Libera o outro processo
    semaforoDown(wait2write1);//Espera

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

        semaforoUp(wait2write2);//Libera o outro processo
        semaforoDown(wait2write1);//Espera
    }
}

/*--------------------------- Funcoes definidas nos slides da aula --------------------*/

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