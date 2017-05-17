#include "desSerializador.h"
#include "estructuras.h"
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
#define BACKLOG 5
#define INICIARPROGRAMA 0
#define FINALIZARPROGRAMA 1
#define DESCONECTARCONSOLA 2
extern pcb* procesos;
int COMUNICACIONHABILITADA=1;
int ACEPTACIONHABILITADA=1;
int SOCKETMEMORIA;
int SOCKETFS;
int * TAMPAGINA;
// SEMAFORO

void aceptar(dataParaComunicarse * dataParaAceptar){
	// VARIABLES PARA LAS CONEXIONES ENTRANTES
	void * interfaz;
	interfaz = malloc(sizeof(int));
	pthread_t hiloComunicador;
	dataParaComunicarse * dataParaConexion;
	int socketNuevaConexion;
	// RETURN VALUES
	int rv;
	// CONFIGURACION PARA ACCEPT
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	// INICIO LA ESCUCHA
	if(rv=listen(dataParaAceptar->socket,BACKLOG)==-1)
		perror("Error en el listen");
	// CICLO PRINCIPAL
	while(ACEPTACIONHABILITADA){
		if ((socketNuevaConexion = accept(dataParaAceptar->socket, (struct sockaddr *)&addr,&addrlen)) == -1)
			perror("Error con conexion entrante");
		else if (socketNuevaConexion >= 0){
			// ME INFORMO SOBRE LA INTERFAZ QUE SE CONECTÓ
			handshakeServer(socketNuevaConexion,KERNEL,interfaz);
			// CONFIGURACION E INICIO DE HILO COMUNICADOR
			dataParaConexion = malloc(sizeof(dataParaComunicarse));
			dataParaConexion->interfaz=*interfaz; // AGREGAR EL CAMPO INTERFAZ A LA ESTRUCTURA
			dataParaConexion->socket=socketNuevaConexion;
			pthread_create(&hiloComunicador,NULL,(void *) comunicarse,dataParaConexion);
		}
	}
	// LIBERO MEMORIA
	free(interfaz);
	free(dataParaAceptar);
}

