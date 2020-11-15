#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define SIZE 512

int main(){
	FILE * file;

	int pfd[2],pfd2[2],pfd3[2];
	char msg[SIZE], linea[SIZE];
	char buffer[SIZE];
	pid_t pid;
	int status, lineanum, contador;
	
	system ("clear");

    printf("Padre: %i", getpid());
    printf("\n=======================================\n");
	system("rm test.txt");
	printf("Se borrara el archivo con los nombres para crear uno nuevo\n\n");
	system("ls -l tareasSO | wc -l >> test.txt"); //Guardar la informacion de un comando
	system("ls tareasSO >> test.txt");

	if (system("mkdir Respaldo") == 0){
		printf("Directorio creado\n");
	}else{
		printf("Se borrara el directorio para crear uno nuevo\n");
		system("rm -r Respaldo");
		system("mkdir Respaldo");
	}

	if (pipe(pfd) < 0) { 
		perror("\nError al crear el pipe");
		exit(1);
	}else if(pipe(pfd2) < 0){
        perror("\nError al crear el pipe");
		exit(1);
    }else if(pipe(pfd3) < 0){
        perror("\nError al crear el pipe");
		exit(1);
    }

	pid = fork();
    switch(pid){
        case -1:
            printf("No se ha podido crear un hijo \n");
            exit(-1);
        break;

        case 0:
          	close(pfd[1]); // Cierra el descriptor de escritura que no va a usar.
			close(pfd2[0]);
			close(pfd3[1]);

     		printf("\nHijo(pid=%i) esperando mensaje de mi padre...\n", getpid());
			read(pfd[0],msg,SIZE);
			printf("\nHijo(pid=%i), instruccion del padre: %s\n", getpid(), msg);
			
			write(pfd2[1],"<--- Hola padre, ¿de cuantos archivos sera el respaldo?...",SIZE);
			
			read(pfd3[0],msg,SIZE);
			/*while(msg != "FIN"){
				if(msg != "FIN"){
					printf("\nHijo(pid=%i), archivos a respaldar:\n", getpid());
					read(pfd3[0],msg,SIZE);
					printf("\n%s\n", msg);
				}else{
					return 0;
				}
				
			}*/

			close(pfd[0]); //Cierra su canal de lectura porque ya termino
			close(pfd2[1]);
			close(pfd3[0]);
			exit(0);
        break;

        default:
          	close(pfd[0]); // Cierra el descriptor de lectura que no va a usar.
			close(pfd2[1]);
			close(pfd3[0]);

			//printf("\nPadre(pid= %i), mensaje a enviar: ", getpid());
			write(pfd[1],"Realiza el siguiente respaldo",SIZE);

			read(pfd2[0], buffer, SIZE);
			printf("\nPadre(pid=%i), lee mensaje del hijo: %s\n", getpid(), buffer);
			
			file = fopen("test.txt","r");
			if(file == NULL){
				perror("\nError al abrir el archivo");
			}else{
				lineanum = atoi(fgets (linea, sizeof linea, file));
				lineanum--;
				while(fgets (linea, sizeof linea, file) != NULL){
					printf("Linea: %d\n",lineanum);
					write(pfd3[1],linea,SIZE);
					contador++;
					if(lineanum == contador)
						write(pfd3[1],"FIN",SIZE);
				}
				
			}
			fclose(file);

			close(pfd[1]); //Cierra su canal de escritura porque ya termino
			close(pfd2[0]);
			close(pfd3[1]);
		break;
	}
	wait(&status);
	printf("\nTermino el proceso padre...\n");
	return 0;
}