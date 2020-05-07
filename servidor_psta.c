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
#define LISTAR "listar"
#define ENCERRAR "encerrar"


//#define DEBUG 1

//struct de parametros da thread
typedef struct
{
    int ctS,dataS,portClient;
    struct sockaddr_in client;   

} thread_arg, *ptr_thread_arg;

//headers
void * thread_func(void *);
int setup_dataS(struct sockaddr_in);

//mutex
pthread_mutex_t mutex;

int main(int argc,char *argv[]){

    
    int ctS,dataS,namelen,ctSThread,dataSThread;
    int portRecv;
    struct sockaddr_in euMesmo,client;

    //variaveis thread
    int thread_create_result, portC;
    pthread_t ptid;
    thread_arg t_arg;

    //crio o mutex
    int mutex_init_result =pthread_mutex_init(&mutex,NULL);
    if(mutex_init_result != 0){
        perror("ERRO - mutex_init()");
        exit(-1);
    }
    //crio o socket de controle
    if((ctS = socket(PF_INET,SOCK_STREAM,0)) < 0){
        perror("ERRO - Socket(ctS)");
        exit(-1);
    }

    //digitou errado padawan!
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
        exit(errno);
    }
    if(listen(ctS,1) !=0){
        perror("ERRO - Listen(ctS)");
        exit(errno);
    }

    system("clear");
    printf("Servidor PSTA iniciado na porta %s!\nAguardando conexoes...\n",argv[1]);
    
    do
    {
        namelen = sizeof(client);
        if((ctSThread = accept(ctS,(struct sockaddr *)&client,(socklen_t *)&namelen)) == -1){
            perror("ERRO - Accept(ctS)");
            exit(errno);
        }

        t_arg.ctS = ctSThread;
        t_arg.client = client;
        
        if (recv(ctSThread, &portRecv, sizeof(portRecv), 0) == -1)
        {
            perror("ERRO - Recv(portRecv)");
            exit(errno);
        }

        t_arg.portClient = portRecv;
        
        #ifdef DEBUG
            printf("Porta informada: %d\n",t_arg.portClient);
        #endif

        thread_create_result = pthread_create(&ptid,NULL,&thread_func,&t_arg);
        if(thread_create_result != 0){
            perror("ERRO - thread_create()");
            exit(thread_create_result);
        }

    } while (1);

    close(ctS);
    pthread_mutex_destroy(&mutex);
    printf("Servidor PSTA encerrado!");
    
    return EXIT_SUCCESS;
}

