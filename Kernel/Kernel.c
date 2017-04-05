#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define MAXCOLA 10

void iniciarServidor(int*);

void sigchld_handler(int s){
        while(wait(NULL) > 0);
    }
int main(void){

	/*
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	LECTURA DE LOS ARCHIVOS DE CONFIGURACION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	*/
	FILE *cfg = fopen ("/home/utnso/tp-2017-1c-Oreo-Triple-Crema/Kernel/kernelCFG.txt", "r");
 	int PUERTO_PROG,PUERTO_CPU,IP_MEMORIA,PUERTO_MEMORIA,IP_FS,PUERTO_FS,QUANTUM,QUANTUM_SLEEP,GRADO_MULTIPROG,STACK_SIZE,*SEM_INIT;
 	char *ALGORITMO,**SEM_IDS,**SHARED_VARS;
 	fscanf(cfg, "PUERTO_PROG=%i\n",&PUERTO_PROG);
 	fscanf(cfg, "PUERTO_CPU=%i\n",&PUERTO_CPU);
 	fscanf(cfg, "IP_MEMORIA=%i\n",&IP_MEMORIA);
 	fscanf(cfg, "PUERTO_MEMORIA=%i\n",&PUERTO_MEMORIA);
 	fscanf(cfg, "IP_FS=%i\n",&IP_FS);
 	fscanf(cfg, "PUERTO_FS=%i\n",&PUERTO_FS);
 	fscanf(cfg, "QUANTUM=%i\n",&QUANTUM);
 	fscanf(cfg, "QUANTUM_SLEEP=%i\n",&QUANTUM_SLEEP);
 	fscanf(cfg, "ALGORITMO=%s\n",ALGORITMO);
 	fscanf(cfg, "GRADO_MULTIPROG=%i\n",&GRADO_MULTIPROG);

 	fclose(cfg);
 	/*
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	*/
	
}
void iniciarServidor(int *MIPUERTO){
struct sigaction sa;
int socketListen,socketNuevo;
int tamanioStruct = sizeof(struct sockaddr_in);
struct sockaddr_in miDireccion, direccionEntrante;

if((socketListen=socket(AF_INET,SOCK_STREAM,0))==-1)
								{  //dominio=AF_INET tipo=SOCK_STREAM protocolo=0 (predeterminado)
	   							perror("socket");
	   							exit(1);
								}
miDireccion.sin_family = AF_INET;         	// Ordenación de máquina
miDireccion.sin_port = htons(*MIPUERTO);     // short, Ordenación de la red
miDireccion.sin_addr.s_addr = INADDR_ANY; 	// nuestra IP
memset(&(miDireccion.sin_zero), '\0', 8); 	// Poner a cero el resto de la estructura

if ((bind(socketListen, (struct sockaddr *)&miDireccion, tamanioStruct))== -1)
								{ //asociar socket a puerto
	    						perror("bind");
	    						exit(1);
								}
if ((listen(socketListen, MAXCOLA)) == -1)
								{
								// abrir cola de espera de conexiones entrantes
	    						perror("listen");
	    						exit(1);
	    						}

sa.sa_handler = sigchld_handler; // Eliminar procesos muertos
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;
if (sigaction(SIGCHLD, &sa, NULL) == -1) {
            							perror("sigaction");
            							exit(1);
        								 }
while(1)
			{
			if ((socketNuevo = accept(socketListen, (struct sockaddr *)&direccionEntrante, &tamanioStruct)) == -1)
								{ // crear nuevo socket para la conexion entrante
								perror("accept");
								continue;
								}
	    	printf("server: got connection from %s\n", inet_ntoa(direccionEntrante.sin_addr));


			}
if (!fork()) {
				// Este es el proceso hijo
                close(socketListen); // El hijo no necesita este descriptor
                if (send(socketNuevo, "Hello, world!\n", 14, 0) == -1)
                		{
                    	perror("send");
                    	close(socketNuevo);
                    	exit(0);
                		}
               close(socketNuevo);  // El proceso padre no lo necesita
        	}



	}



