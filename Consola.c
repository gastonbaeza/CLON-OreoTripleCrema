
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
#include <stddef.h> 
#include <signal.h> 
#include <linux/limits.h>
#define BACKLOG 5
#define KERNEL 0
	#define ARRAYPIDS 51
	#define PIDFINALIZACION 2
	#define PATH 10
#define MEMORIA 1
	#define SOLICITUDMEMORIA 0
	#define RESULTADOINICIARPROGRAMA 23
	#define SOLICITUDINFOPROG 1
	#define ESCRIBIRMEMORIA 2
	#define LIBERARMEMORIA 3
 	#define ACTUALIZARPCB 4
#define CONSOLA 2
	#define INICIARPROGRAMA 0
	#define FINALIZARPROGRAMA 88
	#define DESCONECTARCONSOLA 2
	#define LIMPIARMENSAJES 3
	//------------------------------	
	#define MENSAJE 7
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
int idKernel;
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

int receptorListo=0;
FILE *consolaLog;
char * horaActual;
char* nombreLog;
int len;

void strip(char** string){
	int i ;
	for(i=0;i<string_length(*string); i++){
		if((*string)[i]==' ' || (*string)[i]=='/')
			(*string)[i]='-';
	}
}

void cortar(){
	printf("adasdasd\n");
	fclose(consolaLog);
	exit(0);
}

void escribirEnArchivoLog(char * contenidoAEscribir, FILE ** archivoDeLog,char * direccionDelArchivo){
	
	fseek(*archivoDeLog,0,SEEK_END);
	len=ftell(*archivoDeLog);
	fwrite(contenidoAEscribir,strlen(contenidoAEscribir),1,*archivoDeLog);
	fwrite("\n",1,1,*archivoDeLog);
	}

void programa(t_path * path_ansisop){ 
	struct addrinfo hints;
	struct addrinfo *kernel;
	void * unBuffer=malloc(sizeof(int));
	int primer=0;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int  serverSocket;
	int rv; 
	if ((rv =getaddrinfo(IP, PUERTO, &hints, &kernel)) != 0) fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
	serverSocket = socket(kernel->ai_family, kernel->ai_socktype, kernel->ai_protocol);
	if(-1==connect(serverSocket, kernel->ai_addr, kernel->ai_addrlen)) perror("connect:");
	handshakeCliente(serverSocket,CONSOLA,&unBuffer);
	send(serverSocket,&primer,sizeof(int),0);
	enviarDinamico(PATH,serverSocket,(void*)path_ansisop);
	escribirEnArchivoLog("envio path", &consolaLog,nombreLog);
	send(serverSocket,&idKernel,sizeof(int),0);
	free(path_ansisop->path);
	free(path_ansisop);
							
	
	freeaddrinfo(kernel);
	
	
	
}


void receptor(){
	struct addrinfo hints;
	struct addrinfo *kernel;
	int primer=1;
	void * unBuffer=malloc(sizeof(int));
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int  serverSocket;
	int rv; 
	if ((rv =getaddrinfo(IP, PUERTO, &hints, &kernel)) != 0) fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
	serverSocket = socket(kernel->ai_family, kernel->ai_socktype, kernel->ai_protocol);
	if(-1==connect(serverSocket, kernel->ai_addr, kernel->ai_addrlen)) perror("connect:");
	handshakeCliente(serverSocket,CONSOLA,&unBuffer);
	send(serverSocket,&primer,sizeof(int),0);
	while(0>recv(serverSocket,&idKernel, sizeof(int),0));
	receptorListo=1;
t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
t_mensaje * unMensaje;
t_resultadoIniciarPrograma * resultado;
while(1) {
	resultado=malloc(sizeof(t_resultadoIniciarPrograma));
	while(0>recv(serverSocket,seleccionador, sizeof(t_seleccionador),0));
switch (seleccionador->tipoPaquete){
		case MENSAJE:	
						unMensaje=malloc(sizeof(t_mensaje));
						escribirEnArchivoLog("en mensaje", &consolaLog,nombreLog);
						unMensaje->mensaje=calloc(1,1);
						recibirDinamico(MENSAJE, serverSocket, unMensaje);
						
						printf("%s.\n",unMensaje->mensaje);
		break;

		case RESULTADOINICIARPROGRAMA:
					recibirDinamico(RESULTADOINICIARPROGRAMA,serverSocket, resultado);

					if (resultado->resultado){
						pthread_mutex_lock(&semaforoProcesos);
						list_add(procesos,(void *)(&(resultado->pid)));
						printf("El pid asignado es : %i\n", resultado->pid);
						pthread_mutex_unlock(&semaforoProcesos);
						escribirEnArchivoLog("en resultado reiniciar programa", &consolaLog,nombreLog);
						
					}
		break;
		default: ; break;


}}

}





