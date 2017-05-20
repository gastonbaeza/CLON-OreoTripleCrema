

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
t_marco * asignadorSecuencial;
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
void * contigua=malloc(MARCOS*MARCO_SIZE);
bloquesAdmin=calloc(MARCOS*sizeof(t_estructuraADM),MARCOS*sizeof(t_estructuraADM));
memcpy(contigua,bloquesAdmin,MARCOS*sizeof(t_estructuraADM)); //defini 500 bloques por las dudas, parece que cada adm es para un proceso aunque puedo equivocarme y que sea por cada marco
tamanioAdministrativas=MARCOS*sizeof(t_estructuraADM);
contigua=realloc(contigua,tamanioAdministrativas);
 marcos=malloc(MARCOS-tamanioAdministrativas);
int unMarco=0;
for(unMarco;unMarco<(MARCOS-tamanioAdministrativas); unMarco++) //asignar su numero de marco a cada region de memoria
	{ 	int unaPagina=0;
		while(unaPagina<3)
		{if(unaPagina==0){
		marcos[unMarco].marco=unMarco;
	 	marcos[unMarco].numeroPagina[unaPagina]=&contigua+tamanioAdministrativas;
		
						}
		else
		{ 
		marcos[unMarco].numeroPagina[unaPagina]= &marcos[unMarco].numeroPagina[unaPagina-1]+ ((MARCO_SIZE/4));
		}

		marcos[unMarco].numeroPagina[unaPagina]=calloc(MARCO_SIZE/4,MARCO_SIZE/4);// pone un cero cada 2 bytes o algo asi seguro rompe esto jaja
		unaPagina++;
		}

	}
	
	asignadorSecuencial=&marcos[0];
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
t_header * header=malloc(sizeof(t_header));
t_solicitudMemoria * solicitudMemoria;
t_list * paginasParaUsar;
t_solicitudInfoProg * solicitudInfo;
t_pcb * unPcb;
t_actualizacion * actualizacion;
int paginasRequeridas;
int stackRequeridas;
while(1) {
	recibir=recv(unData,header, sizeof(t_header),0);
	paquete=malloc(header->tamanio);
	if(header->seleccionador.unaInterfaz==MEMORIA){
	switch (header->seleccionador.tipoPaquete){
		case SOLICITUDMEMORIA: // [Identificador del Programa] // paginas necesarias para guardar el programa y el stack
							recibirDinamico(unData,paquete);
							t_solicitudMemoria * solicitudMemoria;
							solicitudMemoria=malloc(header->tamanio);
 							memcpy(solicitudMemoria,paquete,header->tamanio);
 							if(((void *)asignadorSecuencial== marcos[MARCOS-tamanioAdministrativas-1].numeroPagina[3] )&& list_is_empty(paginasLiberadas)) 
 							// y ahora olvidemos nuestros problemas con un gran platon de helado de vainilla, *no me juzguen por este if*
 							{ solicitudMemoria->respuesta=FAIL;
 								enviarDinamico(KERNEL,RESPUESTAOKMEMORIA,socketKernel, (void *) solicitudMemoria, header->tamanio);}

 							else
 							{ //mandarOK memoria
 							// codigo del programa
 							recibirDinamico(unData,paquete);
							
							solicitudMemoria=malloc(header->tamanio);
 							memcpy(solicitudMemoria,paquete,header->tamanio);
 							char * codigo=malloc(solicitudMemoria->codigo.tamanio);
 							memcpy(codigo,solicitudMemoria->codigo.elPrograma,solicitudMemoria->codigo.tamanio);
 							paginasRequeridas=solicitudMemoria->cantidadPaginasCodigo;
 							stackRequeridas=solicitudMemoria->cantidadPaginasStack;
 							//se cuantas paginas usa, tengo que partir el codigo en funcion de la cantidad de paginas, hacer un for con la cantidad de paginas, y buscar pagina, eso y agregar el chain.
 								
 							
 							
 							
 							list_create(paginasParaUsar);
 							/*Deje el algoritmo extra sin codear en buscar paginas*/
 							buscarPaginas((paginasRequeridas+stackRequeridas),paginasParaUsar, asignadorSecuencial,marcos, MARCOS,tamanioAdministrativas);
 							cargarPaginas(paginasParaUsar,stackRequeridas, codigo,tamPagina);
 							free(codigo);
 							free(solicitudMemoria);
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
}//if
}//while	





void consolaMemoria()
{
	int cancelarThread=0;
	while(cancelarThread==0)
	{
	int * instruccionConsola;
	scanf("%d",instruccionConsola);
	switch(*instruccionConsola){

case RETARDO: 
							
							
		break;
	case DUMP:
							
							
							
	
		break;
	case FLUSH:
							
	
		break;
	case SIZE:
							
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
			{	tamPagina=MARCO_SIZE/4;
				memcpy(unBuffer,&tamPagina, sizeof(int));
				send(socketNuevaConexion, unBuffer,sizeof(int),0);
				socketKernel=socketNuevaConexion;	
			}
		dataNuevo->socket=socketNuevaConexion;
		pthread_create(&hiloComunicador,NULL,(void *) comunicarse,dataNuevo);
	}
}
}

/*

fflush(stdout);
listen(listenningSocket, BACKLOG);
struct sockaddr_in addr;
FD_ZERO(&fdParaConectar);
FD_SET(listenningSocket, &fdParaConectar);

fdMayor = listenningSocket;
for(;;) {
	fdParaLeer = fdParaConectar;
	resultadoSelect=select(fdMayor+1, &fdParaLeer, NULL, NULL, NULL);
	if ( resultadoSelect== -1){
		perror("Error en el select.\n");
		exit(1);
	}
	for(unSocket = 0; unSocket <= fdMayor; unSocket++) { // Busca en las conexiones alguna que mande un request
	    if (FD_ISSET(unSocket, &fdParaLeer)) {  //si hay alguna con request...
	    	if (unSocket == listenningSocket) { // se manejan las nuevas conexiones a partir de los SETS
	    		addrlen = sizeof(addr);
	    		if ((socketNuevaConexion = accept(listenningSocket, (struct sockaddr *)&addr,&addrlen)) == -1) perror("accept");
	    		else {
	    			dataParaComunicarse dataParaThread;
	    			dataParaThread.socket=socketNuevaConexion;
					FD_SET(dataParaThread.socket, fdParaConectar);
					printf("Hay una nueva conexion de %s en el socket %i.\n", inet_ntoa(address.sin_addr), dataParaThread.socket);
					send(dataParaThread.socket, bienvenida, 100*sizeof(char), 0);
					recv(dataParaThread.socket, unaInterfaz, sizeof(int),0);
					dataParaThread.unaInterfaz = unaInterfaz;
	    			pthread_t hilo;
	    			pthread_create(&hilo,NULL,(void *)comunicarse,&dataParaThread); //pasar por parametro
	    			if (socketNuevaConexion > fdMayor) fdMayor = socketNuevaConexion;
	    			}
	    	}
	    	else { // tramito los request del cliente
	    		if ((nbytes = recv(unSocket, package, 100*sizeof(char), 0)) <= 0) {
	    			if (nbytes == 0) printf("El cliente %i se ha desconectado.\n", unSocket);
	    			else perror("Error en el recv.\n"); 
	    			fflush(stdout); 
	    			printf("Hubo algun tipo de error.\n");
	    			close(unSocket);
	    			FD_CLR(unSocket, &fdParaConectar); // lo saco de los pendientes de conexion
				}
	    		else { // tenemos datos de algún cliente
	    			printf("%s\n",package );
	    			for(otroSocket = 0; otroSocket <= fdMayor; otroSocket++) { // ESTO HAY QUE CAMBIARLO PARA LOS PROTOCOLOS; SINO ES UN ECHO MULTICLIENTE
	    				if (FD_ISSET(otroSocket, &fdParaConectar)) {
							if (otroSocket != listenningSocket && otroSocket != unSocket) {
	    						if (send(otroSocket, package, 100*sizeof(char), 0) == -1) perror("Error en el send.");
							}
	    				}
	    			}
	    		}
	    	}
	    }
	}
}*/