void comunicarse(dataParaComunicarse * dataDeConexion){
	// RETURN VALUES
	int rv;
	// NUMERO DE BYTES
	int nbytes;
	// BUFFER RECEPTOR
	void * paquete;
	//HEADER
	t_header * header;
	// CICLO PRINCIPAL
	while(COMUNICACIONHABILITADA){
		// RECIBO EL HEADER
		// WAIT
		nbytes = recv(dataDeConexion->socket, header, sizeof(t_header), 0);
		// SIGNAL
		if (nbytes == 0){
			// EL CLIENTE FINALIZÓ LA CONEXIÓN
            printf("El cliente finalizó la conexión: socket->%i.\n", dataDeConexion->socket);
        	break;
        }
        else if (nbytes<0){
         	perror("Error en el recv: socket->%i.\n", dataDeConexion->socket);
           	break;
        }
        switch(header->seleccionador.unaInterfaz){
        	case CONSOLA:
        		switch(header->seleccionador.tipoPaquete){
					case INICIARPROGRAMA:	// RECIBIMOS EL PATH DE UN PROGRAMA ANSISOP A EJECUTAR Y SU PID
						// RECIBO EL PATH
						recibirDinamico(dataDeConexion->socket, paquete);
						char * path;
						path = malloc (header->tamanio);
						memcpy(path,paquete,header->tamanio);
						// RECIBO EL PID
						int * pid;
						pid = malloc (sizeof(int));
						if ((nbytes = recv(dataDeConexion->socket, pid,sizeof(int), 0)) == 0){
							// EL CLIENTE FINALIZÓ LA CONEXIÓN
            				printf("El cliente finalizó la conexión: socket->%i.\n", dataDeConexion->socket);
        					break;
        				}
       	 				else if (nbytes<0){
         					perror("Error en el recv: socket->%i.\n", dataDeConexion->socket);
           					break;
        				}
						// RECUPERO EL PROGRAMA DEL PATH
						t_programaSalida * programa;
						programa= obtenerPrograma(path);
						// CALCULO LA CANTIDAD DE PAGINAS
						int cantPaginas = calcularPaginas(TAMPAGINA,programa->tamanio);
						// SOLICITUD DE MEMORIA
						t_solicitudMemoria * solicitudMemoria;
						solicitudMemoria=malloc(sizeof(t_solicitudMemoria));
						solicitudMemoria->codigo=programa;
						solicitudMemoria->cantPaginasCodigo=cantPaginas;
						enviarDinamico(KERNEL,SOLICITUDMEMORIA,dataDeConexion->socket,solicitudMemoria,sizeof(t_solicitudMemoria));
						pcb *pcb;
						pcb->


						// int id = 0,tamanioAReservar;
						// int rv;
						// char * path = (char *)paquete;
						// char * buffer = malloc(sizeof(char)*33);
						// t_programaSalida * programaDePath;
					
						// while(*procesosEnEjecucion != '\0') if (id == procesosEnEjecucion[id]->pid) id++;
						// if (send(dataParaComunicarse.socket, id, sizeof(int), 0) == -1) perror("send");
						// programaDePath = obtenerPrograma(path);
						// tamanioAReservar = programaDePath->tamanio;
						// struct addrinfo hints;
						// struct addrinfo *serverInfo;
						// memset(&hints, 0, sizeof(hints));
						// hints.ai_family = AF_INET;
						// hints.ai_socktype = SOCK_STREAM;
						// getaddrinfo(dataParaComunicarse.ipMemoria,dataParaComunicarse.puertoMemoria,&hints,&serverInfo);
						// int socketMemoria;
						// socketMemoria = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
						// if ((connect(socketMemoria, serverInfo->ai_addr, serverInfo->ai_addrlen)) == -1)
						//  {
				  //           perror("Error de conexión con la memoria");
				  //           break;
		    //     		}
		    //     		hashSerializador=serializador(KERNEL,SOLICITUDMEMORIA,socketMemoria,&tamanioAReservar);
		        		
		    //     		status=recv(socketMemoria,buffer,sizeof(char)*33,0);
		        		
		    //     		status=recv(socketMemoria,confirmacion,sizeof(int),0);
		    //     		if (confirmacion)//==1 esta todo OK  ==0 recibo exception 
		    //     		{
		    //     			status=send(socketMemoria,programaDePath->elPrograma,programaDePath->tamanio,0);
		    //     		} else {

		    //     			status=recv(socketMemoria,header,sizeof(header),0);
		    //     			hashDesSerializador=desSerializador(KERNEL,EXCEPCION,socketMemoria,(void*)paquete);
		 					
		 			// 		send(sockerMemoria,hashDeserializador,33*sizeof(char),0);
		 			// 		tamanioPaquete=strlen((char*)paquete);
		 			// 		char*mensajeConsola= malloc(tamanioPaquete);
		 			// 		strcpy(mensajeConsola,paquete);
		 			// 		hashSerializador=serializador(CONSOLA,MENSAJECONSOLA,dataParaComunicarse->socket,(void*)mensajeConsola)}
		        		

						//LIBERO MEMORIA
						free(path);
						free(pid);
						free(solicitudMemoria);
					break;
					case FINALIZARPROGRAMA: // RECIBIMOS EL PID DE UN PROGRAMA ANSISOP A FINALIZAR
					break;
					case DESCONECTARCONSOLA: // SE DESCONECTA ESTA CONSOLA
					break;
		        }
		    break;
        	case CPU:
        		switch(header->seleccionador.tipoPaquete){
        			case 0:
        			break;
        		}
        	break;
        }
	}
	// LIBERO MEMORIA
	free(dataDeConexion);
}

