#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "readline.h"
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define TAM_BUFFER 1024
#define WORD_FILE 20
 

int num_operacoes = 1;

int main(int argc , char* argv[]){

    /* Variaveis */
    char buffer[TAM_BUFFER];
    char buffer_filho[TAM_BUFFER];
    char buffer_ant[TAM_BUFFER];
    char* token = NULL;
    char* aux=NULL,file[WORD_FILE];
    char** command = NULL;
    int n,pos_args,pd[2],pd_ant[2],status,status_ant;


    /*Ficheiros*/
    int fd = open(argv[1],O_RDONLY);
    int fd_final = open("temporario.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    int fd_ant,fd_pf;
    
    while ( (n = readln(fd,buffer,TAM_BUFFER)) > 0  ){            

        buffer[n-1] = '\0';
        printf("%s\n",buffer);
        pipe(pd);
        
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
                printf("%s\n",file);
                
                /*LE DO PAI*/
                close(pd[1]);
                int n = read(pd[0],buffer_filho,TAM_BUFFER);
                close(pd[0]);

                /* TOKENIZA OS COMANDOS */
                aux = (char*) malloc (strlen(buffer_filho));
                strcpy(aux,buffer_filho);
                token = strtok(aux," ");
              
                while( token != NULL ) {

                    command = (char**) realloc (command,pos_args+1);
                    command[pos_args] = strdup(token);
                    write(1,command[pos_args],strlen(command[pos_args]));
                    printf("\n");
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
                    if( command[0][1] == '|' ){

                        pipe(pd_ant);

                        if(fork()==0){

                            /* ABRIR E LER O FICHEIRO ANTERIOR */
                            char file_ant[WORD_FILE] ;
                            sprintf(file_ant,"ficheiro%d.txt",num_operacoes-1); 
                            fd_ant = open(file_ant,O_RDONLY,0666);
                            
                            close(pd_ant[0]);
                            while( (n = readln(fd_ant,buffer_ant,sizeof(buffer_ant))) > 0 ) {
                                write(pd_ant[1],buffer_ant,n);
                            }
                            close(pd_ant[1]);
                            
                            exit(0);
                        }
                        else{

                            /*EXECUTAR O COMANDO PARA O RESULTADO ANTERIOR PARA O FICHEIRO ficheiroN.txt*/
                            wait(&status_ant);
                            close(pd_ant[1]);
                            dup2(pd_ant[0],0);
                            close(pd_ant[0]);
                            execvp((command+1)[0],command+1);
                            perror(0);
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
                num_operacoes++;

                fd_pf = open(file,O_RDONLY,0666);
                while( (n = readln(fd_pf,buffer,sizeof(buffer)))>0){
                    write(fd_final,buffer,n);
                }
                /* ESCREVE NO FICHEIRO FINAL '<<<' */              
                write(fd_final,"<<<\n",4);
            
            }
        }

    }
    
    close(fd);
    close(fd_final);
}