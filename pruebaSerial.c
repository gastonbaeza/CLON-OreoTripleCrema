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

struct addrinfo hints;
struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
if((getaddrinfo("127.0.0.1","5005",&hints,&serverInfo))==-1){printf("Error getaddrinfo\n");}


	int  serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	
	if(-1==connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)) { perror("connect"); }
	int * unServer;
	unServer=malloc(sizeof(int));
	handshakeCliente(serverSocket,2,unServer);


	//////////////////////////////////////////////
	/* DECLARAR Y RELLENAR ESTRUCTURA A TESTEAR */
	t_pcb * paquete;
	paquete=malloc(sizeof(t_pcb));
	paquete->pid=1;
	paquete->programCounter=8;
	paquete->estado=2;
	paquete->referenciaATabla=10;
	paquete->paginasCodigo=7;
	paquete->posicionStack=123;
	paquete->indiceCodigo=4;
	paquete->indiceEtiquetas=5;
	paquete->exitCode=-5;
	//////////////////////////////////////////////

	
t_seleccionador * seleccionador=malloc(3*sizeof(int));
seleccionador->unaInterfaz=0;
seleccionador->tipoPaquete=0;

send(serverSocket, seleccionador, sizeof(t_seleccionador),0); 


	////////////////////////////////////////////////////////////
	/* NO OLVIDAR DEFINIR EL TIPO DE PAQUETE AL PRECOMPILADOR */
	enviarDinamico(PCB,serverSocket,paquete);
	////////////////////////////////////////////////////////////

	free(paquete);
	return 0;	
}