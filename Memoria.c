


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
#include <sys/time.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stddef.h>
#include <math.h>
#include <stdint.h>
#include <commons/config.h>
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
#define ASIGNARPAGINAS 63
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
t_list**overflow;

/* LEER CONFIGURACION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
t_config *CFG;
/* LEER CONFIGURACION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
void consolaMemoria();
void comunicarse(int * socket);
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




int nbytes;

int a,listenningSocket,rv,i;
struct addrinfo hints;
struct addrinfo *serverInfo;


memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_flags = AI_PASSIVE;
hints.ai_socktype = SOCK_STREAM;

if ((rv =getaddrinfo(NULL, PUERTO, &hints, &serverInfo)) != 0) fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));

fflush(stdout);
printf("%s \n", "El Servidor esta configurado.\n");
listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
fflush(stdout);
printf("%s \n", "Socket Listo.");

if(bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen)==-1) {perror("Error en el bind."); exit(1);}

fflush(stdout);
printf("%s \n", "Bind Listo.\n");

freeaddrinfo(serverInfo);
fflush(stdout);
config_destroy(CFG);
system("clear");
printf("%s \n", "El Servidor se encuentra listo para escuchar conexiones.");
pthread_t hiloAceptador;
dataParaComunicarse *unData;
unData=malloc(sizeof(dataParaComunicarse));
unData->socket = listenningSocket;

 ////////////////////////////////////// INICIALIZAMOS EL BLOQUE DE MEMORIA/////////////////////////////////////

bloquesAdmin=calloc(1,MARCOS*sizeof(t_estructuraADM)+MARCOS*sizeof(t_marco));
tamanioAdministrativas=MARCOS*sizeof(t_estructuraADM);
overflow = calloc(1,sizeof(t_list*) * MARCOS);
inicializarOverflow(MARCOS,overflow);
int unaAdmin;
for (unaAdmin = 0; unaAdmin < MARCOS; unaAdmin++) //inicializo los bloques de admin con pid-1 libre y num de pag y el hash
{
bloquesAdmin[unaAdmin].estado=0;
bloquesAdmin[unaAdmin].pid=-1;
bloquesAdmin[unaAdmin].pagina=-1;
	
}
 
marcos=(t_marco*)(bloquesAdmin+tamanioAdministrativas);
int unMarco=0;

memoria=calloc(MARCOS,MARCO_SIZE);
cache=calloc(ENTRADAS_CACHE,MARCO_SIZE);
memoriaCache=(t_estructuraCache*)calloc(1,sizeof(t_estructuraCache)*ENTRADAS_CACHE);
 // marcos es un bloque de punteros a void porque el tipo void en c no existe y aca quiero pegar lo que me de la gana.
for(unMarco;unMarco<MARCOS; unMarco++) //asignar su numero de marco a cada region de memoria
	{ 
		marcos[unMarco].numeroPagina=memoria+(unMarco*MARCO_SIZE);
		
	}

for(unMarco=0;unMarco<ENTRADAS_CACHE; unMarco++) //asignar su numero de marco a cada region de memoria
	{
		memoriaCache[unMarco].contenido=cache+(unMarco*MARCO_SIZE);
		memoriaCache[unMarco].frame=-666; //para distinguir  una pagina real de una virgen
		memoriaCache[unMarco].pid=-1;
		memoriaCache[unMarco].antiguedad=-1;
			}
	
	asignador=&marcos[0];
////////////////////////////////INICIAMOS HILOS DE COMINICACION/////////////////////////////////////////////



pthread_create(&hiloAceptador,NULL,(void *)aceptar,unData);
pthread_t hiloConsolaMemoria;
pthread_create(&hiloConsolaMemoria,NULL,(void *)consolaMemoria,NULL);
pthread_join(hiloAceptador,NULL);
pthread_join(hiloConsolaMemoria,NULL);
free(memoria);free(cache);
free(bloquesAdmin); 
free(asignador);
free(marcos);
free(memoriaCache); 
free(overflow); 
free(unData);

}



void comunicarse(int  * socket){ // aca tenemos que agregar toda la wea que es "global"
fflush(stdout); 
int * buffer;
int unData=*socket;

int a=1;
t_seleccionador * seleccionador;
int entrada;
void * paquete;
int paginasRequeridas;
int stackRequeridas;
int indice;
int test;
t_peticionBytes * peticionBytes;
t_almacenarBytes * bytesAAlmacenar;
t_solicitudMemoria * solicitud;
int  * pidALiberar;
int confirmacion;
int * marco;
int pedidos;
t_solicitudAsignar * pedidoAsignacion;
int ultimaPagina;
while(1) {
	
			seleccionador=calloc(1,8);
			while(0>recv(unData,seleccionador,sizeof(t_seleccionador),0));
			printf("el pqeute es %i\n",seleccionador->tipoPaquete);
			switch (seleccionador->tipoPaquete)
			{
					case SOLICITUDMEMORIA: // [Identificador del Programa] // paginas necesarias para guardar el programa y el stack
							 //esto lo vi en stack overflow no me peguen
							solicitud=calloc(1,sizeof(t_solicitudMemoria));
							recibirDinamico(SOLICITUDMEMORIA,unData,solicitud);
							
							fflush(stdout);printf("Tamaño: %i\n", solicitud->tamanioCodigo);
							printf("Codigo: %s\n", solicitud->codigo); 
							printf("Cant Pags Codigo: %i\n", solicitud->cantidadPaginasCodigo);
							printf("Cant Pags Stack: %i\n", solicitud->cantidadPaginasStack);
							printf("PID: %i\n", solicitud->pid);
							
 							
 							paginasRequeridas=solicitud->cantidadPaginasCodigo;
 							stackRequeridas=solicitud->cantidadPaginasStack;
 							
 							if(hayPaginasLibres(paginasRequeridas+stackRequeridas,bloquesAdmin,MARCOS)==-1) 
 							{ 
 							solicitud->respuesta=FAIL;
 							enviarDinamico(SOLICITUDMEMORIA,socketKernel, (void *) solicitud);
 							}

 							else
 							{ //mandarOK memoria
 							solicitud->respuesta=OK;
 						
 							enviarDinamico(SOLICITUDMEMORIA,socketKernel, (void *) solicitud);
 							
 							test=reservarYCargarPaginas(paginasRequeridas,stackRequeridas,MARCOS,bloquesAdmin,&marcos,solicitud->tamanioCodigo, solicitud->pid,&(solicitud->codigo),MARCO_SIZE,overflow,ENTRADAS_CACHE,memoriaCache);
 							if(test==1)printf("%s\n","las paginas fueron reservadas bien" );
 							else printf("%s\n","algo malo paso en la reserva" );
 							free(solicitud->codigo);
 							free(solicitud);
 							
 							
 							
 							 }	
 							
 							
 							
 							

 					break;
 		 
 					case SOLICITUDBYTES:
 										peticionBytes=calloc(1,sizeof(t_peticionBytes));
 										recibirDinamico(SOLICITUDBYTES,unData,peticionBytes);
 										paquete=calloc(1,peticionBytes->size+1);
 										printf("pid: %i\n", peticionBytes->pid);
 										printf("pagina: %i\n",peticionBytes->pagina );
 										printf("size: %i\n", peticionBytes->size);
 										printf("offset: %i\n", peticionBytes->offset);
 										if(!existePagina(peticionBytes->pid,peticionBytes->pagina ,bloquesAdmin,MARCOS))
										{	confirmacion=-1;
											send(unData, &confirmacion, sizeof(int),0);
										}
										else{
											confirmacion=1;
											send(unData, &confirmacion, sizeof(int),0);
 										if((entrada=estaEnCache(peticionBytes->pid,peticionBytes->pagina,memoriaCache,ENTRADAS_CACHE))!=-1)
 										{//lo busco en cache
 											printf("entre a cache\n");
 											
 											paquete=(void*)solicitarBytesCache(peticionBytes->pid,peticionBytes->pagina,memoriaCache,ENTRADAS_CACHE,peticionBytes->offset,peticionBytes->size);
 											printf("%s\n","cargue el paquete con la solicitud" );
 										}
 										else
 										{//lo busco en memoria
 											printf("estoy buscando la cosa en memoria porque no estaba en cache%s\n"," " );
 											indice=calcularPosicion(peticionBytes->pid,peticionBytes->pagina,MARCOS); printf("el indice en memoria: %i\n",indice );
 											entrada=buscarEnOverflow(indice,peticionBytes->pid,peticionBytes->pagina,bloquesAdmin,MARCOS,overflow);printf("la entrada de hash en memoria: %i\n",entrada );
 											memcpy(paquete,marcos[entrada].numeroPagina+peticionBytes->offset,peticionBytes->size);printf("%s\n","antes de escribir en la cache" );
 											escribirEnCache(peticionBytes->pid,peticionBytes->pagina,marcos[entrada].numeroPagina,memoriaCache,ENTRADAS_CACHE,0,MARCO_SIZE);
 											//uso escribirEnCache para guardar una pagina entera en cache que esta en memoria
 										}	
 											buffer=calloc(1,sizeof(int));
											memcpy(buffer,&a,sizeof(int));
											
											
 											printf("solicbytes\n");
 											send(unData,paquete,peticionBytes->size,0);
 											printf("paquete: %s\n", (char*)paquete);
 										}
 											free(paquete);
 											free(buffer);
 											free(peticionBytes);
 					break;
 					case ASIGNARPAGINAS: //: [Identificador del Programa] [Páginas requeridas]
 										pedidoAsignacion=calloc(1,sizeof(t_solicitudAsignar));
 										recibirDinamico(ASIGNARPAGINAS,unData,pedidoAsignacion);
 										if(hayPaginasLibres(pedidoAsignacion->paginasAAsignar,bloquesAdmin,MARCOS)==-1) 
 										{ 
 											confirmacion=-1;
											send(unData, &confirmacion, sizeof(int),0);
 										}
 										else{
 											confirmacion=0;
 											send(unData, &confirmacion, sizeof(int),0);
 											marco=calloc(1,sizeof(int));
 											for(pedidos=0;pedidos<pedidoAsignacion->paginasAAsignar;pedidos++)
 											{
											ultimaPagina=buscarUltimaPaginaAsignada(pedidoAsignacion->pid,bloquesAdmin,MARCOS);
											indice=calcularPosicion(pedidoAsignacion->pid,ultimaPagina,MARCOS);
         									*marco=buscarMarcoLibre(marcos,MARCOS,bloquesAdmin); 
         									if(*marco!=-1)
         									{
         									agregarSiguienteEnOverflow(indice,marco,overflow);
        									 bloquesAdmin[*marco].estado=1;
        									bloquesAdmin[*marco].pid=pedidoAsignacion->pid;
        									bloquesAdmin[*marco].pagina=ultimaPagina;
        									ultimaPagina++;
 											}
											} free(marco);}free(pedidoAsignacion);
							 			
 					//break;
 					case ALMACENARBYTES:	
 								bytesAAlmacenar=calloc(1,sizeof(t_almacenarBytes));
								bytesAAlmacenar->valor=calloc(1,20);
 								recibirDinamico(ALMACENARBYTES,unData,bytesAAlmacenar);
 								printf("el pid que tengo qe almacenar es :%i\n",bytesAAlmacenar->pid ); printf("la pagina que tengo que almacenar es :%i\n",bytesAAlmacenar->pagina );
 								printf(" offset de almacenar %i\n", bytesAAlmacenar->offset);
 								printf("el valor es %s\n",bytesAAlmacenar->valor );
								if(!existePagina(bytesAAlmacenar->pid,bytesAAlmacenar->pagina,bloquesAdmin,MARCOS))
								{	confirmacion=-1;
									send(unData, &confirmacion, sizeof(int),0);
								}
								else
								{
								almacenarBytes(bytesAAlmacenar->pid,bytesAAlmacenar->pagina,bytesAAlmacenar->valor, marcos,MARCOS, bytesAAlmacenar->offset,bytesAAlmacenar->size,bloquesAdmin,overflow,memoriaCache,ENTRADAS_CACHE,MARCO_SIZE);
								confirmacion=0;
								send(unData, &confirmacion, sizeof(int),0);
								}free(bytesAAlmacenar->valor);
								free(bytesAAlmacenar);
 					break;
 					case LIBERARMEMORIA: 
 											pidALiberar=calloc(1,sizeof(int));
 											buffer=calloc(1,sizeof(int));
											memcpy(buffer,&a,sizeof(int));
 											while(0==recv(unData,pidALiberar,sizeof(int),0));
 											send(unData,buffer,sizeof(int),0);
 											liberarPaginas(pidALiberar,bloquesAdmin,marcos,MARCOS,overflow,MARCO_SIZE);
 											free(buffer);free(pidALiberar);
 							 
 					break;
 					/*case ACTUALIZARPCB:
 								recibirDinamico(PCB,unData,paquete);
 								unPcb=(t_pcb *)paquete;
 								free(paquete);
								free(unPcb);

 					break;*/
 				}free(seleccionador);
}//while

}	





