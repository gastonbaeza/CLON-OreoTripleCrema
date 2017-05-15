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
#define INFOPROG 16
#define PCB 17

if ((header.seleccionador).unaInterfaz==3)
{
	switch(tipoPaquete){
		case INFOPROG:
		break;
		case PCB:
		break;
		
	}
}

int main(){
	/* LEER CONFIGURACION
	*
	*/

	t_config *CFG;
	CFG = config_create("cpuCFG.txt");
	char *IP_KERNEL= config_get_string_value(CFG ,"IP_KERNEL");
	char *PUERTO_KERNEL= config_get_string_value(CFG ,"PUERTO_KERNEL");
	printf("Configuración:\nIP_KERNEL = %s,\nPUERTO_KERNEL = %s.\n",IP_KERNEL,PUERTO_KERNEL);
	
	printf("Presione enter para continuar.\n");
	getchar();
	/*
	*
	*/
int bytesRecibidos,iof;
char* message=malloc(100*sizeof(char));
char *handshakeCliente="Hola soy un cpu.";
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;		
	hints.ai_socktype = SOCK_STREAM;	

	getaddrinfo(IP_KERNEL, PUERTO_KERNEL, &hints, &serverInfo);	



	int serverSocket;
	int flagHandshake=1;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);


	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);



	freeaddrinfo(serverInfo);

	int enviar = 1;

	char *messageRecv;

	messageRecv = malloc(100*(sizeof(char)));

	while(flagHandshake){bytesRecibidos=recv(serverSocket,messageRecv,100*sizeof(char),0);
	if(bytesRecibidos>0){flagHandshake=0;fflush(stdout); printf("%s\n", messageRecv);send(serverSocket, handshakeCliente, 100*sizeof(char), 0);}
	}

	while(enviar){
			scanf("%s", message);
			if (message==)

	    }

	free(message);
	free(messageRecv);


	close(serverSocket);
	return 0;


}

void notificarKernel (char *,int socketKernel){
	send(socketKernel,char *, 100*sizeof(char), 0)
}

void enviarPCBKernel (pcb *, int socketKernel){
	send(socketKernel,pcb *, 100*sizeof(pcb), 0)
}


