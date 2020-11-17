#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define SIZE 512

int main(){
	FILE * file;
	int pfd[2],pfd2[2],pfd3[2],pfd4[2];
	char msg[SIZE], linea[SIZE];
	char buffer[SIZE];
	char dirR[60];
	char dirW[60];
	char command[70];
	char lineas[40];
	pid_t pid;
	int status, lineanum=0, contador=0, total;

	system ("clear");

	printf("Ingresa la ruta absoluta del directorio a respaldar: ");
	scanf("%s", dirR);
	printf("Ingresa la ruta absoluta del directorio destino: ");
	scanf("%s", dirW);

	// El comando test realiza una verificación segun la opcion señalada sobre un directorio o archivo
	// en este caso regresa 0 si existe el directorio indicado
	strcpy(command,"");
	strcat(command,"test -d ");
	strcat(command,dirR);
	if(system(command) != 0){
		printf("No existe el directorio indicado. No se puede realizar un respaldo.\n");
		exit(0);
	}
    printf("Padre: %i", getpid());
    printf("\n=======================================\n");
    printf("\tGENERANDO LISTA DE ARCHIVOS A RESPALDAR\n");
	system("rm test.txt");
	strcpy(command,"");
	strcat(command,"ls ");
	strcat(command,dirR);
	strcat(command," >> test.txt");
	//system("ls -l tareasSO | wc -l >> test.txt"); //Guardar la informacion de un comando
	system(command);

	// Crea el directorio de respaldo, si existe lo elimina y crea uno limpio
	strcpy(command,"");
	strcat(command,"test -d ");
    strcat(command,dirW);
	if(system(command) != 0){
		strcpy(command,"");
		strcat(command,"mkdir ");
		strcat(command,dirW);
		system(command);
	}else{
		printf("Eliminando respaldo anterior...\n");
		strcpy(command,"");
		strcat(command,"rm -r -v ");
		strcat(command,dirW);
		system(command);
		strcpy(command,"");
		strcat(command,"mkdir ");
		strcat(command,dirW); //Aquí concatenar la fecha y hora
		//strcat(command,"_$(date +%d%m%Y_%H_%M_%S)");
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
    }else if(pipe(pfd4) < 0){
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
			close(pfd4[0]);

     		printf("\nHijo(pid=%i) esperando mensaje de mi padre...\n", getpid());
			read(pfd[0],msg,SIZE);
			printf("\nHijo(pid=%i), instruccion del padre: %s\n", getpid(), msg);
			
			write(pfd2[1],"<--- Hola padre, ¿de cuantos archivos sera el respaldo?...",SIZE);
			
			read(pfd3[0],msg,SIZE);
			total = atoi(msg);
			//read(pfd3[0],msg,SIZE)
			contador = total;
			while(contador > 0){
				read(pfd3[0],msg,SIZE);
				if (msg == "FIN"){
					break;
				}else{
					printf("\tRealizando el respaldo de %s\tArchivos pendientes: %d/%d\n", msg,contador--,total);
					strcpy(command,"");
					strcat(command,"cp ");
					strcat(command,dirR); 
					strcat(command,"/");
					strcat(command,msg);
					strcat(command," ");
					strcat(command,dirW);
					//printf("%s\n", command);
					system(command);
				}
			}
			printf("\nArchivos leidos con exito\n"); // ENVÍAR MENSAJE AL PADRE

			write(pfd4[1],"Termine de hacer el respaldo", SIZE);

			close(pfd[0]);
			close(pfd2[1]);
			close(pfd3[0]);
			close(pfd4[1]);
			exit(0);
        break;

        default:
          	close(pfd[0]);
			close(pfd2[1]);
			close(pfd3[0]);
			close(pfd4[1]);

			write(pfd[1],"Realiza el siguiente respaldo",SIZE);

			read(pfd2[0], buffer, SIZE);
			printf("\nPadre(pid=%i), lee mensaje del hijo: %s\n", getpid(), buffer);
			
			file = fopen("test.txt","r");
			if(file == NULL){
				perror("\nError al abrir el archivo");
			}else{
				char palabra[30];
				//Contabiliza los archivos a respaldar
				while(!feof(file)){
					fscanf(file,"%s ",palabra);
					lineanum++;
				}
				printf("\nNumero de archivos a respaldar: %d\n", lineanum);
				//Rebobina el apuntador para volver a leer el archivo
				rewind(file);
				sprintf(lineas,"%d",lineanum);
				write(pfd3[1],lineas,SIZE);
				for (int i = 0; i < lineanum; ++i){
					fgets (linea, sizeof linea, file);
					strtok(linea,"\n");
					write(pfd3[1],linea,SIZE);
				}
			}

			write(pfd3[1],"FIN",SIZE);
			//(Validar que haya terminado el hijo)
			if(fclose(file) != 0){
				printf("Error al cerrar el directorio de respaldo.\n");
				exit(0);
			}
			
			read(pfd4[0], buffer, SIZE);

			printf("\nPadre(pid=%i), lee mensaje del hijo: %s\n", getpid(),buffer);
			printf("\nPadre(pid=%i), comprobando respaldo:\n", getpid());
			//Lista el directorio de respaldo (Validar que haya terminado el hijo)
			strcpy(command,"");
			strcat(command,"ls -l ");
			strcat(command,dirW);
			system(command);
			strcpy(command,"");
			strcat(command,"ls -l ");
			strcat(command,dirW);
			strcat(command," | wc -l");
			system(command);
			printf("ARCHIVOS RESPALDADOS\n");
			printf("========================================================================\n");
			
			close(pfd[1]);
			close(pfd2[0]);
			close(pfd3[1]);
			close(pfd4[0]);
		break;
	}
	wait(&status);
	printf("\nTermino el proceso padre...\n");
	return 0;
}