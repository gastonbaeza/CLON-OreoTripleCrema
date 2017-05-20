#include "funciones.h"
#include "estructuras.h"
#include <commons/collections/list.h>
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
#include <stddef.h>
#include <openssl/md5.h>
#include <parser/metadata_program.h>
#include "dummy_ansisop.h"
#define INFOPROG 16
#define PCB 17



int PID;

int programCounter;


void conectarKernel(socketKernel){
int socketKernel;
int * unBuffer=malloc(sizeof(int));
void handshakeCliente(socketKernel, CPU, int * unBuffer);
void * paquete;
int recibir;
t_header * header;

while(1) {
	recibir=recv(socketKernel,header, sizeof(t_header),0);
	paquete=malloc(header->tamanio);
	if(header->seleccionador.unaInterfaz==CPU){
	switch (header->seleccionador.tipoPaquete){
		case PCB: // [Identificador del Programa] // informacion del proceso
							recibirDinamico(unSocket,paquete,sizeof(paquete));
							t_pcb * pcb;
							pcb=malloc(header->tamanio);
 							memcpy((void *)pcb,(void *)paquete,header->tamanio);
 							PID= pcb->pid;
 							
							envioDinamico(MEMORIA, solicitudDeCodigo, socketMemoria, (void *)PID, sizeof(PID)); 		
 								
 				break;}
}}	



void conectarMemoria(socketMemoria){
	int socketMemoria;
	int *  unnBuffer=malloc(sizeof(int));
	void handshakeCliente(socketMemoria, CPU, int * unnBuffer);
	void * paquete;
	int recibir;
	t_header * header;

	while(1){
		recibir=recv(socketMemoria, header, sizeof(t_header));
		paquete=malloc(header->tamanio);
	
	if(header->seleccionador.unaInterfaz==MEMORIA){
 	switch(header->seleccionador.tipoPaquete){
 		case LINEA: 
 					recibirDinamico(socketMemoria, paquete);
 					t_lineaCodigo * lineaCodigo;
 					linea=malloc(header->tamanio);
 					memcpy((void *)linea,(void *)paquete,header->tamanio);
 					linea=linea->lineaCodigo;
 					iniciarEjecucion(linea);
 		break;



 		case CODIGO:
 					recibirDinamico(socketMemoria,paquete);
 					t_programaSalida * programa;
 					programa=malloc(header->tamanio);
 					memcpy((void *)codigo,(void *)paquete,header->tamanio);
 					codigo=programa->elPrograma;
 					t_metadata_program * metadata;
 					metadata = metadata_desde_literal(codigo);
 					start=metadata->instrucciones_serializado[programCounter].start;
 					offset=metadata->instrucciones_serializado[programCounter].offset;
 					t_actualizacion * actualizacion;
					actualizacion=malloc(sizeof(header->tamanio));
					actualizacion->pid=PID;
					actualizacion->programCounter=programCounter;
					actualizacion->start=start;
					actualizacion->ofsset=offset;
					enviarDinamico(MEMORIA,ACTUALIZARPCB,socketMemoria,(void *)actualizacion,sizeof(actualizacion));
					t_linea* peticionLinea;
 					peticionLinea=malloc(sizeof(t_linea));
 					peticionLinea->start=start;
 					peticionLinea->offset=offset;
 					envioDinamico(MEMORIA, solicitudDeLinea, socketMemoria, (void *) peticionLinea, sizeof(peticionLinea));

 		break;
}}

				
 	}			


 





AnSISOP_funciones functions = {
		.AnSISOP_definirVariable		= dummy_definirVariable,
		.AnSISOP_obtenerPosicionVariable= dummy_obtenerPosicionVariable,
		.AnSISOP_finalizar 				= dummy_finalizar,
		.AnSISOP_dereferenciar			= dummy_dereferenciar,
		.AnSISOP_asignar				= dummy_asignar,

};
AnSISOP_kernel kernel_functions = { };


//tiene que estar siempre en contacto con la memoria
//cuando se conecta con el kernel, queda a la espera de recibir el pcb de un proceso
//tiene que aumentar el program counter del PCB
//utilizo el indice de codigo para solicitar a memoria la proxima linea a ejecutar
//cuando la ejecuto se debe acutualizar los valores del programa en la memoria
//actualizar el program counter del PCB
//cuando termina todo el codigo de ejecutar avisarle al kernel para que libere ese espacio que ocupaba










int main(){


	t_config *CFG;
	CFG = config_create("CPUCFG.txt");
	char *IP_KERNEL= config_get_string_value(CFG ,"IP_KERNEL");
	char *PUERTO_KERNEL= config_get_string_value(CFG ,"PUERTO_KERNEL");
	char *PUERTO_MEMORIA= config_get_string_value(CFG,"PUERTO_MEMORIA");
	printf("Configuración:\nIP_KERNEL = %s,\nPUERTO_KERNEL = %s.\n,PUERTO_MEMORIA=%s. \n",IP_KERNEL,PUERTO_KERNEL);
	
	printf("Presione enter para continuar.\n");
	getchar();
	/*
	*
	*/
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

	pthread_t conectarKernel, conectarMemoria;
	pthread_create(&conectarKernal, NULL, (void *) conectar,&socketKernel);
	pthread_create(&conectarMemoria, NULL, (void *) conectar,&socketMemoria);
	pthread_join(conectarKernal,NULL);
	pthread_join(conectarMemoria,NULL);






iniciarEjecucion(LINEA){
	printf("Ejecutando\n");
	char *linea = strdup(LINEA);

		printf("\t Evaluando -> %s", linea);
		
		analizadorLinea(linea, &functions, &kernel_functions);
		
		free(linea);
		programCounter++;
	

	}
	metadata_destruir(metadata);
	printf("terminoDeEjecutar\n");
	t_finalizacion * finalizar;
	finalizar=malloc(sizeof(header->tamanio));
	finalizar->pid=PID;
	envioDinamico(KERNEL,PIDFINALIZOPROCESO,socketKernel,(void *) finalizar,sizeof(t_finalizar));
		return EXIT_SUCCESS;
}

