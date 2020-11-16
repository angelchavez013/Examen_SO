#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

#define SIZE 512

int main(){
	FILE *file;
	int pfd[2],pfd2[2],pfd3[2];
	char msg[SIZE], linea[SIZE];
	char buffer[SIZE];
	char dirR[60];
	char dirW[60], *dirW2;
	char command[70];
	pid_t pid;
	int status, lineanum, contador;
	
	system ("clear");
	// Se reciben las rutas d eorigen del respaldo y destino
	printf("Ingresa la ruta absoluta del directorio a respaldar: ");
	scanf("%s", dirR);
	printf("Ingresa la ruta absoluta del directorio destino: ");
	scanf("%s", dirW);
	//printf("%s, %s\n", dirR, dirW);

	// El comando test realiza una verificación segun la opcion señalada sobre un directorio o archivo
	// en este caso regresa 0 si existe el directorio indicado
	strcpy(command,"");
	strcat(command,"test -d ");
	strcat(command,dirR);
	if(system(command) != 0){
		printf("No existe el directorio indicado. No se puede realizar un respaldo.\n");
		exit(0);
	}
	//printf("Hoy: %s\n",ctime(&actual));

    printf("Padre: %i", getpid());
    printf("\n=======================================\n");
    printf("\tGENERANDO LISTA DE ARCHIVOS A RESPALDAR\n");
	system("rm test.txt");
	strcpy(command,"");
	strcat(command,"ls ");
	strcat(command,dirR);
	strcat(command," >> test.txt");
	system(command);
		
	// Crea el directorio de respaldo, si existe lo elimina y crea uno limpio
	//dirW2 = "%s $(date +%d%m%Y_%H_%M_%S)",dirW;
	strcpy(command,"");
	strcat(command,"test -d ");
    strcat(command,dirW);
	if(system(command) != 0){
		strcpy(command,"");
		strcat(command,"mkdir ");
		strcat(command,dirW);
		//strcat(command,"_$(date +%d%m%Y_%H_%M_%S)");
		system(command);
	}else{
		strcpy(command,"");
		strcat(command,"rm -r ");
		strcat(command,dirW);
		system(command);
		strcpy(command,"");
		strcat(command,"mkdir ");
		strcat(command,dirW); //Aquí concatenar la fecha y hora
		strcat(command,"_$(date +%d%m%Y_%H_%M_%S)"); 
		system(command);
	}
	// Hasta aquí genera la lista de archivos y crea o sobreescribe el directorio respaldo


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
          	close(pfd[1]);
			close(pfd2[0]);
			close(pfd3[1]);

     		printf("\nHijo(pid=%i) esperando mensaje de mi padre...\n", getpid());
			read(pfd[0],msg,SIZE);
			printf("\nHijo(pid=%i), instruccion del padre: %s\n", getpid(), msg);
			
			write(pfd2[1],"<--- Hola padre, ¿de cuantos archivos sera el respaldo?...\n",SIZE);
			
			printf("\nHijo(pid=%i), archivos a respaldar:\n", getpid());
			for(int i = 0; i < 9; i++){
				read(pfd3[0],msg,SIZE);
				printf("%s\n", msg);
				
			}

			close(pfd[0]);
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
			
			file = fopen("/home/chavez/test.txt","r");
			if(file == NULL){
				perror("\nError al abrir el archivo");
			}else{
				lineanum = atoi(fgets (linea, sizeof linea, file));
				lineanum--;
				while(fgets (linea, sizeof linea, file) != NULL){
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