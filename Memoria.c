


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

	#define SOLICITUDMEMORIA 0
	#define SOLICITUDINFOPROG 1
	#define ESCRIBIRMEMORIA 2
	#define LIBERARMEMORIA 3
 	#define ACTUALIZARPCB 4
	#define SOLICITUDBYTES 31
	#define LINEA 19
	#define ALMACENARBYTES 36
	//-------------------------------
	#define DELAY 0
	#define DUMP 1
	#define FLUSH 2
	#define SIZE 3
#define CONSOLA 2
	#define INICIARPROGRAMA 0
	#define FINALIZARPROGRAMA 104
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

#define CACHE 0
#define MEMORIA 1 
#define TABLA 2
#define PID 3
#define PCB 3
#define PIDSIZE 1
#define MEMORIASIZE 0
int retardo;
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
void * memoria;
t_marco * marcos;
t_estructuraCache* memoriaCache;
int socketKernel;
void * memoria;
void * cache;
t_list** overflow;
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

bloquesAdmin=malloc(MARCOS*sizeof(t_estructuraADM)+MARCOS*sizeof(t_marco));
 tamanioAdministrativas=MARCOS*sizeof(t_estructuraADM);
iniciarOverlow(MARCOS);
int unaAdmin;
for (unaAdmin = 0; unaAdmin < MARCOS; unaAdmin++) //inicializo los bloques de admin con pid-1 libre y num de pag y el hash
{
bloquesAdmin[unaAdmin].estado=LIBRE;
bloquesAdmin[unaAdmin].pid=-1;
bloquesAdmin[unaAdmin].frame=unaAdmin;
bloquesAdmin[unaAdmin].pagina=-1;
	
}
 
marcos=(t_marco*)(bloquesAdmin+tamanioAdministrativas);
int unMarco=0;

 memoria=calloc(MARCOS*MARCO_SIZE,MARCOS*MARCO_SIZE);
 cache=calloc(ENTRADAS_CACHE,MARCO_SIZE);
memoriaCache=(t_estructuraCache*)cache;
 // marcos es un bloque de punteros a void porque el tipo void en c no existe y aca quiero pegar lo que me de la gana.
for(unMarco;unMarco<MARCOS; unMarco++) //asignar su numero de marco a cada region de memoria
	{ 
		marcos[unMarco].numeroPagina=memoria+(unMarco*MARCO_SIZE);
		marcos[unMarco].estado=0;
	}

