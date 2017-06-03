

// #include "desSerializador.h"
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
#define clear() printf("\033[H\033[J")
#define BACKLOG 5
#define LIBRE 0
#define OCUPADO 1
#define KERNEL 0
	#define ARRAYPIDS 5
	#define PIDFINALIZACION 2
	#define PATH 3
	#define PIDINFO 4
	#define RESPUESTAOKMEMORIA 1
#define MEMORIA 1
	#define SOLICITUDMEMORIA 0
	#define SOLICITUDINFOPROG 1
	#define ESCRIBIRMEMORIA 2
	#define LIBERARMEMORIA 3
 	#define ACTUALIZARPCB 4
	
	//-------------------------------
	#define RETARDO 0
	#define DUMP 1
	#define FLUSH 2
	#define SIZE 3
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
t_estructuraADM * bloquesAdmin;
int tamPagina;
t_list * paginasLiberadas;
t_marco * asignador;
int hayQueCambiarDeAlgoritmo=0;
unsigned int tamanioAdministrativas;
char * mensajeError="la instruccion solicitada no existe, gracias vuelvas prontos";
char * mensajeFinalizacionHilo="el hilo no esta, el hilo se fue, el hilo se escapa de mi vida";
char *IP_KERNEL;
char *PUERTO_KERNEL;
char *PUERTO;
int MARCOS;
int MARCO_SIZE;
int ENTRADAS_CACHE;
int CACHE_X_PROC;
char *REEMPLAZO_CACHE;
int RETARDO_MEMORIA;
t_marco * marcos;
int socketKernel;

