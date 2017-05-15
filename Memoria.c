#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <commons/config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/txt.h>
#include <errno.h>
#define clear() printf("\033[H\033[J")
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <commons/config.h>
#include <math.h>
// #include "desSerializador.h"
#include "../Kernel/estructuras.h"
#define clear() printf("\033[H\033[J")
#define BACKLOG 5


void comunicarse(dataParaComunicarse * unData){
printf("%i \n", unData->socket);
void * paquete;
int recibir;
t_header * header;
while(1) {
	recibir=recv(unData->socket,header, sizeof(t_header),0);
	if(header->seleccionador.unaInterfaz==MEMORIA){
	switch (header->seleccionador.tipoPaquete){
		case SOLICITUDMEMORIA: // [Identificador del Programa] // paginas necesarias para guardar el programa y el stack
							recibirDinamico(unSocket,paquete);
							t_solicitudMemoria * solicitudMemoria;
							solicitudMemoria=malloc(header->tamanio);
 							memcpy(solicitudMemoria,paquete,header->tamanio);
 							if(&asignadorSecuencial== &marcos[MARCOS-tamanioAdministrativas-1].numeroPagina[3] && list_is_empty(paginasLiberadas)) 
 							// y ahora olvidemos nuestros problemas con un gran platon de helado de vainilla, *no me juzguen por este if*
 							{ //NO HAY MEMORIA LOCO DEJEN DE METER PROGRAMAS ANSISOP}

 							else
 							{//esto es lo mas comun que pase yo te voy a dar paginar para el codigo, y para tu stack de forma contigua
 							//uso para algo el codigo en memoria???
 								char * codigo=malloc(solicitudMemoria->codigo.tamanio)
 								memcpy(codigo,solicitudMemoria->codigo.elPrograma,solicitudMemoria->codigo.tamanio);
 								paginasRequeridas=solicitudMemoria->cantidadPaginasCodigo;
 								//se cuantas paginas usa, tengo que partir el codigo en funcion de la cantidad de paginas, hacer un for con la cantidad de paginas, y buscar pagina, eso y agregar el chain.
 								
 								t_list * paginasParaUsar;
 								buscarPaginas(paginasParaUsar);

 								int unaPagina;
 								for ( unaPagina = 0; unaPagina < paginasRequeridas; unaPagina++)
 								{
 									if(unaPagina==0)
 									{
 									
 									t_chain * chainPrograma=malloc(tamPagina);
 									memcpy(chainPrograma->codigo,codigo,(tamPagina-sizeof(unsigned int)));
 									chainPrograma->chain=&list_get(paginasParaUsar,unaPagina+1);
 									}
 										//hasta aca arme el primer bloque del chain, me falta desplazarme en el codigo original, repetir
 										//como detecto hasta donde copiar en el ultimo tramo?
 								}

 							 }	
 							
 							
 							
 							

 					break;
 		case CODIGO: // codigo del programa
 							 recibirDinamico(unSocket,paquete);
 							t_programaSalida * codigoPrograma;
 							codigoPrograma=malloc(header->tamanio);
 							memcpy(codigoPrograma,paquete,header->tamanio);
 							pegarEnLaPaginaDeMemoria();
 					break;
 		case SOLICITUDINFOPROG:// informacion del programa en ejecucion (memoria)
							 recibirDinamico(unSocket,paquete);					
 					break;
 		case ESCRIBIRMEMORIA:
 							recibirDinamico(unSocket,paquete);
 					break;
 		case LIBERARMEMORIA:
 							 recibirDinamico(unSocket,paquete);
 									
 					break;
 				}
}
}
char * consolaMemoria(){}
void * unBuffer=malloc(sizeof(int));
void aceptar(dataParaComunicarse * unData){
dataParaComunicarse * dataNuevo;
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
		if(*unBuffer==KERNEL)
			{	int tamPagina=MARCOS_SIZE/4;
				memcpy(unBuffer,&tamPagina, sizeof(int));
				send(socketNuevaConexion, unBuffer,sizeof(int),0);	
			}
		dataNuevo->socket=socketNuevaConexion;
		pthread_create(&hiloComunicador,NULL,(void *) comunicarse,dataNuevo);
	}
}
}

int main(){

/* LEER CONFIGURACION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
t_config *CFG;
CFG = config_create("memoriaCFG.txt");
char *IP_KERNEL= config_get_string_value(CFG ,"IP_KERNEL");
char *PUERTO_KERNEL= config_get_string_value(CFG ,"PUERTO_KERNEL");
char *PUERTO= config_get_string_value(CFG ,"PUERTO");
int MARCOS= config_get_int_value(CFG ,"MARCOS");
int MARCO_SIZE= config_get_int_value(CFG ,"MARCO_SIZE");
int ENTRADAS_CACHE= config_get_int_value(CFG ,"ENTRADAS_CACHE");
int CACHE_X_PROC= config_get_int_value(CFG ,"CACHE_X_PROC");
char *REEMPLAZO_CACHE= config_get_string_value(CFG ,"REEMPLAZO_CACHE");
int RETARDO_MEMORIA= config_get_int_value(CFG ,"RETARDO_MEMORIA");
printf("Configuración:\nPUERTO = %s,\nIP_KERNEL = %s,\nPUERTO_KERNEL = %s,\nMARCOS = %i,\nMARCO_SIZE = %i,\nENTRADAS_CACHE = %i,\nCACHE_X_PROC = %i,\nREEMPLAZO_CACHE = %s,\nRETARDO_MEMORIA = %i.\n"
		,PUERTO,IP_KERNEL,PUERTO_KERNEL,MARCOS,MARCO_SIZE,ENTRADAS_CACHE,CACHE_X_PROC,REEMPLAZO_CACHE,RETARDO_MEMORIA);
/* LEER CONFIGURACION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/

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
void * contigua=malloc(MARCOS*MARCOS_SIZE);
t_estructuraADM * bloquesAdmin=malloc (MARCOS*sizeof(t_estructuraADM));
memcpy(contigua,bloquesAdmin,MARCOS*sizeof(t_estructuraADM)); //defini 500 bloques por las dudas, parece que cada adm es para un proceso aunque puedo equivocarme y que sea por cada marco
unsigned int tamanioAdministrativas=MARCOS*sizeof(t_estructuraADM);
contigua=realloc(contigua,tamanioAdministrativas);
t_marco  marcos[MARCOS-tamanioAdministrativas];
unMarco=0;
for(unMarco;unMarco<(MARCOS-tamanioAdministrativas); unMarco++) //asignar su numero de marco a cada region de memoria
	{ 	unaPagina=0;
		while(unaPagina<3)
		{if(unaPagina==0){
		marcos[unMarco].numeroMarco=unMarco;
	 	marcos[unMarco].numeroPagina[unaPagina]=&contigua+tamanioAdministrativas;
		
						}
		else
		{ 
		marcos[unMarco].numeroPagina[unaPagina]= &marcos[unMarco].numeroPagina[unaPagina-1]+ ((MARCOS_SIZE/4));
		}

		marcos[unMarco].numeroPagina[unaPagina]=calloc(MARCOS_SIZE/4,MARCOS_SIZE/4);// pone un cero cada 2 bytes o algo asi seguro rompe esto jaja
		unaPagina++
		}

	}
	t_marco * asignadorSecuencial;
	
	asignadorSecuencial=marcos[0];
////////////////////////////////INICIAMOS HILOS DE COMINICACION/////////////////////////////////////////////


pthread_create(&hiloAceptador,NULL,(void *)aceptar,unData);
pthread_join(hiloAceptador,NULL);











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
return 0;
}