for(unMarco=0;unMarco<ENTRADAS_CACHE; unMarco++) //asignar su numero de marco a cada region de memoria
	{
		memoriaCache[unMarco].contenido=cache+(unMarco*MARCO_SIZE);
		memoriaCache[unMarco].frame=-unMarco; //para distinguir  una pagina real de una virgen
		memoriaCache[unMarco].pid=-1;
		memoriaCache[unMarco].antiguedad=0;
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
fflush(stdout); 
 int unData=estructura->socket;


int recibir;
t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
int entrada;
t_pcb * unPcb;
void * paquete;
t_actualizacion * actualizacion;
int paginasRequeridas;
int stackRequeridas;

while(1) {
	paquete=malloc(peticionBytes->size);
	t_peticionBytes * peticionBytes=malloc(sizeof(t_peticionBytes));
	t_almacenarBytes * bytesAAlmacenar=malloc(sizeof(t_almacenarBytes));
	t_solicitudMemoria * solicitud=malloc(sizeof(t_solicitudMemoria));
	while(0>recv(unData,seleccionador,sizeof(t_seleccionador),0));
	
	switch (seleccionador->tipoPaquete){
		case SOLICITUDMEMORIA: // [Identificador del Programa] // paginas necesarias para guardar el programa y el stack
							 //esto lo vi en stack overflow no me peguen
							solicitud->codigo=calloc(1,solicitud->tamanioCodigo+sizeof(char));
							recibirDinamico(SOLICITUDMEMORIA,unData,solicitud);
							
							fflush(stdout);printf("Tamaño: %i\n", solicitud->tamanioCodigo);
							printf("Codigo: %s\n", solicitud->codigo); 
							printf("Cant Pags Codigo: %i\n", solicitud->cantidadPaginasCodigo);
							printf("Cant Pags Stack: %i\n", solicitud->cantidadPaginasStack);
							printf("PID: %i\n", solicitud->pid);
							
 							
 							paginasRequeridas=solicitud->cantidadPaginasCodigo;
 							stackRequeridas=solicitud->cantidadPaginasStack;
 							printf("hay x canatidad paginas libres %i\n",hayPaginasLibres(paginasRequeridas+stackRequeridas,bloquesAdmin,MARCOS));
 							if(hayPaginasLibres(paginasRequeridas+stackRequeridas,bloquesAdmin,MARCOS)==FAIL) 
 							{ 
 							solicitud->respuesta=FAIL;
 							enviarDinamico(SOLICITUDMEMORIA,socketKernel, (void *) solicitud);
 							}

 							else
 							{ //mandarOK memoria
 							solicitud->respuesta=OK;
 						
 							enviarDinamico(SOLICITUDMEMORIA,socketKernel, (void *) solicitud);
 							printf("Despues de enviar\n");
 							char * codigo=malloc(solicitud->tamanioCodigo);
 							memcpy(codigo,solicitud->codigo,solicitud->tamanioCodigo);
 							
 							
 							buscarPaginas((paginasRequeridas+stackRequeridas),MARCOS,bloquesAdmin,marcos,solicitud->pid);
 							cargarPaginas(paginasParaUsar,stackRequeridas, codigo, MARCO_SIZE);
 							
 							free(codigo);
 							free(solicitud);
 							
 							
 							
 							 }	
 							
 							
 							
 							

 					break;
 		 
 					case SOLICITUDBYTES:
 										recibirDinamico(SOLICITUDBYTES,unData,peticionBytes);
 										
 										if((entrada=estaEnCache(peticionBytes->pid,peticionBytes->pagina,memoriaCache,ENTRADAS_CACHE))!=-1)
 										{//lo busco en cache
 											
 											memcpy(paquete,memoriaCache[entrada].contenido+peticionBytes->offset,peticionBytes->size);
 											
 											memoriaCache[entrada].antiguedad=0;
 											

 										}
 										else
 										{//lo busco en memoria
 											entrada=buscarPagina(peticionBytes->pid,peticionBytes->pagina,marcos,MARCOS);
 											memcpy(paquete,marcos[entrada].numeroPagina+peticionBytes->offset,peticionBytes->size);
 											escribirEnCache(peticionBytes->pid,peticionBytes->pagina,marcos[entrada].numeroPagina,memoriaCache,ENTRADAS_CACHE,0,MARCO_SIZE);
 											//uso escribirEnCache para guardar una pagina entera en cache que esta en memoria
 										}	
 											
 											send(unData,paquete,peticionBytes->size,0);
 											free(paquete);
 					break;
 		/*case SOLICITUDINFOPROG:// informacion del programa en ejecucion (memoria)
							 recibirDinamico(SOLICITUDINFOPROG,unData,paquete);		
							 
							 unPcb=(t_pcb *)paquete;
							 
							 buscarAdministrativa(unPcb->pid, unPcb,bloquesAdmin,MARCOS);	
							 enviarDinamico(PIDINFO,socketKernel,(void *)unPcb);
							 free(paquete);
							 free(unPcb);*/
							 	
 					break;
 		case ALMACENARBYTES:	recibirDinamico(ALMACENARBYTES,unData,bytesAAlmacenar);
 								if((entrada=estaEnCache(bytesAAlmacenar->pid,bytesAAlmacenar->pagina,memoriaCache,ENTRADAS_CACHE))!=-1);
								
								else
 								{//lo busco en memoria
								entrada=buscarPagina(bytesAAlmacenar->pid,bytesAAlmacenar->pagina,marcos,MARCOS);
																	
								}	
								almacenarBytes(bytesAAlmacenar->pid,bytesAAlmacenar->pagina,(void*)&bytesAAlmacenar->valor, marcos,MARCOS, bytesAAlmacenar->offset,bytesAAlmacenar->size);
								escribirEnCache(bytesAAlmacenar->pid,bytesAAlmacenar->pagina,marcos[entrada].numeroPagina,memoriaCache,ENTRADAS_CACHE,0,MARCO_SIZE);
 								
 					break;
 		case LIBERARMEMORIA:
 							 
 					break;
 		/*case ACTUALIZARPCB:
 								recibirDinamico(PCB,unData,paquete);
 								unPcb=(t_pcb *)paquete;
 								free(paquete);
								free(unPcb);

 		break;*/
 				}
}//switch

}//while	





