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
//#define DEBUG 1

typedef struct
{
    int ctS,dataS;
    struct sockaddr_in client;   

} thread_arg, *ptr_thread_arg;

void * thread_func(void *);
int setup_dataS(struct sockaddr_in);

pthread_mutex_t mutex;

int main(int argc,char *argv[]){

    int ctS,dataS,namelen,ctSThread,dataSThread;
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
    printf("Servidor PSTA iniciado na porta %s!\nAguardando conexoes...\n",argv[1]);
    do
    {
        namelen = sizeof(client);
        if((ctSThread = accept(ctS,(struct sockaddr *)&client,(socklen_t *)&namelen)) == -1){
            perror("ERRO - Accept(ctS)");
            exit(-1);
        }

        t_arg.ctS = ctSThread;
        t_arg.client = client;
        
        thread_create_result = pthread_create(&ptid,NULL,&thread_func,&t_arg);
        if(thread_create_result != 0){
            perror("ERRO - thread_create()");
            exit(thread_create_result);
        }

    } while (1);
    close(ctS);
    pthread_mutex_destroy(&mutex);

    printf("Servidor PSTA encerrado!");
    return 0;
}

void * thread_func(void *arg){
//função da thread

ptr_thread_arg thread_arg = (ptr_thread_arg)arg;

int ctS = thread_arg->ctS;
int dataS = thread_arg->dataS;
struct sockaddr_in client =thread_arg->client;
client.sin_port = htons(3315);
int pid_thread = pthread_self();
FILE *fp;
char *comando[80];//array para o strtok
char action[100];//entrada recebida em ascii
char sendBuf[200];
char list[200];
printf("LOG - Conexao aceita de %s porta %d, thread ID: %u\n",
        inet_ntoa(thread_arg->client.sin_addr),
        ntohs(thread_arg->client.sin_port),pid_thread);
    do{
      
     if(recv(ctS, action,sizeof(action),0) == -1){
            perror("ERRO THREAD - Recv(ctS)");
            exit(-1);
        }
        //tokenizacao da string recebida
        comando[0]=strtok(action," ");
        if(comando[0]!=NULL) comando[0][strlen(comando[0])]='\0';
        else comando[0] = "ERRO";
        comando[1]=strtok(NULL," ");
        if(comando[1]!=NULL) comando[1][strlen(comando[1])]='\0';
        comando[2]=strtok(NULL," ");
        if(comando[2]!=NULL) comando[2][strlen(comando[2])-1]='\0';

        #ifdef DEBUG
            printf("-COMANDO0: %s",comando[0]);
            printf("-COMANDO1: %s",comando[1]);
            printf("-COMANDO2: %s",comando[2]);
        #endif

        if(strcmp(comando[0], ENCERRAR) ==0){
            break;
        }
        if (strcmp(comando[0], RECEBER) == 0){
            /*Enviar arquivo ao cliente*/
            printf("LOG - Enviando arquivo ao cliente, id: %u\n",pid_thread);
        }else if (strcmp(comando[0], ENVIAR) == 0){
            /* Receber arquivo do cliente */
            printf("LOG - Recebendo arquivo do cliente, id: %u\n",pid_thread);
        }else if (strcmp(comando[0], LISTAR) == 0){
            /* Enviar listagem ao cliente*/
            dataS = setup_dataS(client);
            printf("LOG - Listagem requisitada pelo cliente, id: %u\n",pid_thread);
            
            if(connect(dataS,(struct sockaddr *)&client,sizeof(client)) < 0){
                perror("ERRO THREAD - connect(dataS)");
                break;
            }
            pthread_mutex_trylock(&mutex);
            
            getcwd(sendBuf,sizeof(sendBuf));
            fp = popen("ls","r");
            strcat(sendBuf,"\n----\n");
            while(fgets(list,sizeof(list),fp)!= NULL){
                strcat(sendBuf,list);
            }
            pclose(fp);
            pthread_mutex_unlock(&mutex);
            

            if(send(dataS,sendBuf,sizeof(sendBuf),0) < 0){
                perror("ERRO THREAD - send(dataS)");
                break;
            }
            
            close(dataS);
        }

    }while(strcmp(comando[0], ENCERRAR) !=0);
    printf("LOG - Conexao de %s porta %d, encerrada, thread ID: %u\n",
        inet_ntoa(thread_arg->client.sin_addr),
        ntohs(thread_arg->client.sin_port),pid_thread);
    close(ctS);

}

int setup_dataS(struct sockaddr_in info){
    
    int dataS;
    if((dataS = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("ERRO - socket(dataS)");
        return -1;
    }
    return dataS;
}