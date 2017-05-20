
#include "funciones.h"
#include "estructuras.h"
#include <commons/collections/list.h>
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

#define CPU 3
#define FS 4
#define TRUE 1
#define FALSE 0
#define OK 1
#define FAIL 0
#define BLOQUE 20

t_list * procesos;

pthread_mutex_t semaforoProcesos;
char * mensajeError="la instruccion solicitada no existe, gracias vuelvas prontos";
char * mensajeFinalizacionHilo="el hilo no esta, el hilo se fue, el hilo se escapa de mi vida";



void recibir(int socket){
void * paquete;
int recibir;
t_header * header=malloc(sizeof(t_header));
char * unMensaje;
int * unPid;
while(1) {
	recibir=recv(socket,header, sizeof(t_header),0);
	paquete=malloc(header->tamanio);
switch (header->seleccionador.tipoPaquete){
		case MENSAJES:
						recibirDinamico(socket, paquete, header->tamanio);
						unMensaje=malloc(header->tamanio);
						printf("%s",unMensaje);
		break;

		case PIDNUEVO:
					recibirDinamico(socket, paquete, header->tamanio);
					 unPid=malloc(sizeof(int));
					memcpy(unPid,paquete,sizeof(int));
					pthread_mutex_lock(&semaforoProcesos);
					list_add(procesos,(void *)unPid);
					pthread_mutex_unlock(&semaforoProcesos);
		break;


}}}

void enviar(int socket){
int cancelarThread=0;
int PIDS[list_size(procesos)];
int unPid;
int * PID;
char path_ansisop[100];
while(cancelarThread==0)
	{int * instruccionConsola;
	scanf("%d",instruccionConsola);
switch(*instruccionConsola){
	case INICIARPROGRAMA: //Recibe el path del ansisop y lo envia al kernel
							
							printf ("path: ");
							fgets(path_ansisop, 100, stdin); //puede pasar que lo que esriba en una consola me afecte esto? jaja seria malo.
							enviarDinamico(KERNEL,PATH,socket,(void *)path_ansisop,100* sizeof(char));
		break;
	case FINALIZARPROGRAMA:
							//Recibe el pid del proceso a matar y lo envia al kernel
							
							printf ("PID: ");
							scanf("%d", PID);
							enviarDinamico(KERNEL,PIDFINALIZACION,socket,(void *)PID,sizeof(int));
	
		break;
	case DESCONECTARCONSOLA:
							//Recibo los PIDs que Kernel fue guardando y se los mando para que los mate
							//FALTA finalizar la conexión de todos los threads de la consola con el kernel -----!?!?!?!?!?!?
							pthread_mutex_lock(&semaforoProcesos);
							
							for(unPid=0;unPid<list_size(procesos);unPid++)
							{
								PIDS[unPid]=*(int *)list_get(procesos,unPid);

							}
							int * procesosAMatar;
							procesosAMatar=&PIDS[0];
							enviarDinamico(KERNEL,ARRAYPIDS,socket,(void *)procesosAMatar,list_size(procesos)*sizeof(int)); 
							pthread_mutex_unlock(&semaforoProcesos);
							cancelarThread ++;
	
		break;
	case LIMPIARMENSAJES:
							clear();
		break;

	default:	printf("%s",mensajeError);//error no se declara
	
	}
	printf("%s",mensajeFinalizacionHilo);
}
}
int main(){

	//////////////////////////////////////// LEER CONFIGURACION

	t_config *CFG;
	list_create(procesos);
	CFG = config_create("consolaCFG.txt");
	char *IP_KERNEL= config_get_string_value(CFG ,"IP_KERNEL");
	char *PUERTO_KERNEL= config_get_string_value(CFG ,"PUERTO_KERNEL");
	printf("Configuración:\nIP_KERNEL = %s,\nPUERTO_KERNEL = %s.\n",IP_KERNEL,PUERTO_KERNEL);
	
	printf("Presione enter para continuar.\n");
	getchar();

	////////////////////////////////////////
	pthread_mutex_init(&semaforoProcesos,NULL);
	int bytesRecibidos;
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(IP_KERNEL,PUERTO_KERNEL,&hints,&serverInfo);

	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);


	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);

	freeaddrinfo(serverInfo);

	config_destroy(CFG);

	pthread_t hiloEnviar, hiloRecibir;
	pthread_create(&hiloEnviar, NULL, (void *) enviar, &serverSocket);
	pthread_create(&hiloRecibir, NULL, (void *) recibir, &serverSocket);
	pthread_join(hiloEnviar,NULL); // cuando el hilo enviar termina (el usuario pone quit por ej) se termina el proceso consola
	pthread_join(hiloRecibir,NULL);//tiene que ser 2 argumentos

	
	return 0;
}
