#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define CONECTAR "conectar"
#define RECEBER "receber"
#define ENVIAR "enviar"
#define LISTAR "listar\n"
#define ENCERRAR "encerrar\n"
#define PORTA 3315

void help();
int conectar();
int enviar();
int receber();
int listar();

int main(void){

    char *comando[80],action[100];
    int i=0;

    do{
    printf("pstr>");    
    fgets(action,sizeof(action),stdin);
    comando[0]=strtok(action," ");
    while (comando[i] != NULL) {
        i++;
        comando[i] = strtok(NULL, " ");           
    }
    if(strcmp(comando[0],CONECTAR)==0){
        /*Conectar aqui*/

    }else if (strcmp(comando[0],RECEBER)==0){
        /*Receber aqui*/

    }else if (strcmp(comando[0],ENVIAR)==0){
        /*Enviar aqui*/
        
    }else if (strcmp(comando[0],LISTAR)==0){
        /*Listar aqui*/
        
    }else if (strcmp(comando[0],ENCERRAR)==0){
        break;
    }else if (strcmp(comando[0],"CLEAR\n") == 0 || strcmp(comando[0],"CLS\n") == 0 ){
        system("clear");
    }else{
        printf(" - Comando invalido! - \n\n");
        help();
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