/* LEER CONFIGURACION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
t_config *CFG;
/* LEER CONFIGURACION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
void consolaMemoria();
void comunicarse(dataParaComunicarse * estructura);
void aceptar(dataParaComunicarse * unData);
int main(){

CFG = config_create("memoriaCFG.txt");
IP_KERNEL= config_get_string_value(CFG ,"IP_KERNEL");
PUERTO_KERNEL= config_get_string_value(CFG ,"PUERTO_KERNEL");
PUERTO= config_get_string_value(CFG ,"PUERTO");
MARCOS= config_get_int_value(CFG ,"MARCOS");
MARCO_SIZE= config_get_int_value(CFG ,"MARCO_SIZE");
ENTRADAS_CACHE= config_get_int_value(CFG ,"ENTRADAS_CACHE");
CACHE_X_PROC= config_get_int_value(CFG ,"CACHE_X_PROC");
REEMPLAZO_CACHE= config_get_string_value(CFG ,"REEMPLAZO_CACHE");
RETARDO_MEMORIA= config_get_int_value(CFG ,"RETARDO_MEMORIA");
printf("Configuración:\nPUERTO = %s,\nIP_KERNEL = %s,\nPUERTO_KERNEL = %s,\nMARCOS = %i,\nMARCO_SIZE = %i,\nENTRADAS_CACHE = %i,\nCACHE_X_PROC = %i,\nREEMPLAZO_CACHE = %s,\nRETARDO_MEMORIA = %i.\n"
		,PUERTO,IP_KERNEL,PUERTO_KERNEL,MARCOS,MARCO_SIZE,ENTRADAS_CACHE,CACHE_X_PROC,REEMPLAZO_CACHE,RETARDO_MEMORIA);


int tamanioFrame = 1024 * 1024;
void * frame = malloc(tamanioFrame);
char * bienvenida="Bienvenido, soy la memoria. \n";
int nbytes;
fd_set fdParaConectar, fdParaLeer;
char package[100];
int fdMayor;
int a,listenningSocket,rv,iof,i;
struct addrinfo hints;
struct addrinfo *serverInfo;

for(i=0;i<100;i++){
	package[i]='\0';
}

memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_flags = AI_PASSIVE;
hints.ai_socktype = SOCK_STREAM;

if ((rv =getaddrinfo(NULL, PUERTO, &hints, &serverInfo)) != 0) fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));

fflush(stdout);
printf("%s \n", "El Servidor esta configurado.\n");
sleep(1);
listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
sleep(1);
fflush(stdout);
printf("%s \n", "Socket Listo.");

if(bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen)==-1) {perror("Error en el bind."); exit(1);}

fflush(stdout);
sleep(1);
printf("%s \n", "Bind Listo.\n");

freeaddrinfo(serverInfo);
sleep(1);
fflush(stdout);
config_destroy(CFG);
clear();
printf("%s \n", "El Servidor se encuentra listo para escuchar conexiones.");
pthread_t hiloAceptador;
dataParaComunicarse *unData;
unData=malloc(sizeof(dataParaComunicarse));
unData->socket = listenningSocket;

 ////////////////////////////////////// INICIALIZAMOS EL BLOQUE DE MEMORIA/////////////////////////////////////

bloquesAdmin=malloc(MARCOS*sizeof(t_estructuraADM));
 tamanioAdministrativas=MARCOS*sizeof(t_estructuraADM);
int unaAdmin;
for (unaAdmin = 0; unaAdmin < MARCOS; unaAdmin++) //inicializo los bloques de admin con pid-1 libre y num de pag y el hash
{
bloquesAdmin[unaAdmin].estado=LIBRE;
bloquesAdmin[unaAdmin].pid=-1;
bloquesAdmin[unaAdmin].frame=unaAdmin;
bloquesAdmin[unaAdmin].hashPagina=miFuncionDeHash(bloquesAdmin[unaAdmin].pid,bloquesAdmin[unaAdmin].frame);
	
}
bloquesAdmin=realloc(bloquesAdmin,((tamanioAdministrativas*t_estructuraADM)+(MARCOS*MARCO_SIZE)));
marcos=bloquesAdmin+tamanioAdministrativas;
int unMarco=1;
void * memoria=calloc(MARCOS*MARCO_SIZE,MARCOS*MARCO_SIZE); // marcos es un bloque de punteros a void porque el tipo void en c no existe y aca quiero pegar lo que me de la gana.
for(unMarco;unMarco<MARCOS; unMarco++) //asignar su numero de marco a cada region de memoria
	{ 
		marcos[unMarco].numeroPagina[unaPagina]=memoria+(unMarco*MARCO_SIZE);
		unMarco++;
	}
	
	asignador=&marcos[0];
////////////////////////////////INICIAMOS HILOS DE COMINICACION/////////////////////////////////////////////



pthread_create(&hiloAceptador,NULL,(void *)aceptar,unData);
pthread_t hiloConsolaMemoria;
pthread_create(&hiloConsolaMemoria,NULL,(void *)consolaMemoria,NULL);
pthread_join(hiloAceptador,NULL);
pthread_join(hiloConsolaMemoria,NULL);
}



void comunicarse(dataParaComunicarse * estructura){ // aca tenemos que agregar toda la wea que es "global"

 int unData=estructura->socket;
printf("%i \n", unData);

void * paquete;
int recibir;
t_seleccionador * seleccionador;
t_list * paginasParaUsar;
t_solicitudInfoProg * solicitudInfo;
t_pcb * unPcb;
t_actualizacion * actualizacion;
int paginasRequeridas;
int stackRequeridas;
while(1) {
	while(0>recv(unData,seleccionador,sizeof(t_seleccionador),0));
	
	switch (seleccionador->tipoPaquete){
		case SOLICITUDMEMORIA: // [Identificador del Programa] // paginas necesarias para guardar el programa y el stack
							recibirDinamico(unData,paquete);
							
							t_solicitudMemoria * solicitudMemoria=(t_solicitudMemoria *)paquete; //esto lo vi en stack overflow no me peguen
 							
 							paginasRequeridas=solicitudMemoria->cantidadPaginasCodigo;
 							stackRequeridas=solicitudMemoria->cantidadPaginasStack;
 							if(hayPaginasLibres(paginasRequeridas+stackRequeridas,bloquesAdmin,MARCOS)==FAIL) 
 							{ 
 							solicitudMemoria->respuesta=FAIL;
 							enviarDinamico(KERNEL,RESPUESTAOKMEMORIA,socketKernel, (void *) solicitudMemoria, header->tamanio);
 							}

 							else
 							{ //mandarOK memoria
 							solicitudMemoria->respuesta=OK;
 							// codigo del programa
 							enviarDinamico(KERNEL,RESPUESTAOKMEMORIA,socketKernel, (void *) solicitudMemoria, header->tamanio);
 							char * codigo=malloc(solicitudMemoria->codigo.tamanio);
 							memcpy(codigo,solicitudMemoria->codigo.elPrograma,solicitudMemoria->codigo.tamanio);
 							
 							//se cuantas paginas usa, tengo que partir el codigo en funcion de la cantidad de paginas, hacer un for con la cantidad de paginas, y buscar pagina, eso y agregar el chain.
 								
 							
 							
 							
 							paginasParaUsar=list_create();
 							/*Deje el algoritmo extra sin codear en buscar paginas*/
 							buscarPaginas((paginasRequeridas+stackRequeridas),paginasParaUsar,MARCOS);
 							cargarPaginas(paginasParaUsar,stackRequeridas, codigo, MARCO_SIZE);
 							free(codigo);
 							free(solicitudMemoria);
 							free(paginasParaUsar);
 							//enviarDinamico notificacion de que se asigno genialmente

 							 }	
 							
 							
 							
 							

 					break;
 		 
 					
 		case SOLICITUDINFOPROG:// informacion del programa en ejecucion (memoria)
							 recibirDinamico(unData,paquete);		
							 
							 unPcb=malloc(sizeof(t_pcb));
							 solicitudInfo= malloc(sizeof(header->tamanio));
							 memcpy(solicitudInfo,paquete,header->tamanio);
							 buscarAdministrativa(solicitudInfo, unPcb,bloquesAdmin,MARCOS);	
							 enviarDinamico(KERNEL,PIDINFO,socketKernel,(void *)unPcb,sizeof(t_pcb));
							 free(paquete);
							 free(solicitudInfo);		
 					break;
 		case ESCRIBIRMEMORIA:
 							recibirDinamico(unData,paquete);
 					break;
 		case LIBERARMEMORIA:
 							 recibirDinamico(unData,paquete);
 									
 					break;
 		case ACTUALIZARPCB:
 								recibirDinamico(unData,paquete);
 								
 								actualizacion=malloc(header->tamanio);
 								memcpy(actualizacion,paquete,header->tamanio);

 		break;
 				}
}//switch

}//while	





