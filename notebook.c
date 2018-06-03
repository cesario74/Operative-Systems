#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "readline.h"
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <ctype.h>

#define TAM_BUFFER 1024
#define WORD_FILE 20
#define TEMPORARIO "temporario.txt"
#define ERRO "std_err.txt"

/*VARIAVEIS GLOBAIS*/
int num_operacoes = 1;
int fd, fd_err, fd_final;


void remove_all_right(char* orig){

    char file[WORD_FILE];
    
    close(fd);            
    close(fd_err);
    close(fd_final);

    remove(orig);
    rename(TEMPORARIO,orig); 
    remove(ERRO);

    for (int i = 1; i <= num_operacoes ; i++){
        sprintf(file,"ficheiro%d.txt",i);
        remove(file);
    } 
}
void remove_all_fail(){

    char file[WORD_FILE];

    close(fd);
    close(fd_err);
    close(fd_final);

    remove(TEMPORARIO);
    remove(ERRO);

    for (int i = 1; i <= num_operacoes ; i++){
        sprintf(file,"ficheiro%d.txt",i);
        remove(file);
    }

}

void handler (int signal){

    switch(signal){
        case SIGINT:
            remove_all_fail();
            break;
    }

}


int main(int argc , char* argv[]){

    /* Variaveis */
    char buffer[TAM_BUFFER];
    char buffer_filho[TAM_BUFFER];
    char buffer_ant[TAM_BUFFER];
    char* token = NULL;
    char* aux=NULL,file[WORD_FILE];
    char* numero = NULL;
    char** command = NULL;
    int n,i,pos_args,status,status_ant,command_ant,flag = 0,size;
    int pd[2],pd_ant[2];
    FILE* f_err;

    /*Ficheiros*/
    fd_final;
    int fd_ant,fd_pf;

    /*SINAIS*/
    signal(SIGINT,handler);
    int a =1;
    while(a<argc){
        fd = open(argv[a],O_RDONLY);
        fd_err = open(ERRO,O_RDWR | O_CREAT | O_TRUNC, 0666);
    
        if (fd_err < 0){
            perror("Erro a criar o ficheiro do stderror\n");
            _exit(-1);
        }
        else{

            dup2(fd_err,2);
            if( fd < 0 ){
                perror("Erro a abrir o notebook\n");
                _exit(-2);
            }
            else{

                fd_final = open(TEMPORARIO, O_RDWR | O_CREAT | O_TRUNC, 0666);

                if(fd_final < 0){
                    perror("Erro a criar o ficheiro temporário\n");
                    _exit(-3);
                }
                else{
    
                    while ( (n = readln(fd,buffer,TAM_BUFFER)) > 0  ){            

                        buffer[n-1] = '\0';
                    
                        pipe(pd);

                        if (strcmp(buffer,">>>") == 0 ){
                            flag = 1;
                        }
                        else{
                            if(strcmp(buffer,"<<<")==0){
                                flag = 0;
                            }
                            else{
                                if( flag == 0){
                                
                                    /* PAI ESCREVE NO FICHEIRO FINAL LINHA LIDA */
                                    write(fd_final,buffer,n-1);
                                    write(fd_final,"\n",1);
                            
                                

                                    pos_args = 0;
                                    if(buffer[0] == '$'){

                                        /* PAI ESCREVE NO FICHEIRO FINAL '>>>' */
                                        write(fd_final,">>>\n",4);
                                        /* PAI NOMEIA FICHEIRO FILHO */
                                        sprintf(file,"ficheiro%d.txt",num_operacoes);
                                    

                                        if(fork() == 0){

                                            /* CRIA FICHEIRO */
                                            int fd_filho = open(file, O_RDWR | O_CREAT | O_TRUNC ,0666);

                                            if (fd_filho < 0){
                                                perror("Erro ao criar o ficheiro do filho\n");
                                                _exit(-4);
                                            }
                                            else{
                                            
                                                /*LE DO PAI*/
                                                close(pd[1]);
                                                int n = read(pd[0],buffer_filho,TAM_BUFFER);
                                                close(pd[0]);

                                                /* TOKENIZA OS COMANDOS */
                                                aux = (char*) malloc (strlen(buffer_filho));
                                                strcpy(aux,buffer_filho);
                                                token = strtok(aux," ");
                                                
                                                while( token != NULL ) {
                                                    command = (char**) realloc (command,(pos_args + 1)*sizeof(char *));
                                                    command[pos_args] = strdup(token);
                                                    token = strtok(NULL," ");
                                                    pos_args++;
                                                    
                                                }
                                                
                                                command[pos_args] = NULL;

                                                dup2(fd_filho,1);

                                                if( strlen (command[0]) == 1 ){

                                                    /*EXECUTA PARA O FICHEIRO O COMANDO*/
                                                    execvp((command+1)[0],command+1);
                                                    perror(0);

                                                }
                                                else{
                                                    if( strcmp(command[0],"$") != 0 ){

                                                        pipe(pd_ant);

                                                        /*VERIFICAR SE TEM NUMERO ENTRE DOLAR E PIPE*/
                                                        if ( isdigit(command[0][1]) ){      

                                                            /*RECOLHER O NUMERO E PASSAR PARA INT*/
                                                            i = 1;
                                                            while(isdigit(command[0][i])){
                                                                numero = (char*) realloc (numero,i);
                                                                numero[i-1] = command[0][i];
                                                                i++;
                                                            }
                                                            command_ant = atoi(numero);
                                                        
                                                        }
                                                        else{

                                                            /*ASSUME VALOR DE ! PARA IR BUSCAR O COMANDO ANTERIOR*/
                                                            if( command[0][1] == '|' ) command_ant = 1;

                                                        }

                                                        if(fork()==0){

                                                            /* ABRIR,LER E FECHAR O FICHEIRO ANTERIOR */
                                                            char file_ant[WORD_FILE] ;
                                                            sprintf(file_ant,"ficheiro%d.txt",num_operacoes-command_ant); 
                                                            fd_ant = open(file_ant,O_RDONLY,0666);

                                                            if (fd_ant >= 0){
                                                            
                                                                close(pd_ant[0]);
                                                                while( (n = readln(fd_ant,buffer_ant,sizeof(buffer_ant))) > 0 ) {
                                                                    write(pd_ant[1],buffer_ant,n);
                                                                }
                                                                close(pd_ant[1]);
                                                                close(fd_ant);

                                                            }
                                                            else{
                                                                perror("Erro a abrir o ficheiro anterior \n");
                                                                _exit(-5);
                                                            }
                                                            
                                                            exit(0);
                                                        }
                                                        else{

                                                            
                                                            wait(&status_ant);

                                                            /* VE SE O FILHO ACABA BEM*/

                                                            if(WIFEXITED(status_ant)){
                                                                if(WEXITSTATUS(status) < 0){
                                                                    _exit(-6);
                                                                }
                                                            }
                                                            
                                                            /*EXECUTAR O COMANDO PARA O RESULTADO ANTERIOR PARA O FICHEIRO ficheiroN.txt*/
                                                            close(pd_ant[1]);
                                                            dup2(pd_ant[0],0);
                                                            close(pd_ant[0]);
                                                            execvp((command+1)[0],command+1);
                                                            perror(0);
                                                        }

                                                    }
                                                }
                                            }
                                            exit(0);
                                        }
                                        else{

                                            close(pd[0]);
                                            write(pd[1],buffer,n);
                                            close(pd[1]);
                                            wait(&status);
                                            if ( WIFEXITED(status)){
                                                if (WEXITSTATUS(status) == 0){
                                                    f_err = fdopen(fd_err,"w");
                                                    fseek (f_err, 0, SEEK_END);
                                                    size = ftell(f_err);
                                                    if (size != 0) {
                                                        remove_all_fail();
                                                    }
                                                }
                                                else{
                                                    remove_all_fail();
                                                }
                                            }
                                            num_operacoes++;

                                            /*LE FICHEIRO REFERENTE AO FILHO DESTA ITERAÇAO */

                                            fd_pf = open(file,O_RDONLY,0666);
                                            while( (n = readln(fd_pf,buffer,sizeof(buffer)))>0){
                                                write(fd_final,buffer,n);
                                            }
                                            close(fd_pf);

                                            /* ESCREVE NO FICHEIRO FINAL '<<<' */              
                                            write(fd_final,"<<<\n",4);
                                        
                                        }
                                    }
                                }
                            }
                        }
                    }

                        remove_all_right(argv[a]);
                    
                                
                     
                }
       
                    
                }
            }
             a++;
         }  
             return 0;
            
        }

