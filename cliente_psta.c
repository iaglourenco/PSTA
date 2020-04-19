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

#define CONECTAR "conectar"
#define RECEBER "receber"
#define ENVIAR "enviar"
#define LISTAR "listar\n"
#define ENCERRAR "encerrar\n"
#define PORTA 3315


//#define DEBUG 1

void help();
int conectar();
int enviar();
int receber();
int listar();

int main(void){

    char *comando[80],action[100];
    int i=0,ctS,dataS,ret,isConnected=0,inetaddr;
    struct hostent *hostnm;
    struct sockaddr_in server;
    if(ctS = socket(PF_INET,SOCK_STREAM,0) < 0){
        perror("Control Socket");
        exit(3);
    }
    if(dataS = socket(PF_INET,SOCK_STREAM,0) < 0){
        perror("Data Socket");
        exit(3);
    }
    do{
    comando[0]=NULL;
    comando[1]=NULL;
    comando[2]=NULL;
    printf("pstr>");  
    fgets(action,sizeof(action),stdin);
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

    if(strcmp(comando[0],CONECTAR)==0){
        //Nao digitou hostname e/ou porta
        if(comando[1] == NULL || comando[2] == NULL) goto erro;
        if(strcmp(comando[1],"\n")==0 ||strcmp(comando[2],"\n")==0) goto erro;
        if(strcmp(comando[1],"\0")==0 ||strcmp(comando[2],"\0")==0) goto erro;
        
        hostnm = gethostbyname(comando[1]);
        inetaddr = inet_addr(comando[1]);
        
        server.sin_family=AF_INET;
        server.sin_port = htons(atoi(comando[2]));
        if( hostnm == (struct hostent *) 0){
            if(inetaddr == INADDR_NONE){
                fprintf(stderr,"ERRO - nome do servidor\n");
                goto erro;
            }
            server.sin_addr.s_addr=inetaddr;
        }else server.sin_addr.s_addr=*((unsigned long *)hostnm->h_addr_list[0]);

        printf("Socket fd=%i",ctS);
        if(connect(ctS,(struct sockaddr *)&server,sizeof(server)) < 0){
            perror("ERRO - connect()");
        }else{
            printf("(conectado a %s:%s",comando[1],comando[2]);
        }
    
    }else if (strcmp(comando[0],RECEBER)==0){
        /*Receber aqui*/

    }else if (strcmp(comando[0],ENVIAR)==0){
        /*Enviar aqui*/
        
    }else if (strcmp(comando[0],LISTAR)==0){
        /*Listar aqui*/
        if(!isConnected) goto erro;
        
    }else if (strcmp(comando[0],ENCERRAR)==0){
        break;
    }else if (strcmp(comando[0],"clear\n") == 0 || strcmp(comando[0],"cls\n") == 0 ){
        system("clear");
    }else if (strcmp(comando[0],"help\n") == 0)
        help();   /* code */
    else{
erro:
        printf(" - Comando invalido! - \n\n");
    }
    

}while(strcmp(comando[0],ENCERRAR) != 0);
    exit(0);
}


void help(){
    printf("PSTR 1.0\n");
    printf("by @adrianomunin,@fabioirokawa\n@iaglourenco,@lucasrcoutinho,@marcoslelis\nmore info: https://github.com/iaglourenco/PSTR\n\n");

    printf("Ajuda: \n\n");
    printf("Comandos: \n");
    printf("- %s <nome do servidor> [<porta do servidor>]\n",CONECTAR);
    printf("- %s <arquivo local> [<arquivo remoto>]\n",ENVIAR);
    printf("- %s <arquivo remoto> [<arquivo local>]\n",RECEBER);
    printf("- %s",LISTAR);
    printf("- %s",ENCERRAR);
    
}
int conectar(){

    return 0;
}
int enviar(){

    return 0;
}
int receber(){
    
    return 0;
}
int listar(){

    return 0;
}

