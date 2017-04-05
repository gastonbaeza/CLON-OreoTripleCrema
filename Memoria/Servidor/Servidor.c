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

#define MAXCOLA 10 //numero de conexiones en espera en cola permitidas
void sigchld_handler(int s){ //orphan collector
        while(wait(NULL) > 0);
}
int main (void){

	/*
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	LECTURA DE LOS ARCHIVOS DE CONFIGURACION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	*/
	FILE *cfg = fopen ("memoriaCFG.txt", "r");
 	int PUERTO,MARCOS,MARCO_SIZE,ENTRADAS_CACHE,CACHE_X_PROC,RETARDO_MEMORIA;
 	char *REEMPLAZO_CACHE;
 	fscanf(cfg, "PUERTO=%i\n",&PUERTO);
 	fscanf(cfg, "MARCOS=%i\n",&MARCOS);
 	fscanf(cfg, "MARCO_SIZE=%i\n",&MARCO_SIZE);
 	fscanf(cfg, "ENTRADAS_CACHE=%i\n",&ENTRADAS_CACHE);
 	fscanf(cfg, "CACHE_X_PROC=%i\n",&CACHE_X_PROC);
 	fscanf(cfg, "REEMPLAZO_CACHE=%s\n",REEMPLAZO_CACHE);
 	fscanf(cfg, "RETARDO_MEMORIA=%i",&RETARDO_MEMORIA);
 	fclose(cfg);
 	/*
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	*/
	//struct sigaction sa; // esto es para el orphans collector
	int socketListen,socketNuevo;
	int tamanioStruct = sizeof(struct sockaddr_in);
	struct sockaddr_in miDireccion, direccionEntrante;

	if((socketListen=socket(AF_INET,SOCK_STREAM,0))==-1){  //dominio=AF_INET tipo=SOCK_STREAM protocolo=0 (predeterminado)
    	perror("socket");
	    exit(1);
	}
	miDireccion.sin_family = AF_INET;         	// Ordenación de máquina
	miDireccion.sin_port = htons(PUERTO);     // short, Ordenación de la red
	miDireccion.sin_addr.s_addr = INADDR_ANY; 	// nuestra IP
	memset(&(miDireccion.sin_zero), '\0', 8); 	// Poner a cero el resto de la estructura

	if ((bind(socketListen, (struct sockaddr *)&miDireccion, tamanioStruct))== -1) { //asociar socket a puerto
    	perror("bind");
    	exit(1);
    }
	if ((listen(socketListen, MAXCOLA)) == -1) { 	// abrir cola de espera de conexiones entrantes
    	perror("listen");
    	exit(1);
    }
/*sa.sa_handler = sigchld_handler; // Eliminar procesos muertos
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;
if (sigaction(SIGCHLD, &sa, NULL) == -1) 
				{
            			perror("sigaction");
            			exit(1);
				}*/
    while(1){
		if ((socketNuevo = accept(socketListen, (struct sockaddr *)&direccionEntrante, &tamanioStruct)) == -1) { // crear nuevo socket para la conexion entrante
			perror("accept");
    		continue;}
    	printf("server: got connection from %s\n", inet_ntoa(direccionEntrante.sin_addr));
        
     
    }


}