int main(){	
/* LEER CONFIGURACION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
t_config *CFG;
CFG = config_create("kernelCFG.txt");
char *PUERTO_PROG= config_get_string_value(CFG ,"PUERTO_PROG");
char *PUERTO_CPU= config_get_string_value(CFG ,"PUERTO_CPU");
char *IP_MEMORIA= config_get_string_value(CFG ,"IP_MEMORIA");
char *PUERTO_MEMORIA= config_get_string_value(CFG ,"PUERTO_MEMORIA");
char *IP_FS= config_get_string_value(CFG ,"IP_FS");
char *PUERTO_FS= config_get_string_value(CFG ,"PUERTO_FS");
int QUANTUM= config_get_int_value(CFG ,"QUANTUM");
int QUANTUM_SLEEP= config_get_int_value(CFG ,"QUANTUM_SLEEP");
char *ALGORITMO= config_get_string_value(CFG ,"ALGORITMO");
int GRADO_MULTIPROG= config_get_int_value(CFG ,"GRADO_MULTIPROG");
char* SEM_IDS= config_get_string_value(CFG ,"SEM_IDS");
char* SEM_INIT= config_get_string_value(CFG ,"SEM_INIT");
char* SHARED_VARS= config_get_string_value(CFG ,"SHARED_VARS");
int STACK_SIZE= config_get_int_value(CFG ,"STACK_SIZE");
printf("Configuración:\nPUERTO_PROG = %s,\nPUERTO_CPU = %s,\nIP_MEMORIA = %s,\nPUERTO_MEMORIA = %s,\nIP_FS = %s,\nPUERTO_FS = %s,\nQUANTUM = %i,\nQUANTUM_SLEEP = %i,\nALGORITMO = %s,\nGRADO_MULTIPROG = %i,\nSEM_IDS = %s,\nSEM_INIT = %s,\nSHARED_VARS = %s,\nSTACK_SIZE = %i.\n"
		,PUERTO_PROG,PUERTO_CPU,IP_MEMORIA,PUERTO_MEMORIA,IP_FS,PUERTO_FS,QUANTUM,QUANTUM_SLEEP,ALGORITMO,GRADO_MULTIPROG,SEM_IDS,SEM_INIT,SHARED_VARS,STACK_SIZE);
printf("Presione enter para continuar.\n");
getchar();
/* LEER CONFIGURACION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
// RETURN VALUES
int rv;
// PREFERENCIAS DE CONEXION
struct addrinfo hints;
memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_flags = AI_PASSIVE;
hints.ai_socktype = SOCK_STREAM;
// CONEXION CON MEMORIA
struct addrinfo *memoria;
getaddrinfo(IP_MEMORIA,PUERTO_MEMORIA,&hints,&memoria);
int socketMemoria;
if ((rv = connect(socketMemoria,memoria->ai_addr,memoria->ai_addrlen)) == -1) 
	perror("No se pudo conectar con memoria.\n");
else if (rv == 0)
	printf("Se conectó con memoria correctamente.\n");
handshakeCliente(socketMemoria,KERNEL,void * interfaz);
SOCKETMEMORIA=socketMemoria;
// RECIBO EL TAMAÑO DE PAGINA
int nbytes;
int * tamPagina;
if ((nbytes = recv(SOCKETMEMORIA, tamPagina, sizeof(int), 0)) == 0){
	// SE CERRÓ LA CONEXION
    printf("Finalizó la conexión: .\n");
  	break;
}
else if (nbytes<0){
   	perror("Error en el recv.\n");
   	break;
}
TAMPAGINA=malloc(sizeof(int));
TAMPAGINA=tamPagina;
freeaddrinfo(memoria);
// CONEXION CON FILESYSTEM (NO ES NECESARIO HACER HANDSHAKE, KERNEL ES EL ÚNICO QUE SE CONECTA A FS)
struct addrinfo *fs;
getaddrinfo(IP_FS,PUERTO_FS,&hints,&fs);
int socketFS;
if ((rv = connect(socketFS,fs->ai_addr,fs->ai_addrlen)) == -1) 
	perror("No se pudo conectar con filesystem.\n");
else if (rv == 0)
	printf("Se conectó con filesystem correctamente.\n");
SOCKETFS=socketFS;
freeaddrinfo(fs);
// CONFIGURACION DEL SERVIDOR
struct addrinfo *serverInfo;
if ((rv =getaddrinfo(NULL, PUERTO_PROG, &hints, &serverInfo)) != 0)
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
socketEscuchador = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
if(bind(socketEscuchador,serverInfo->ai_addr, serverInfo->ai_addrlen)==-1) 
	perror("Error en el bind.\n");
freeaddrinfo(serverInfo);
// SE LIBERA EL ARCHIVO DE CONFIGURACION
config_destroy(CFG);
// CONFIGURACION DE HILO ACEPTADOR
dataParaComunicarse *dataParaAceptar;
dataParaAceptar=malloc(sizeof(dataParaComunicarse));
dataParaAceptar->socket = socketEscuchador;
// INICIO DE HILO ACEPTADOR
pthread_t hiloAceptador;
pthread_create(&hiloAceptador,NULL,(void *)aceptar,dataParaAceptar);
pthread_join(hiloAceptador,NULL);
return 0;
}