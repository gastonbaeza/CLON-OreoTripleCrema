#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <commons/config.h>
#include "desSerializador.h"
#include "estructuras.h"
extern pthread_mutex_t comunicacionConsolas;



void enviar(){

}


void recibir(){

}


int main(){
	/* LEER CONFIGURACION
	*
	*/

	t_config *CFG;
	CFG = config_create("consolaCFG.txt");
	char *IP_KERNEL= config_get_string_value(CFG ,"IP_KERNEL");
	char *PUERTO_KERNEL= config_get_string_value(CFG ,"PUERTO_KERNEL");
	printf("Configuración:\nIP_KERNEL = %s,\nPUERTO_KERNEL = %s.\n",IP_KERNEL,PUERTO_KERNEL);
	
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

	pthread_t hiloEnviar, hiloRecibir;
	pthread_create(&hiloEnviar, NULL, (void *) enviar, NULL);
	pthread_create(&hiloRecibir, NULL, (void *) recibir, NULL);
	pthread_join(hiloEnviar); // cuando el hilo enviar termina (el usuario ingresa quit por ej) se termina el proceso consola


	free(message);
	free(messageRecv);
	close(serverSocket);
	return 0;


}
