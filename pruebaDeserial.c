#include <commons/collections/list.h>
#include "estructuras.h"
#include "funciones.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/txt.h>
#include <errno.h>
#define clear() printf("\033[H\033[J")
#include <sys/time.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stddef.h>
#include <math.h>
#include <stdint.h>
#include <commons/config.h>
#define BACKLOG 5
#define KERNEL 0
	#define ARRAYPIDS 5
	#define PIDFINALIZACION 2
	#define PATH 3
#define MEMORIA 1
	#define SOLICITUDMEMORIA 0
	#define SOLICITUDINFOPROG 1
	#define ESCRIBIRMEMORIA 2
	#define LIBERARMEMORIA 3
 	#define ACTUALIZARPCB 4
#define CONSOLA 2
	#define INICIARPROGRAMA 0
	#define FINALIZARPROGRAMA 1
	#define DESCONECTARCONSOLA 2
	#define LIMPIARMENSAJES 3
	//------------------------------	
	#define MENSAJES 0
	#define PIDNUEVO 1
	#define PCB 10

#define CPU 3
#define FS 4
#define TRUE 1
#define FALSE 0
#define OK 1
#define FAIL 0
#define BLOQUE 20




int main()
{	
	int * interfaz;
	interfaz = malloc(sizeof(int));
	int socketNuevaConexion;
	int socketlisten;
	t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
	
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *serverInfo;
	int * buffer;
	getaddrinfo(NULL, "5005", &hints, &serverInfo);
	
	
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	socketlisten = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	if((bind(socketlisten,serverInfo->ai_addr, serverInfo->ai_addrlen))==-1) {fflush(stdout);printf("error bind");}
	
	listen(socketlisten,BACKLOG);
	socketNuevaConexion = accept(socketlisten, (struct sockaddr *)&addr,&addrlen);
	handshakeServer(socketNuevaConexion,1,interfaz);
	printf("asd\n");
	while(1){
		while(0>=recv(socketNuevaConexion, seleccionador, sizeof(t_seleccionador), 0)){}
		
		//////////////////////////////////////////
		/* DEFINIR TIPO DE ESTRUCTURA A RECIBIR */
		t_pcb * paquete=malloc(sizeof(t_pcb));
		//////////////////////////////////////////
		
		switch(seleccionador->unaInterfaz){
			case 0:
			switch(seleccionador->tipoPaquete){
				case 0:	

						///////////////////////////////////////////////////////////////////////////////////////////		
						/* RECIBIR (NO OLVIDAR DEFINIR EL TIPO DE PAQUETE EN EL PRECOMPILADOR) Y MOSTRAR PAQUETE */
						recibirDinamico(PCB,socketNuevaConexion,paquete);	
						fflush(stdout);printf("Valores recibidos:\n");
						fflush(stdout);printf("%i\n",paquete->pid);
						fflush(stdout);printf("%i\n",paquete->programCounter);
						fflush(stdout);printf("%i\n",paquete->estado);
						fflush(stdout);printf("%i\n",paquete->referenciaATabla);
						fflush(stdout);printf("%i\n",paquete->paginasCodigo);
						fflush(stdout);printf("%i\n",paquete->posicionStack);
						fflush(stdout);printf("%i\n",paquete->indiceCodigo);
						fflush(stdout);printf("%i\n",paquete->indiceEtiquetas);
						fflush(stdout);printf("%i\n",paquete->exitCode);
						///////////////////////////////////////////////////////////////////////////////////////////
							
				break;


			}
        	break;
    	}	
		free(paquete);
	
	}
	free(seleccionador);
}