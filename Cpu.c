#include "funciones.h"
#include "estructuras.h"
#include "parser.h"
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
#include "metadata_program.h"
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stddef.h>
#include "dummy_ansisop.h"


#define CPU 1
#define solicitudDeLineaCodigo 3
#define ACTUALIZARPCB 4
#define solicitudDeCodigo 5
#define FINALIZARPROCESO 16
#define PCB 17
#define CODIGO 18
#define LINEA 19

int continuarEjecucion=1;
int index=0;
int PID;

t_pcb * pcb;


int socketKernel;
int socketMemoria;


void conectarKernel(socketKernel){

int bytesRecibidos;
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(IP_KERNEL,PUERTO_KERNEL,&hints,&serverInfo);


	
	
	
	socketKernel = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);


	connect(socketKernel, serverInfo->ai_addr, serverInfo->ai_addrlen);

	
	freeaddrinfo(serverInfo);


void handshakeCliente(socketKernel, CPU, int * unBuffer);
void * paquete;
int recibir;
t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
t_pcb * pcb;
while(1) {
	while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0));
	paquete=malloc(seleccionador->tamanio);
	if(seleccionador->unaInterfaz==CPU){
	switch (seleccionador->tipoPaquete){
		case PCB: // [Identificador del Programa] // informacion del proceso
							recibirDinamico(PCB,socketKernel,(void *) paquete);
							
							pcb=malloc(t_pcb);
 							memcpy((void *)pcb,(void *)paquete,sizeof(paquete));
 							PID= pcb->pid;
 							
							envioDinamico(solicitudDeCodigo, socketMemoria, (void *)PID); 		
 								
 				break;

 		case FINALIZARPROCESO: 
 								
 								continuarEjecucion=0; 								
 		break;
}}}}	



void conectarMemoria(socketMemoria){
	int socketMemoria;
	int bytesRecibidos;
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(IP_MEMORIA,PUERTO_MEMORIA,&hints,&serverInfo);


	socketMemoria = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);


	connect(socketMemoria, serverInfo->ai_addr, serverInfo->ai_addrlen);

	freeaddrinfo(serverInfo);

	
	void handshakeCliente(socketMemoria, CPU, int * unnBuffer);
	void * paquete;
	int recibir;
	t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
	t_linea * linea;
	t_programaSalida * programa;
	while(1){
		while(0>recv(socketMemoria, seleccionador, sizeof(t_seleccionador),0));
		paquete=malloc(seleccionador->tamanio);
		if(continuarEjecucion==1){
	if(seleccionador->unaInterfaz==CPU){
 	switch(seleccionador->tipoPaquete){
 		case LINEA: 
 					recibirDinamico(LINEA,socketMemoria, (void *) paquete);
 					
 					linea=malloc(t_linea);
 					memcpy((void *)linea,(void *)paquete,sizeof(paquete));
 					linea=linea->linea;
 					iniciarEjecucion(linea);
 					free(linea);
 		break;



 		case CODIGO:
 					recibirDinamico(CODIGO,socketMemoria,(void*)paquete);
 					
 					programa=malloc(t_programaSalida);
 					memcpy((void *)codigo,(void *)paquete,sizeof(paquete));
 					codigo=programa->elPrograma;
 					t_metadata_program * metadata;
 					metadata = metadata_desde_literal(codigo);
 					int indice[metadata->instruccionesSize][2];
 					int unaFila;
 					int unaColumna;
 					for ( unaFila= 0; unaFila< metadata->instruccionesSize; unaFila++)
 					{	
 						for ( unaColumna= 0; unaColumna < 2; ++unaColumna)
 						{
 							if (unaColumna==0)
 							{
 								indice[unaFila][unaColumna]=(metadata->instruccionesSerializadas+unaFila*sizeof(t_instructions))->start;
 							}
 							else if (unaColumna==1)
 							{
 								indice[unaFila][unaColumna]=(metadata->instruccionesSerializadas+unaFila*sizeof(t_instructions))->offset;
 							}
 						}
 					}
 					//creo el vector del indice de codigo con tamaño de intrucciones_size
 					t_peticionLinea * peticionLinea=malloc(sizeof(t_peticionLinea));
 					peticionLinea->start=pcb->indiceCodigọ[index][0];
 					peticionLinea->offset=pcb->indiceCodigo[index][1]];
 					enviarDinamico(solicitudDeLineaCodigo, socketMemoria,peticionLinea);
 					pcb->programCounter=pcb->indiceCodigo[index][0];
 					free(peticionLinea);
					
 					

 		break;
}}}

				
 	}}			


 





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
	char *IP_KERNEL=(char*) config_get_string_value(CFG ,"IP_KERNEL");
	char *IP_MEMORIA= (char*)config_get_string_value(CFG, "IP_MEMORIA");
	char *PUERTO_KERNEL= (char*)config_get_string_value(CFG ,"PUERTO_KERNEL");
	char *PUERTO_MEMORIA= (char*)config_get_string_value(CFG,"PUERTO_MEMORIA");
	printf("Configuración:\nIP_KERNEL = %s,\nIP_MEMORIA=%s,\nPUERTO_KERNEL = %s.\n,PUERTO_MEMORIA=%s. \n",IP_KERNEL,IP_MEMORIA,PUERTO_KERNEL,PUERTO_MEMORIA);
	
	printf("Presione enter para continuar.\n");
	getchar();
	/*
	*
	*/


	config_destroy(CFG);

	pthread_t conectarKernel, conectarMemoria;
	pthread_create(&conectarKernel, NULL, (void *) conectarKernel,&socketKernel);
	pthread_create(&conectarMemoria, NULL, (void *) conectarMemoria,&socketMemoria);
	pthread_join(conectarKernel,NULL);
	pthread_join(conectarMemoria,NULL);






void iniciarEjecucion(linea){

		
		printf("\t Evaluando -> %s", linea);
		
		analizadorLinea(linea, &functions, &kernel_functions);

		//tengo que guardar en que linea estoy en el program counter para que cuando tuermine un quantum guardar ese contexto para que despues pueda seguir desde ahi
		
		
		index++
		
		t_peticionLinea * peticionLinea=malloc(sizeof(t_peticionLinea));
		peticionLinea->start=pcb->indiceCodigo[index][0];
		peticionLinea->offset=pcb->indiceCodigo[index][1]];		
		envioDinamico(solicitudDeLineaCodigo,socketMemoria,(void *) peticionLinea);
		pcb->programCounter=pcb->indiceCodigo[index][0];
	
	
		
}}

