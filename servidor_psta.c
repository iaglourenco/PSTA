#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>

#define CONECTAR "conectar"
#define RECEBER "receber"
#define ENVIAR "enviar"
#define LISTAR "listar\n"
#define ENCERRAR "encerrar\n"

typedef struct
{
    int ctS,dataS;
    char *comando[80];
    struct sockaddr_in client;   

} thread_arg, *ptr_thread_arg;

void * thread_func(void *);
int setup_dataS(struct sockaddr_in);

pthread_mutex_t mutex;

int main(int argc,char *argv[]){

    int ctS,dataS,namelen,ctSThread,dataSThread;
    char *comando[80];
    struct sockaddr_in euMesmo,client;

    //variaveis thread
    int thread_create_result;
    pthread_t ptid;
    thread_arg t_arg;

    int mutex_init_result =pthread_mutex_init(&mutex,NULL);
    if(mutex_init_result != 0){
        perror("ERRO - mutex_init()");
        exit(-1);
    }

    if((ctS = socket(PF_INET,SOCK_STREAM,0)) < 0){
        perror("ERRO - Socket(ctS)");
        exit(-1);
    }

    if(argc != 2){
        printf("Use %s <porta>\n",argv[0]);
        exit(1);
    }

    //Guarda o endereço do servidor para conexoes na porta de dados do cliente
    euMesmo.sin_family = AF_INET;
    euMesmo.sin_port = htons(atoi(argv[1]));
    euMesmo.sin_addr.s_addr = INADDR_ANY;
    if(bind(ctS,(struct sockaddr *)&euMesmo,sizeof(euMesmo))<0){
        perror("ERRO - bind(ctS)");
        exit(-1);
    }
    if(listen(ctS,1) !=0){
        perror("ERRO - Listen(ctS)");
        exit(-1);
    }
    system("clear");
    do
    {
        printf("Servidor PSTA iniciado na porta %s!\nAguardando conexoes...\n",argv[1]);
        namelen = sizeof(client);
        if((ctSThread = accept(ctS,(struct sockaddr *)&client,(socklen_t *)&namelen)) == -1){
            perror("ERRO - Accept(ctS)");
            exit(-1);
        }
        t_arg.ctS = ctSThread;
        memcpy(t_arg.comando,comando,sizeof(comando));
        t_arg.client = client;
        
        thread_create_result = pthread_create(&ptid,NULL,&thread_func,&t_arg);
        if(thread_create_result != 0){
            perror("ERRO - thread_create()");
            exit(thread_create_result);
        }

    } while (1);
    close(ctS);
    pthread_mutex_destroy(&mutex);

    printf("Servidor PSTA encerrado!\n");
    return 0;
}

void * thread_func(void *arg){
//função da thread

ptr_thread_arg thread_arg = (ptr_thread_arg)arg;

int ctS = thread_arg->ctS;
int dataS = thread_arg->dataS;
int pid_thread = pthread_self();
char *comando[80];

printf("LOG - Thread iniciada, id: %u\n",pid_thread);
printf("Conexao aceita de %s porta %d, iniciando thread!\n",
        inet_ntoa(thread_arg->client.sin_addr),
        ntohs(thread_arg->client.sin_port));   

    do{
        if(recv(ctS,comando[0],sizeof(comando[0]),0) == -1){
            perror("ERRO THREAD - Recv(ctS)");
            exit(-1);
        }
        if (strcmp(comando[0],RECEBER) == 0){
            /*Enviar arquivo ao cliente*/
            printf("LOG - Enviando arquivo\n");
        }else if (strcmp(comando[0],ENVIAR) == 0){
            /* Receber arquivo do cliente */
            printf("LOG - Recebendo arquivo\n");
        }else if (strcmp(comando[0],LISTAR) == 0){
            /* Enviar listagem ao cliente*/
            dataS = setup_dataS(thread_arg->client);
            printf("LOG - Listagem requisitada pelo cliente\n");
            if(connect(dataS,(struct sockaddr *)&thread_arg->client,sizeof(thread_arg->client)) < 0){
                perror("ERRO THREAD - connect(dataS)");
                break;
            }
            pthread_mutex_trylock(&mutex);

            /* Operação aqui */


            pthread_mutex_unlock(&mutex);
            if(send(dataS,"OLA MUNDO",sizeof("OLA MUNDO"),0) < 0){
                perror("ERRO THREAD - send(dataS)");
                break;
            }
            close(dataS);
        }

    }while(strcmp(thread_arg->comando[0],ENCERRAR) !=0);
    printf("\nLOG - Thread encerrada, id: %u\n",pid_thread);
    close(ctS);

}

int setup_dataS(struct sockaddr_in info){
    
    int dataS;
    info.sin_port=htons(3315);
    info.sin_family=AF_INET;
    if((dataS = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("ERRO - socket(dataS)");
        return -1;
    }
    return dataS;
}