void consolaMemoria()
{	printf("%s\n", "bienvenido a la consola de memoria");
	int pid;
	int unMarco;
	
	int cancelarThread=0;
	while(cancelarThread==0)
	{ system("clear");
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
						system("clear");
						printf("%s\n","Ingrese el nuevo valor de retardo: " );
						getchar();
						scanf("%d",&retardo);
						
							
						retardo=retardo;
						printf("El nuevo retardo ha sido actualizado a: %i %s\n", retardo,"milisegundos" );printf("%s\n", "presione una tecla para volver al menu de la consola");getchar(); getchar();
		break;
	case DUMP:		system("clear");
					fflush(stdout);printf("%s\n", "Para realizar un dump de cache ingrese 0"); 
					fflush(stdout);printf("%s\n", "Para realizar un dump de memoria ingrese 1"); 
					fflush(stdout);printf("%s\n", "Para realizar un dump de administrativas ingrese 2"); 
					fflush(stdout);printf("%s\n", "Para realizar un dump de un proceso ingrese 3"); 
					fflush(stdout);printf("%s\n","para realizar un dump de overflow ingrese 4" );


					scanf("%d",instruccionConsola);//preguntar si en disco quiere decir en filesystem o en disco posta
					switch(*instruccionConsola)
					{
						case CACHE:
									system("clear");
									generarDumpCache(memoriaCache,ENTRADAS_CACHE,MARCO_SIZE);printf("%s\n", "presione una tecla para volver al menu de la consola");getchar();getchar();
						break;
						case MEMORIA:
										system("clear");
										generarDumpMemoria(asignador,MARCOS,MARCO_SIZE);printf("%s\n", "presione una tecla para volver al menu de la consola");getchar();getchar();
						break;
						case TABLA:
										system("clear");
										generarDumpAdministrativas(bloquesAdmin, MARCOS);	printf("%s\n", "presione una tecla para volver al menu de la consola");getchar();getchar();
						break;
						case PID:		system("clear");
										printf("%s\n","ingrese un pid para realizar dump" );
										getchar();
										scanf("%d",&pid);
										printf("%i",pid);
										generarDumpProceso(bloquesAdmin,MARCOS,pid,marcos); printf("%s\n", "presione una tecla para volver al menu de la consola");getchar();getchar();
						break;
						case 4 : system("clear");
									generarDumpOverflow(overflow,MARCOS);printf("%s\n", "presione una tecla para volver al menu de la consola");getchar();getchar();
									break;
						default: pagaraprata();
						break;
					}
							
							
	
		break;
	case FLUSH:	system("clear");
					
					for(unMarco=0;unMarco<ENTRADAS_CACHE; unMarco++) //asignar su numero de marco a cada region de memoria
					{
					memset(memoriaCache[unMarco].contenido,0,MARCO_SIZE);
					memoriaCache[unMarco].frame=-666; //para distinguir  una pagina real de una virgen
					memoriaCache[unMarco].pid=-1;
					memoriaCache[unMarco].antiguedad=-1;
					}	printf("%s\n","la memoria cache ha sido reinicializada" );	
						printf("%s\n", "presione una tecla para volver al menu de la consola");getchar(); getchar();
		break;
	case SIZE:	system("clear");		fflush(stdout);printf("%s\n", "para conocer el tamaño de la memoria ingrese 0");

								fflush(stdout);printf("%s\n", "para conocer el tamaño de un proceso ingrese 1");getchar();
			 					scanf("%d",instruccionConsola);
			switch(*instruccionConsola)
			{
				case MEMORIASIZE:	system("clear");
								fflush(stdout);printf("cantidadDePaginas: %i\n", MARCOS);
								bloquesLibre=cantidadBloquesLibres(MARCOS,bloquesAdmin);
								fflush(stdout);printf("cantidadBloquesLibres: %i\n", bloquesLibre);
								bloquesOcupados=cantidadBloquesOcupados(MARCOS,bloquesAdmin);
								fflush(stdout);printf("cantidadBloquesOcupados: %i\n", bloquesOcupados);
								printf("%s\n", "presione una tecla para volver al menu de la consola");
								fflush(stdout);
								getchar();getchar();
				break;
				case PIDSIZE:system("clear"); printf("%s\n","ingrese un pid" );
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

int socketNuevaConexion,rv;
pthread_t hiloComunicador;
struct sockaddr_in addr;
int addrlen = sizeof(addr);
if(rv=listen(unData->socket,BACKLOG)==-1) perror("Error en el listen");
while(1){
	int * unBuffer=calloc(1,sizeof(int));
	if ((socketNuevaConexion = accept(unData->socket, (struct sockaddr *)&addr,&addrlen)) == -1) perror("Error con conexion entrante");
	else {
		
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
			free(unBuffer);
		pthread_create(&hiloComunicador,NULL,(void *) comunicarse,(void*)&socketNuevaConexion);
	}
} pthread_join(hiloComunicador,NULL);
}

