#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>

#define CONECTAR "conectar"
#define RECEBER "receber"
#define ENVIAR "enviar"
#define LISTAR "listar"
#define ENCERRAR "encerrar"
#define PORTA 3315

//#define DEBUG 1

void help();
int setup_dataS(struct sockaddr_in);

int main(void){

    char *comando[80],action[100],action1[100],ctsBuf[20],datasBuf[10000],path[100],* fileBuf;
    int i=0,ctS, dataS, dataSaccept,isConnected=0,inetaddr, namelen;
    struct hostent *hostnm;
    ssize_t size,ret;
    struct sockaddr_in server,euMesmo;
    FILE *fp;
    socklen_t len = sizeof(euMesmo);

    

    //gerar numero aleatorio pra criar porta
    time_t t;
    srand((unsigned)time(&t));

    if((ctS = socket(PF_INET,SOCK_STREAM,0)) < 0){
        perror("Control Socket");
        exit(3);
    }
   

    char portC1[10];
    int portC = htons(rand());
    euMesmo.sin_family=AF_INET;
    euMesmo.sin_port = portC;
    euMesmo.sin_addr.s_addr =INADDR_ANY;

    //Apenas copio a porta gerada formatada
    snprintf(portC1,8,"%d",portC);

    #ifdef DEBUG
    //Print da porta que usarei para receber dados, enviarei isso ao servidor quando conectar
    printf("Porta atribuida: %i, Porta gerada: %i\n",euMesmo.sin_port, portC);
    #endif
    


    if((dataS = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("ERRO - socket(dataS)");
        exit(EXIT_FAILURE);
    }
    if(bind(dataS,(struct sockaddr *)&euMesmo,sizeof(euMesmo)) < 0){
        perror("ERRO - bind()");
        exit(EXIT_FAILURE);  
    }
    if(listen(dataS,1) != 0){
        perror("ERRO - listen()");
        exit(EXIT_FAILURE);
    }


/*if (getsockname(dataS, (struct sockaddr *)&euMesmo, &len) == -1)
    perror("getsockname");
else
    printf("port number %d\n", ntohs(euMesmo.sin_port));
*/

do{
    comando[0]=NULL;//conectar,receber,listar,enviar,encerrar,ajuda,cls/clear
    comando[1]=NULL;//segunda parte do comando, usado em algumas operacoes
    comando[2]=NULL;//mesmo do de cima
    comando[3]=NULL; //porta de dados
    
    printf("psta>");  
    fgets(action1,sizeof(action1),stdin);
    memcpy(action,action1,sizeof(action));
        
    //extracao dos comandos
    comando[0]=strtok(action1," \n");
    comando[1]=strtok(NULL," \n");
    comando[2]=strtok(NULL," \n");
    comando[3]=strtok(NULL," \n");
    
    #ifdef DEBUG
    printf("-COMANDO0: %s",comando[0]);
    printf("-COMANDO1: %s",comando[1]);
    printf("-COMANDO2: %s",comando[2]);
    printf("-COMANDO3: %s",comando[3]);
    #endif

    if(strcmp(comando[0],CONECTAR)==0){
        //Nao digitou hostname e/ou porta
        if(comando[1] == NULL || comando[2] == NULL) goto erro;
        if(strcmp(comando[1],"\n")==0 ||strcmp(comando[2],"\n")==0) goto erro;
        if(strcmp(comando[1],"\0")==0 ||strcmp(comando[2],"\0")==0) goto erro;
        if(isConnected==1)
        {
            printf("Desconectando de %s\n",inet_ntoa(server.sin_addr));
        
            if((send(ctS,ENCERRAR,sizeof(ENCERRAR),0)) < 0) perror("ERRO - send(ctS)");
            
            close(ctS);
            if((ctS = socket(PF_INET,SOCK_STREAM,0)) < 0)
            {
                    perror("Control Socket");
                    exit(3);
            }
           isConnected=0;
        }
        
        hostnm = gethostbyname(comando[1]);
        inetaddr = inet_addr(comando[1]);
        
        server.sin_family=AF_INET;
        server.sin_port = htons(atoi(comando[2]));
        if( hostnm == (struct hostent *) 0)
        {
            if(inetaddr == INADDR_NONE)
            {
                fprintf(stderr,"ERRO - nome do servidor\n");
                goto erro;
            }
            server.sin_addr.s_addr=inetaddr;
        }else server.sin_addr.s_addr=*((unsigned long *)hostnm->h_addr_list[0]);
        if(connect(ctS,(struct sockaddr *)&server,sizeof(server)) < 0) perror("ERRO - connect(ctS)");
        else{
            isConnected=1;
            printf("(conectado a %s na porta %s)\n",comando[1],comando[2]);
        }

        //mandar port de dados de si mesmo para seridor
        if((send(ctS,&portC,sizeof(portC),0)) < 0) perror("ERRO - send(Porta)");

    }else if (strcmp(comando[0],RECEBER)==0){
        /*Receber aqui*/
        if(!isConnected) printf("Por favor conecte-se antes!\n");
        //Nao digitou os parametros requeridos
        if(comando[1] == NULL) goto erro;
        if(strcmp(comando[1],"\n")==0) goto erro;
        if(strcmp(comando[1],"\0")==0) goto erro;
        //se nao houver nome do arquivo local uso o do remoto
        if(comando[2] == NULL || strcmp(comando[2],"\n")==0 || strcmp(comando[2],"\0")==0) comando[2]=comando[1];
        if(send(ctS,action,sizeof(action),0)<0) perror("ERRO - send(ctS)");
        else{
            //configuro o socket de dados
            
            namelen=sizeof(server);
            if((dataSaccept = accept(dataS,(struct sockaddr *)&server,&namelen)) == -1) perror("ERRO - Accept(dataS)");
            else{
                //Operação de recebimento
                
                //verifico o tamanho do arquivo que vou receber 
                if((recv(dataSaccept, datasBuf,sizeof(datasBuf),0)) == -1) perror("ERRO - Recv(dataSaccept)");
                size = atol(datasBuf);
                printf("Tamanho do arquivo a receber: %lu\n",size);
                //aloco o espaco necessario
                fileBuf = malloc(size);
                if((ret = recv(dataSaccept, fileBuf,size,0)) == -1) perror("ERRO - Recv(dataSaccept)");
                else{
                    //Verifico se a leitura foi um sucesso
                    if(strcmp(strerror(atoi(fileBuf)),"Success") == 0){
                        //gero o caminho ao meu futuro arquivo
                        getcwd(path,sizeof(path));
                        strcat(path,"/");
                        strcat(path,comando[2]);
                        fp = fopen(path,"wb");  
                        fwrite(fileBuf,size,1,fp);
                        fclose(fp);
                        printf("Recebidos %lu bytes de %s\n",ret,inet_ntoa(server.sin_addr));
                    }else{
                        printf("ERRO - %s\n",strerror(atoi(fileBuf)));
                    }
                }
                free(fileBuf);
                close(dataSaccept);
            }
        }
    }else if (strcmp(comando[0],ENVIAR)==0){
        /*Enviar aqui*/
        if(!isConnected) printf("Por favor conecte-se antes!\n");
        //Nao digitou os parametros requeridos
        if(comando[1] == NULL) goto erro;
        if(strcmp(comando[1],"\n")==0) goto erro;
        if(strcmp(comando[1],"\0")==0) goto erro;
        if(send(ctS,action,sizeof(action),0)<0) perror("ERRO - send(ctS)");
        else{
            namelen=sizeof(server);
            if((dataSaccept = accept(dataS,(struct sockaddr *)&server,&namelen)) == -1) perror("ERRO - Accept(dataS)");
            else{
                //Operação de envio
                getcwd(path,sizeof(path));
                strcat(path,"/");
                strcat(path,comando[1]);

                fp = fopen(path,"rb");
                if(fp){
                    //verifico o tamanho do arquivo
                    fseek(fp,0,SEEK_END);
                    size = ftell(fp);
                    fseek(fp,0,SEEK_SET);
                    fileBuf = malloc(size);
                    fread(fileBuf,1,size,fp);
                    fclose(fp);
                    //envio o tamanho do arquivo primeiro
                    sprintf(datasBuf,"%lu",size);
                    if(send(dataSaccept,datasBuf,sizeof(size),0) < 0){
                        perror("ERRO - send(dataSaccept)");
                        break;
                    }
                    if(send(dataSaccept,fileBuf,size,0)<0){
                        perror("ERRO - send(dataSaccept)");
                        break;    
                    }
                    free(fileBuf);
                    printf("Enviado %lu bytes para %s\n",size,inet_ntoa(server.sin_addr));
                
                }else{
                    //Falha ao ler
                    perror("ERRO - fread");
                    sprintf(datasBuf,"%lu",sizeof(errno));
                    if(send(dataSaccept,datasBuf,sizeof(errno),0)<0){
                        perror("ERRO - send(dataS)");
                        break;
                    }
                    sprintf(datasBuf,"%d",errno);
                    if(send(dataSaccept,datasBuf,sizeof(errno),0)<0){
                        perror("ERRO - send(dataS)");
                        break;
                    }
                }
                close(dataSaccept);
            }
        }
        
        
    }else if (strcmp(comando[0],LISTAR)==0){
        /*Listar aqui*/
        if(!isConnected) printf("Por favor conecte-se antes!\n");
        if((send(ctS,action,sizeof(action),0)) < 0) perror("ERRO - send(ctS)");
        else
        {
            namelen = sizeof(server);
            if((dataSaccept = accept(dataS, (struct sockaddr *)&server, &namelen)) == -1) perror("ERRO - Accept(dataS)");
            else{
                
                if((recv(dataSaccept, datasBuf,sizeof(datasBuf),0)) == -1) perror("ERRO - Recv(dataSaccept)");
                else fprintf(stdout,"\nDIRETORIO - %s\n",datasBuf);        
                close(dataSaccept);  
            }
        }
    }
    else if (strcmp(comando[0],ENCERRAR)==0){
        break;
    }
    else if (strcmp(comando[0],"clear") == 0 || strcmp(comando[0],"cls") == 0 )
        system("clear");
    
    else if (strcmp(comando[0],"ajuda") == 0)
        help();
    

    else
erro:
        printf("- Comando invalido! -\n- Se confuso esta, o comando 'ajuda' te guiara! - \n");
    
    
    
}while(strcmp(comando[0],ENCERRAR) != 0);

    if(isConnected==1){
        if((send(ctS,ENCERRAR,sizeof(ENCERRAR),0)) < 0) {
            perror("ERRO - send(ctS) SINAL NAO ENVIADO");
            close(dataS);
            close(ctS);
            exit(errno);
        }
    }
    close(dataS);
    close(ctS);
    exit(EXIT_SUCCESS);
}


void help(){
    printf("PSTR 1.0\n");
    printf("by @adrianomunin,@fabioirokawa\n@iaglourenco,@lucasrcoutinho,@marcoslelis\nmore info: https://github.com/iaglourenco/PSTA\n\n");

    printf("Ajuda: \n\n");
    printf("Comandos: \n");
    printf("- %s <nome do servidor> [<porta do servidor>]\n",CONECTAR);
    printf("- %s <arquivo local> [<arquivo remoto>]\n",ENVIAR);
    printf("- %s <arquivo remoto> [<arquivo local>]\n",RECEBER);
    printf("- %s",LISTAR);
    printf("- %s",ENCERRAR);
    printf("- ajuda\n");
    printf("- cls/clear\n");
}