void * thread_func(void *arg){
//função da thread

ptr_thread_arg thread_arg = (ptr_thread_arg)arg;

//passo todos os parametros da struct para variaveis locais 
int ctS = thread_arg->ctS;
int dataS = thread_arg->dataS;
struct sockaddr_in client =thread_arg->client;
client.sin_port = thread_arg->portClient;
int pid_thread = pthread_self();
FILE *fp;
ssize_t size,ret;
char *comando[80];//array para o strtok
char action[100];//entrada recebida em ascii

//ISSO NAO EH BONITO, DEVIA SER DINAMICO
char datasBuf[10000],list[10000];//strings para envio de dados
char path[1000];//string com o caminho dos arquivos
char * fileBuf; // ALOCACAO DINAMICA :), envio ou recebimento de dados

printf("LOG - Conexao aceita de %s porta %d, cliente id: %u\n",
        inet_ntoa(thread_arg->client.sin_addr),
        ntohs(thread_arg->client.sin_port),pid_thread);
    do{
    comando[0]=NULL;//conectar,enviar,listar,receber,encerrar
    comando[1]=NULL;//nome do arquivo pra receber ou enviar
    comando[2]=NULL;// same as above

     if(recv(ctS, action,sizeof(action),0) == -1){
            //perror("ERRO - Recv(ctS)");
            fprintf(stderr,"ERRO - Recv(ctS): %s, cliente id: %u\n",strerror(errno),pid_thread);
            exit(errno);
        }
        
        //tokenizacao da string recebida
        comando[0]=strtok(action," \n");
        comando[1]=strtok(NULL," \n");
        comando[2]=strtok(NULL," \n");
        


    #ifdef DEBUG
        printf("-COMANDO0: %s",comando[0]);
        printf("-COMANDO1: %s",comando[1]);
        printf("-COMANDO2: %s",comando[2]);
    #endif


        if(comando[0] == NULL) break;
        if(strcmp(comando[0], ENCERRAR) ==0){
            break;
        }
        
        if (strcmp(comando[0], RECEBER) == 0){
            /*Enviar arquivo ao cliente*/
            pthread_mutex_trylock(&mutex);
            if((dataS = setup_dataS(client)) < 0){
                break;
            }
            if(connect(dataS,(struct sockaddr *)&client,sizeof(client)) < 0){
                fprintf(stderr,"ERRO - connect(dataS): %s, cliente id: %u\n",strerror(errno),pid_thread);
                break;
            }
            else{
                printf("LOG - Enviando arquivo ao cliente, id: %u\n",pid_thread);
                //gero o caminho do arquivo solicitado
                getcwd(path,sizeof(path));
                strcat(path,"/");
                strcat(path,comando[1]);

                fp = fopen(path,"rb");

                if(fp) {
                    //determino o tamanho do arquivo    
                    fseek(fp,0,SEEK_END);
                    size = ftell(fp);
                    fseek(fp,0,SEEK_SET);
                    fileBuf = malloc(size);
                    fread(fileBuf,size,1,fp);
                    fclose(fp);
                    //envio o tamanho do arquivo primeiro
                    sprintf(datasBuf,"%lu",size);
                    if(send(dataS,datasBuf,sizeof(datasBuf),0) < 0){
                        fprintf(stderr,"ERRO - send(dataS): %s, cliente id: %u\n",strerror(errno),pid_thread);
                        break;
                    }
                    //depois o arquivo em si
                    if((ret = send(dataS,fileBuf,size,0)) < 0){
                        fprintf(stderr,"ERRO - send(dataS): %s, cliente id: %u\n",strerror(errno),pid_thread);
                        break;
                    }
                    free(fileBuf);
                    printf("LOG - Enviado %lu bytes ao cliente, id: %u\n",ret,pid_thread);

                }else{ 
                    //Falha na leitura
                    fprintf(stderr,"ERRO - fread: %s, cliente id: %u\n",strerror(errno),pid_thread);
                    sprintf(datasBuf,"%lu",sizeof(errno));
                    if(send(dataS,datasBuf,sizeof(long),0) < 0){
                        fprintf(stderr,"ERRO - send(dataS): %s, cliente id: %u\n",strerror(errno),pid_thread);
                        break;
                    }
                    sprintf(datasBuf,"%d",errno);
                    if(send(dataS,datasBuf,sizeof(errno),0) < 0){
                        fprintf(stderr,"ERRO - send(dataS): %s, cliente id: %u\n",strerror(errno),pid_thread);
                        break;
                    }
                }
            }   
            close(dataS);
            pthread_mutex_unlock(&mutex);
            
        }else if (strcmp(comando[0], ENVIAR) == 0){
            /* Receber arquivo do cliente */
            if(comando[2] == NULL || strcmp(comando[2],"\n")==0 || strcmp(comando[2],"\0")==0) comando[2]=comando[1];
            pthread_mutex_trylock(&mutex);
            if((dataS = setup_dataS(client)) < 0){
                break;
            }
            if(connect(dataS,(struct sockaddr *)&client,sizeof(client))<0){
                fprintf(stderr,"ERRO - connect(dataS): %s, cliente id: %u\n",strerror(errno),pid_thread);
                break;
            }else{
                printf("LOG - Recebendo arquivo do cliente, id: %u\n",pid_thread);
                //recebo o tamanho do arquivo
                if(recv(dataS,datasBuf,sizeof(size),0) == -1) fprintf(stderr,"ERRO - recv(dataS): %s, cliente id: %u\n",strerror(errno),pid_thread);
                size = atol(datasBuf);
                fileBuf = malloc(size);
                if((ret = recv(dataS,fileBuf,size,0)) == -1) fprintf(stderr,"ERRO - recv(dataS): %s, cliente id: %u\n",strerror(errno),pid_thread);
                else{
                    if(strcmp(strerror(atoi(fileBuf)),"Success") == 0){
                        getcwd(path,sizeof(path));
                        strcat(path,"/");
                        strcat(path,comando[2]);
                        fp = fopen(path,"wb");
                        fwrite(fileBuf,size,1,fp);
                        fclose(fp);
                        printf("LOG - Recebidos %lu bytes do cliente, id: %u\n",ret,pid_thread);
                    }else{
                        printf("ERRO - %s\n",strerror(atoi(fileBuf)));
                    }
                }
                free(fileBuf);
            }
            close(dataS);
            pthread_mutex_unlock(&mutex);
        }else if (strcmp(comando[0], LISTAR) == 0){
            /* Enviar listagem ao cliente*/
            
            if((dataS = setup_dataS(client)) < 0){
                break;
            }
            
            pthread_mutex_trylock(&mutex);
            if(connect(dataS,(struct sockaddr *)&client,sizeof(client)) < 0){
                fprintf(stderr,"ERRO - connect(dataS): %s, cliente id: %u\n",strerror(errno),pid_thread);
                break;
            }else{
                getcwd(datasBuf,sizeof(datasBuf));
                fp = popen("ls","r");
                strcat(datasBuf,"\n----\n");
                while(fgets(list,sizeof(list),fp)!= NULL){
                    strcat(datasBuf,list);
                }
                pclose(fp);

                if(send(dataS,datasBuf,sizeof(datasBuf),0) < 0){
                    fprintf(stderr,"ERRO - send(dataS): %s, cliente id: %u\n",strerror(errno),pid_thread);
                    break;
                }
                printf("LOG - Listagem enviada ao cliente, id: %u\n",pid_thread);
                pthread_mutex_unlock(&mutex);
                close(dataS);
            }
        }
    }while(strcmp(comando[0], ENCERRAR) !=0);
    printf("LOG - Conexao de %s porta %d, encerrada, cliente id: %u\n",
        inet_ntoa(thread_arg->client.sin_addr),
        ntohs(thread_arg->client.sin_port),pid_thread);
    close(ctS);
    close(dataS);

}

int setup_dataS(struct sockaddr_in info){
    
    int dataS;
    if((dataS = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("ERRO - socket(dataS)");
        return errno;
    }
    return dataS;
}