#include "funciones.h"
#include "estructuras.h"
#include <parser/parser.h>
#include <commons/collections/list.h>
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
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stddef.h>



#define SOLICITUDLINEA 2
#define FINALIZARPROCESO 16
#define PCB 17
#define CPU 1

#define LINEA 19

t_puntero (*AnSISOP_definirVariable)(t_nombre_variable identificador_variable){

}

int continuarEjecucion=1;
int PID;
int i;
t_pcb * pcb;
char * IP_KERNEL;
char * PUERTO_KERNEL;
char * IP_MEMORIA;
char * PUERTO_MEMORIA;



int socketKernel;
int socketMemoria;


void conectarKernel(void){

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

void * unBuffer;
handshakeCliente(socketKernel, CPU, unBuffer);
void * paquete;
int recibir;
t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));

t_peticionLinea * peticionLinea;
while(1) {
	while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0));
	
	if(seleccionador->unaInterfaz==CPU){
	switch (seleccionador->tipoPaquete){
		case PCB: // [Identificador del Programa] // informacion del proceso
							
							recibirDinamico(PCB,socketKernel,pcb);
							
 							
 							peticionLinea=malloc(sizeof(t_peticionLinea));
							peticionLinea->start=pcb->indiceCodigo[0].start;
							peticionLinea->offset=pcb->indiceCodigo[0].offset;		
							envioDinamico(SOLICITUDLINEA,socketMemoria,(void *) peticionLinea);
							free(peticionLinea);
							pcb->programCounter=pcb->indiceCodigo[0].start;
							i=0;

 								
 								
 				break;

 		case FINALIZARPROCESO: 
 								continuarEjecucion=0; 
 								free(pcb->indiceCodigo);					
 		break;
}}}
free(seleccionador);
}	



void conectarMemoria(void){
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
	void * unBuffer;

	
	handshakeCliente(socketMemoria, CPU, unBuffer);
	void * paquete;
	int recibir;
	t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
	t_linea * linea;
	char* lineaCodigo;
	t_programaSalida * programa;
	while(1){
		while(0>recv(socketMemoria, seleccionador, sizeof(t_seleccionador),0));
		
		if(continuarEjecucion==1){
	if(seleccionador->unaInterfaz==CPU){
 	switch(seleccionador->tipoPaquete){
 		case LINEA: 
 					linea=malloc(sizeof(t_linea));
 					recibirDinamico(LINEA,socketMemoria, linea);
 					lineaCodigo=malloc(linea->tamanio);
 					lineaCodigo=linea->linea;
 					iniciarEjecucion(lineaCodigo);
 					free(linea);
 		break;



 		
}}}

				
 	}
 	free(seleccionador);
 }			


 







//tiene que estar siempre en contacto con la memoria
//cuando se conecta con el kernel, queda a la espera de recibir el pcb de un proceso
//tiene que aumentar el program counter del PCB
//utilizo el indice de codigo para solicitar a memoria la proxima linea a ejecutar
//cuando la ejecuto se debe acutualizar los valores del programa en la memoria
//actualizar el program counter del PCB
//cuando termina todo el codigo de ejecutar avisarle al kernel para que libere ese espacio que ocupaba










int main(){


	t_config * CFG;
	CFG = config_create("CPUCFG.txt");
	IP_KERNEL=(char*) config_get_string_value(CFG ,"IP_KERNEL");
	IP_MEMORIA= (char*)config_get_string_value(CFG, "IP_MEMORIA");
	PUERTO_KERNEL= (char*)config_get_string_value(CFG ,"PUERTO_KERNEL");
	PUERTO_MEMORIA= (char*)config_get_string_value(CFG,"PUERTO_MEMORIA");
	printf("Configuración:\nIP_KERNEL = %s,\nIP_MEMORIA=%s,\nPUERTO_KERNEL = %s.\n,PUERTO_MEMORIA=%s. \n",IP_KERNEL,IP_MEMORIA,PUERTO_KERNEL,PUERTO_MEMORIA);
	
	printf("Presione enter para continuar.\n");
	getchar();
	pcb=malloc(sizeof(t_pcb));
	/*
	*
	*/


	config_destroy(CFG);

	pthread_t conectarKernel, conectarMemoria;
	pthread_create(&conectarMemoria, NULL, (void *) conectarMemoria,&socketMemoria);
	pthread_create(&conectarKernel, NULL, (void *) conectarKernel,&socketKernel);
	pthread_join(conectarKernel,NULL);
	pthread_join(conectarMemoria,NULL);






void iniciarEjecucion(char * linea){

		
		printf("\t Evaluando -> %s", linea);
		
		analizadorLinea(linea, &functions, &kernel_functions);

		//tengo que guardar en que linea estoy en el program counter para que cuando tuermine un quantum guardar ese contexto para que despues pueda seguir desde ahi
		
		
		i++;
		
		t_peticionLinea * peticionLinea=malloc(sizeof(t_peticionLinea));
		peticionLinea->start=pcb->indiceCodigo[i].start;
		peticionLinea->offset=pcb->indiceCodigo[i].offset;		
		envioDinamico(SOLICITUDLINEA,socketMemoria,(void *) peticionLinea);
		pcb->programCounter=pcb->indiceCodigo[i].start;
		free(peticionLinea);
	
	
		
}}