int main(){

	//////////////////////////////////////// LEER CONFIGURACION
	
	t_config *CFG;
	procesos=list_create();
	CFG = config_create("consolaCFG.txt");
	IP_KERNEL= config_get_string_value(CFG ,"IP_KERNEL");
	PUERTO_KERNEL= config_get_string_value(CFG ,"PUERTO_KERNEL");
	printf("Configuración:\nIP_KERNEL = %s,\nPUERTO_KERNEL = %s.\n",IP_KERNEL,PUERTO_KERNEL);
	char * nombreLog=calloc(1,200);
	strcpy(nombreLog,"logConsola-");

	horaActual=calloc(1,40);
	horaYFechaActual(&horaActual);
	strip(&horaActual);
	printf("horaActual: %s\n", horaActual);
	strcat(horaActual,".txt");
	strcat(nombreLog,horaActual);
	printf("nombreLog: %s\n", nombreLog);
	////////////////////////////////////////

	pthread_t  hiloPrograma,hiloReceptor;
	int rv;
	int primer=0;
	IP=malloc(13);
	PUERTO=malloc(10);
	strcpy(IP,IP_KERNEL);
	strcpy(PUERTO,PUERTO_KERNEL);
	pthread_create(&hiloReceptor,NULL,(void*)receptor,NULL);
	while(!receptorListo);
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;	
	hints.ai_socktype = SOCK_STREAM;
	fflush(stdout);

	if ((rv =getaddrinfo(IP_KERNEL, PUERTO_KERNEL, &hints, &serverInfo)) != 0) fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	
	config_destroy(CFG);
	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	if(-1==connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)) perror("connect:");
	freeaddrinfo(serverInfo);
	void* buffer=malloc(sizeof(int));
	handshakeCliente(serverSocket,CONSOLA,&buffer);
	send(serverSocket,&primer,sizeof(int),0);
	consolaLog=fopen(nombreLog,"w+");
	perror("asd");
	printf("consolaLog%p.\n",consolaLog);
pthread_mutex_init(&semaforoProcesos,NULL);
int cancelarThread=0;
int unPid;
int * PID;
PID=malloc(sizeof(int));
t_path * path_ansisop;
char * path;
signal(SIGINT,cortar);

	int * instruccionConsola=malloc(sizeof(int));
	t_programaSalida * prog;

while(cancelarThread==0){
	
	printf("Seleccione una de las siguientes opciones ingresando el número correspondiente.\n");
	printf("%i %s\n",INICIARPROGRAMA,"-Iniciar programa.");
	printf("%i %s\n",DESCONECTARCONSOLA,"-Desconectar consola.");
	printf("%i %s\n",LIMPIARMENSAJES,"-Limpiar los mensajes.");
	printf("%i %s\n",FINALIZARPROGRAMA,"-Finalizar programa.");
	
	

	
	scanf("%d",instruccionConsola);
	switch(*instruccionConsola){
	case INICIARPROGRAMA: //Recibe el path del ansisop y lo envia al kernel
							escribirEnArchivoLog("en iniciar programa", &consolaLog,nombreLog);
							path_ansisop=malloc(sizeof(t_path));
							path_ansisop->path=calloc(1,150);
							path=calloc(1,150);
							printf ("path: \n");
							scanf("%s",path);
							 //puede pasar que lo que esriba en una consola me afecte esto? jaja seria malo.
							prog= obtenerPrograma(path);
							path_ansisop->path=prog->elPrograma;
							path_ansisop->tamanio=prog->tamanio;
							pthread_create(&hiloPrograma, NULL, (void *) programa, path_ansisop);/*
							escribirEnArchivoLog("escribir1",&consolaLog,"consolaLog.txt");
							escribirEnArchivoLog("escribir2",&consolaLog,"consolaLog.txt");*/
							

							free(path);
							
							
		break;
	case FINALIZARPROGRAMA:
							//Recibe el pid del proceso a matar y lo envia al kernel
							escribirEnArchivoLog("en finalizar programa", &consolaLog,nombreLog);
							printf ("PID: \n");
							scanf("%d", &unPid);
							
							
							enviarDinamico(FINALIZARPROGRAMA,serverSocket,&unPid);
							escribirEnArchivoLog("envio finalizar programa", &consolaLog,nombreLog);	
	
		break;
	case DESCONECTARCONSOLA:
							//Recibo los PIDs que Kernel fue guardando y se los mando para que los mate
							pthread_mutex_lock(&semaforoProcesos);
							escribirEnArchivoLog("en desconectar consola", &consolaLog,nombreLog);
							printf("%s\n"," ha inicializado el proceso de desconexion" );
							printf("%i\n",list_size(procesos));
							t_arrayPids * pids=malloc(sizeof(t_arrayPids));
							
							pids->cantidad=list_size(procesos);
						
							pids->pids=malloc(list_size(procesos)*sizeof(int));
							
							for(unPid=0;unPid<list_size(procesos);unPid++)
							{
								
								pids->pids[unPid]=*(int *)list_get(procesos,unPid);

							}
							
							enviarDinamico(ARRAYPIDS,serverSocket,(void *)pids); 
							escribirEnArchivoLog("envio arrays pids", &consolaLog,nombreLog);
							printf("sali\n");
							free(pids->pids);
							free(pids);

							pthread_mutex_unlock(&semaforoProcesos);
							cancelarThread=1;
	
		break;
	case LIMPIARMENSAJES:
							clear();
		break;

	default:	printf("%s\n","habia una veez un barco chicquito");//error no se declara
	
	}
}
	printf("%s\n",mensajeFinalizacionHilo);
	fclose(consolaLog);
	return 0;
}
	
	
	
	

