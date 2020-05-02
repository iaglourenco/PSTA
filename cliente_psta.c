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
    int i=0,ctS, dataS, dataSaccept,isConnected=0, inetaddr, namelen;
    struct hostent *hostnm;
    long int size,ret;
    struct sockaddr_in server,euMesmo;
    FILE *fp;
    
    if((ctS = socket(PF_INET,SOCK_STREAM,0)) < 0){
        perror("Control Socket");
        exit(3);
    }

do{
    comando[0]=NULL;
    comando[1]=NULL;
    comando[2]=NULL;
    printf("psta>");  
    fgets(action1,sizeof(action1),stdin);
    memcpy(action,action1,sizeof(action));
        
    //extracao dos comandos
    comando[0]=strtok(action1," \n");
    comando[1]=strtok(NULL," \n");
    comando[2]=strtok(NULL," \n");
    
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
        if( hostnm == (struct hostent *) 0)
        {
            if(inetaddr == INADDR_NONE)
            {
                fprintf(stderr,"ERRO - nome do servidor\n");
                goto erro;
            }
            server.sin_addr.s_addr=inetaddr;
        }else server.sin_addr.s_addr=*((unsigned long *)hostnm->h_addr_list[0]);


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
        
        if(connect(ctS,(struct sockaddr *)&server,sizeof(server)) < 0) perror("ERRO - connect(ctS)");
        
        else
        {
            isConnected=1;
            printf("(conectado a %s na porta %s)\n",comando[1],comando[2]);
        }

    }else if (strcmp(comando[0],RECEBER)==0){
        /*Receber aqui*/
        if(!isConnected) printf("Por favor conecte-se antes!\n");
        //Nao digitou os parametros requeridos
        if(comando[1] == NULL) goto erro;
        if(strcmp(comando[1],"\n")==0) goto erro;
        if(strcmp(comando[1],"\0")==0) goto erro;
        //se nao houver nome do arquivo local uso o do remoto
        if(comando[2] == NULL || strcmp(comando[2],"\n")==0 || strcmp(comando[2],"\0")==0) comando[2]=comando[1];
        if((dataS=setup_dataS(euMesmo)) == -1){
            break;
        }
        if(send(ctS,action,sizeof(action),0)<0) perror("ERRO - send(ctS)");
        else{
            //configuro o socket de dados
            
            namelen=sizeof(server);
            if((dataSaccept = accept(dataS,(struct sockaddr *)&server,&namelen)) == -1) perror("ERRO - Accept(dataS)");
            else{
                //Operação de recebimento
                
                //verifico o tamanho do arquivo que vou receber 
                if((recv(dataSaccept, datasBuf,sizeof(long int),0)) == -1) perror("ERRO - Recv(dataSaccept)");
                size = atol(datasBuf);
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
                        printf("Recebidos %ld bytes de %s\n",ret,inet_ntoa(server.sin_addr));
                    }else{
                        printf("ERRO - %s\n",strerror(atoi(fileBuf)));
                    }
                }
                free(fileBuf);
                close(dataSaccept);
            }
        }
        close(dataS);
    }else if (strcmp(comando[0],ENVIAR)==0){
        /*Enviar aqui*/
        if(!isConnected) printf("Por favor conecte-se antes!\n");
        //Nao digitou os parametros requeridos
        if(comando[1] == NULL) goto erro;
        if(strcmp(comando[1],"\n")==0) goto erro;
        if(strcmp(comando[1],"\0")==0) goto erro;
        if((dataS=setup_dataS(euMesmo)) == -1){
            break;
        }
        if(send(ctS,action,sizeof(action),0)<0) perror("ERRO - send(ctS)");
        else{
            //configuro o socket de dados
            namelen=sizeof(server);
            if((dataSaccept = accept(dataS,(struct sockaddr *)&server,&namelen)) == -1) perror("ERRO - Accept(dataS)");
            else{
                //Operação de envio
                //gero o caminho do arquivo solicitado
                getcwd(path,sizeof(path));
                strcat(path,"/");
                strcat(path,comando[1]);

                fp = fopen(path,"rb");
                if(fp){
                //determino o tamanho do arquivo    
                    fseek(fp,0,SEEK_END);
                    size = ftell(fp);
                    fseek(fp,0,SEEK_SET);
                    fileBuf = malloc(size);
                    fread(fileBuf,1,size,fp);
                    fclose(fp);
                //envio o tamanho do arquivo primeiro
                sprintf(datasBuf,"%ld",size);
                if(send(dataSaccept,datasBuf,sizeof(size),0) < 0){
                    perror("ERRO - send(dataS)");
                    break;
                }
                //depois o arquivo em si
                if(send(dataSaccept,fileBuf,size,0) < 0){
                    perror("ERRO - send(dataS)");
                    break;
                }
                printf("Enviado %ld bytes para %s\n",size,inet_ntoa(server.sin_addr));
                }else perror("ERRO - fread");

                free(fileBuf);
                close(dataSaccept); 
            } 
        }
        close(dataS);
    }else if (strcmp(comando[0],LISTAR)==0){
        /*Listar aqui*/
        if(!isConnected) printf("Por favor conecte-se antes!\n");
        if((send(ctS,action,sizeof(action),0)) < 0) perror("ERRO - send(ctS)");
        else
        {
            //configuro o socket de dados
            dataS = setup_dataS(euMesmo);

            namelen = sizeof(server);
            if((dataSaccept = accept(dataS, (struct sockaddr *)&server, &namelen)) == -1) perror("ERRO - Accept(dataS)");
            else{
                
                if((recv(dataSaccept, datasBuf,sizeof(datasBuf),0)) == -1) perror("ERRO - Recv(dataSaccept)");
                else fprintf(stdout,"\nDIRETORIO - %s\n",datasBuf);    
                close(dataS);    
                close(dataSaccept);  
            }
        }
    }
    else if (strcmp(comando[0],ENCERRAR)==0)
    {
        if((send(ctS,action,sizeof(action),0)) < 0) perror("ERRO - send(ctS)");
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
    exit(EXIT_SUCCESS);
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
    printf("- ajuda\n");
    printf("- cls/clear\n");
}

int setup_dataS(struct sockaddr_in euMesmo){
    
    int dataS;
    euMesmo.sin_family=AF_INET;
    euMesmo.sin_port = htons(PORTA);
    euMesmo.sin_addr.s_addr =INADDR_ANY;
    if((dataS = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("ERRO - socket(dataS)");
        return -1;
    }
    if(bind(dataS,(struct sockaddr *)&euMesmo,sizeof(euMesmo)) < 0){
        perror("ERRO - bind()");
        return -1;  
    }
    if(listen(dataS,1) != 0){
        perror("ERRO - listen()");
        return -1;
    }
    return dataS;
}