void consolaMemoria()
{	printf("%s\n", "bienvenido a la consola de memoria");
	int pid;
	int unMarco;
	
	int cancelarThread=0;
	while(cancelarThread==0)
	{ clear();
	int * instruccionConsola=malloc(sizeof(int));
	int bloquesOcupados, bloquesLibre;
	printf("%s\n","**********************");
	printf("%s\n","ingrese 0 para modificar el retardo de acceso de memoria" );
	printf("%s\n","**********************");
	printf("%s\n","ingrese 1 para realizar acciones de tipo dump" );
	printf("%s\n","**********************");
	printf("%s\n", "ingrese 2 para realizar un flush de cache");
	printf("%s\n","**********************");
	printf("%s\n", "ingrese 3 para obtener informacion acerca del tamaño");
	printf("%s\n","**********************");
	
	scanf("%d",instruccionConsola);
	switch(*instruccionConsola){

	case DELAY: 		
						clear();
						printf("%s\n","Ingrese el nuevo valor de retardo: " );
						getchar();
						scanf("%d",&retardo);
						
							
						retardo=retardo;
						printf("El nuevo retardo ha sido actualizado a: %i %s\n", retardo,"milisegundos" );printf("%s\n", "presione una tecla para volver al menu de la consola");getchar(); getchar();
		break;
	case DUMP:		clear();
					fflush(stdout);printf("%s\n", "Para realizar un dump de cache ingrese 0"); 
					fflush(stdout);printf("%s\n", "Para realizar un dump de memoria ingrese 1"); 
					fflush(stdout);printf("%s\n", "Para realizar un dump de administrativas ingrese 2"); 
					fflush(stdout);printf("%s\n", "Para realizar un dump de un proceso ingrese 3"); 



					scanf("%d",instruccionConsola);//preguntar si en disco quiere decir en filesystem o en disco posta
					switch(*instruccionConsola)
					{
						case CACHE:
									clear();
									generarDumpCache(memoriaCache,ENTRADAS_CACHE,MARCO_SIZE);printf("%s\n", "presione una tecla para volver al menu de la consola");getchar();getchar();
						break;
						case MEMORIA:
										clear();
										generarDumpMemoria(asignador,MARCOS);printf("%s\n", "presione una tecla para volver al menu de la consola");getchar();getchar();
						break;
						case TABLA:
										clear();
										generarDumpAdministrativas(bloquesAdmin, MARCOS);	printf("%s\n", "presione una tecla para volver al menu de la consola");getchar();getchar();
						break;
						case PID:		clear();
										printf("%s\n","ingrese un pid para realizar dump" );
										getchar();
										scanf("%d",&pid);
										printf("%i",pid);
										generarDumpProceso(bloquesAdmin,MARCOS,pid,marcos); printf("%s\n", "presione una tecla para volver al menu de la consola");getchar();getchar();
						break;
						default: pagaraprata();
						break;
					}
							
							
	
		break;
	case FLUSH:	clear();
					
					for(unMarco=0;unMarco<ENTRADAS_CACHE; unMarco++) //asignar su numero de marco a cada region de memoria
					{
					memoriaCache[unMarco].contenido=cache+(unMarco*MARCO_SIZE);
					memoriaCache[unMarco].frame=-unMarco; //para distinguir  una pagina real de una virgen
					memoriaCache[unMarco].pid=-1;
					}	printf("%s\n","la memoria cache ha sido reinicializada" );	
						printf("%s\n", "presione una tecla para volver al menu de la consola");getchar(); getchar();
		break;
	case SIZE:	clear();		fflush(stdout);printf("%s\n", "para conocer el tamaño de la memoria ingrese 0");

								fflush(stdout);printf("%s\n", "para conocer el tamaño de un proceso ingrese 1");getchar();
			 					scanf("%d",instruccionConsola);
			switch(*instruccionConsola)
			{
				case MEMORIASIZE:	clear();
								fflush(stdout);printf("cantidadDePaginas: %i\n", MARCOS);
								bloquesLibre=cantidadBloquesLibres(MARCOS,bloquesAdmin);
								fflush(stdout);printf("cantidadBloquesLibres: %i\n", bloquesLibre);
								bloquesOcupados=cantidadBloquesOcupados(MARCOS,bloquesAdmin);
								fflush(stdout);printf("cantidadBloquesOcupados: %i\n", bloquesOcupados);
								printf("%s\n", "presione una tecla para volver al menu de la consola");
								fflush(stdout);
								getchar();getchar();
				break;
				case PIDSIZE:clear(); printf("%s\n","ingrese un pid" );
				getchar(); scanf("%d",&pid);
				 calcularTamanioProceso(pid,bloquesAdmin,MARCOS);printf("%s\n", "presione una tecla para volver al menu de la consola"); getchar();getchar();
				break;
				default: pagaraprata();
				break;
			}	
							
		break;

	default:	
	break;
	}
	}
}




void aceptar(dataParaComunicarse * unData){
dataParaComunicarse * dataNuevo;
int * unBuffer=malloc(sizeof(int));
dataNuevo = malloc(sizeof(dataParaComunicarse));
int socketNuevaConexion,rv;
pthread_t hiloComunicador;
struct sockaddr_in addr;
int addrlen = sizeof(addr);
if(rv=listen(unData->socket,BACKLOG)==-1) perror("Error en el listen");
while(1){
	if ((socketNuevaConexion = accept(unData->socket, (struct sockaddr *)&addr,&addrlen)) == -1) perror("Error con conexion entrante");
	else {
		memcpy(dataNuevo,unData,sizeof(dataParaComunicarse));
		handshakeServer(socketNuevaConexion,MEMORIA,(void*)unBuffer);
		
		if(*unBuffer==KERNEL)
			{	tamPagina=MARCO_SIZE;
				printf("MARCO SIZE: %i\n",tamPagina );
				memcpy(unBuffer,&tamPagina, sizeof(int));
				send(socketNuevaConexion, unBuffer,sizeof(int),0);
				socketKernel=socketNuevaConexion;	
			}
			if(*unBuffer==CPU)
			{	tamPagina=MARCO_SIZE;
				printf("MARCO SIZE: %i\n",tamPagina );
				memcpy(unBuffer,&tamPagina, sizeof(int));
				send(socketNuevaConexion, unBuffer,sizeof(int),0);
					
			}
		dataNuevo->socket=socketNuevaConexion;
		pthread_create(&hiloComunicador,NULL,(void *) comunicarse,dataNuevo);
	}
}
}

