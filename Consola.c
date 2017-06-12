
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
	#define PATH 10
	#define RESULTADOINICIARPROGRAMA 6
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
	#define MENSAJE 0
	#define PIDNUEVO 1
#define PATH 10
#define CPU 3
#define FS 4
#define TRUE 1
#define FALSE 0
#define OK 1
#define FAIL 0
#define BLOQUE 20

t_list * procesos;

pthread_mutex_t semaforoProcesos;
pthread_mutex_t mutexComunicacion;
pthread_mutex_t mutexfreeDataConexion;
int PUEDOLIBERAR=2;
char * mensajeError="la instruccion solicitada no existe, gracias vuelvas prontos";
char * mensajeFinalizacionHilo="el hilo no esta, el hilo se fue, el hilo se escapa de mi vida";
char *IP_KERNEL;
char * PUERTO_KERNEL;
char* IP;
char* PUERTO;

void programa(t_path * path_ansisop){
	struct addrinfo hints;
	struct addrinfo *kernel;
	int * unBuffer=malloc(sizeof(int));
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int  serverSocket;
	int rv; 
	if ((rv =getaddrinfo(IP, PUERTO, &hints, &kernel)) != 0) fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
	serverSocket = socket(kernel->ai_family, kernel->ai_socktype, kernel->ai_protocol);

	if(-1==connect(serverSocket, kernel->ai_addr, kernel->ai_addrlen)) perror("connect:");
	handshakeCliente(serverSocket,2,unBuffer);
	
	
	freeaddrinfo(kernel);
	
	
	
void * paquete;
int recibir;
t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
t_mensaje * unMensaje;
int * unPid;
t_resultadoIniciarPrograma * resultado;
while(1) {
	while(0>recv(serverSocket,seleccionador, sizeof(t_seleccionador),0)){;}

switch (seleccionador->tipoPaquete){
		case MENSAJE:	
						unMensaje=malloc(sizeof(t_mensaje));
						unMensaje->mensaje=malloc(1);
						pthread_mutex_lock(&mutexComunicacion);
						recibirDinamico(MENSAJE, socket, unMensaje);
						pthread_mutex_unlock(&mutexComunicacion);
						printf("%s",unMensaje->mensaje);
		break;

		case RESULTADOINICIARPROGRAMA:
					pthread_mutex_lock(&mutexComunicacion);
					recibirDinamico(RESULTADOINICIARPROGRAMA,socket, paquete);
					pthread_mutex_unlock(&mutexComunicacion);
					 resultado=malloc(sizeof(t_resultadoIniciarPrograma));
					memcpy(resultado,paquete,sizeof(t_resultadoIniciarPrograma));
					if (resultado->resultado){
						pthread_mutex_lock(&semaforoProcesos);
						list_add(procesos,(void *)(resultado->pid));
						pthread_mutex_unlock(&semaforoProcesos);
					}
		break;
		default: ; break;


}}}



int main(){

	//////////////////////////////////////// LEER CONFIGURACION

	t_config *CFG;
	procesos=list_create();
	CFG = config_create("consolaCFG.txt");
	IP_KERNEL= config_get_string_value(CFG ,"IP_KERNEL");
	PUERTO_KERNEL= config_get_string_value(CFG ,"PUERTO_KERNEL");
	printf("Configuración:\nIP_KERNEL = %s,\nPUERTO_KERNEL = %s.\n",IP_KERNEL,PUERTO_KERNEL);
	
	printf("Presione enter para continuar.\n");
	getchar();

	////////////////////////////////////////

	pthread_t  hiloPrograma;
	int rv;
IP=malloc(13);
PUERTO=malloc(10);
strcpy(IP,IP_KERNEL);
strcpy(PUERTO,PUERTO_KERNEL);
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;	
	hints.ai_socktype = SOCK_STREAM;
	fflush(stdout);

	printf("Configuración:\nIP_KERNEL = %s,\nPUERTO_KERNEL = %s.\n",IP_KERNEL,PUERTO_KERNEL);
	if ((rv =getaddrinfo(IP_KERNEL, PUERTO_KERNEL, &hints, &serverInfo)) != 0) fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	
	config_destroy(CFG);
	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	if(-1==connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)) perror("connect:");
	freeaddrinfo(serverInfo);
	int * buffer=malloc(sizeof(int));
	handshakeCliente(serverSocket,CONSOLA,buffer);
	

pthread_mutex_init(&semaforoProcesos,NULL);
int cancelarThread=0;
int PIDS[list_size(procesos)];
int unPid;
int * PID;
PID=malloc(sizeof(int));
t_path * path_ansisop;


while(cancelarThread==0){
	
	printf("Ingrese una instruccion\n");
	

	int * instruccionConsola=malloc(sizeof(int));
	
	scanf("%d",instruccionConsola);
	switch(*instruccionConsola){
	case INICIARPROGRAMA: //Recibe el path del ansisop y lo envia al kernel
							path_ansisop=malloc(sizeof(t_path));
							path_ansisop->path=malloc(150*sizeof(char));
							printf ("path: \n");
							scanf("%s",path_ansisop->path);
							 //puede pasar que lo que esriba en una consola me afecte esto? jaja seria malo.
							path_ansisop->tamanio=strlen(path_ansisop->path)+1;
							path_ansisop->path=realloc(path_ansisop->path,(path_ansisop->tamanio)*sizeof(char));
							fflush(stdout);printf("este es el code%s\n",path_ansisop->path ); getchar();
							enviarDinamico(PATH,serverSocket,(void*)path_ansisop);
							fflush(stdout);printf("este es el code%s\n",path_ansisop->path );
							pthread_create(&hiloPrograma, NULL, (void *) programa, path_ansisop);
							
							
		break;
	case FINALIZARPROGRAMA:
							//Recibe el pid del proceso a matar y lo envia al kernel
							
							printf ("PID: ");
							scanf("%d", PID);
							pthread_mutex_lock(&mutexComunicacion);
							enviarDinamico(FINALIZARPROGRAMA,socket,PID);
							pthread_mutex_unlock(&mutexComunicacion);
	
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
							pthread_mutex_lock(&mutexComunicacion);
							enviarDinamico(ARRAYPIDS,socket,(void *)procesosAMatar); 
							pthread_mutex_unlock(&mutexComunicacion);
							pthread_mutex_unlock(&semaforoProcesos);
							cancelarThread ++;
	
		break;
	case LIMPIARMENSAJES:
							clear();
		break;

	default:	printf("%s\n",mensajeError);//error no se declara
	
	}
}
	printf("%s\n",mensajeFinalizacionHilo);
	return 0;
}
	
	
	
	