void consolaMemoria()
{	int pid;
	
	
	int cancelarThread=0;
	while(cancelarThread==0)
	{
	int * instruccionConsola;
	scanf("%d",instruccionConsola);
	switch(*instruccionConsola){

case RETARDO: 
						float retardo;
						fflush(stdout);printf("%s", "ingrese retardo (en milisegundos)");
						scanf("%d",&retardo);	
						retardo=retardo*0.001;
		break;
	case DUMP:		fflush(stdout);printf("%s", "seleccione objeto de dump"); //preguntar si en disco quiere decir en filesystem o en disco posta
					switch()
					{
						case CACHE:generarDumpCache(memoriaCache,ENTRADAS_CACHE);
						break;
						case MEMORIA:generarDumpMemoria(asignador,MARCOS);
						break;
						case TABLA:generarDumpAdministrativas(bloquesAdmin, MARCOS);	
						break;
						case PID:		
										scanf("%d",&pid);
										generarDumpProceso(pid)
						break;
						default: pagaraprata();
					}
							
							
	
		break;
	case FLUSH:
					memset(memoria,0,ENTRADAS_CACHE);		
	
		break;
	case SIZE:			
			switch()
			{
				case MEMORIA:	int cantidadBloquesOcupados, cantidadBloquesLibres;
								fflush(stdout);printf("cantidadDePaginas %i", MARCOS);
								cantidadBloquesLibre=cantidadBloquesLibres();
								fflush(stdout);printf("cantidadBloquesLibres%i", cantidadBloquesLibres);
								cantidadBloquesOcupados=cantidadBloquesOcupados();
								fflush(stdout);printf("cantidadBloquesOcupados%i", cantidadBloquesOcupados);
				break;
				case PID:
				break;
				default: pagaraprata();
			}	
							
		break;

	default:	printf("%s",mensajeError);//error no se declara
	
	}
	printf("%s",mensajeFinalizacionHilo);
}
}




void aceptar(dataParaComunicarse * unData){
dataParaComunicarse * dataNuevo;
void * unBuffer=malloc(sizeof(int));
dataNuevo = malloc(sizeof(dataParaComunicarse));
int socketNuevaConexion,rv;
pthread_t hiloComunicador;
struct sockaddr_in addr;
int addrlen = sizeof(addr);
if(rv=listen(unData->socket,BACKLOG)==-1) perror("Error en el listen");
printf("%i %i\n",unData->socket,rv);
while(1){
	if ((socketNuevaConexion = accept(unData->socket, (struct sockaddr *)&addr,&addrlen)) == -1) perror("Error con conexion entrante");
	else {
		memcpy(dataNuevo,unData,sizeof(dataParaComunicarse));
		handshakeServer(socketNuevaConexion,MEMORIA,unBuffer);
		if((int *)unBuffer==KERNEL)
			{	tamPagina=MARCO_SIZE;
				memcpy(unBuffer,&tamPagina, sizeof(int));
				send(socketNuevaConexion, unBuffer,sizeof(int),0);
				socketKernel=socketNuevaConexion;	
			}
		dataNuevo->socket=socketNuevaConexion;
		pthread_create(&hiloComunicador,NULL,(void *) comunicarse,dataNuevo);
	}
}
}

