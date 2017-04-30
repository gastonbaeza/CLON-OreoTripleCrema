#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <commons/config.h>
#define MENSAJE 0
#define PID 1
#define Excepcion 2


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
	char *message=malloc(100*sizeof(char));
	char *handshakeCliente="Hola soy una consola.";
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(IP_KERNEL,PUERTO_KERNEL,&hints,&serverInfo);


	
	
	int serverSocket;
	int flagHandshake=1;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);


	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);

	freeaddrinfo(serverInfo);

	config_destroy(CFG);

	int enviar = 1;

	char *messageRecv;

	messageRecv = malloc(100*(sizeof(char)));

	while(flagHandshake){bytesRecibidos=recv(serverSocket,messageRecv,100*sizeof(char),0);
	if(bytesRecibidos>0){flagHandshake=0;fflush(stdout); printf("%s\n", messageRecv);send(serverSocket, handshakeCliente, 100*sizeof(char), 0);}
	}

	while(enviar){



		scanf("%s", message);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		if (!strcmp(message,"exit\n")) enviar = 0;			// Chequeo que el usuario no quiera salir
		if (enviar) send(serverSocket, message,100*sizeof(char), 0); 	// Solo envio si el usuario no quiere salir.
	}

	free(message);
	free(messageRecv);


	close(serverSocket);
	return 0;


}
void desSerializador(unEntero){

switch (unEntero){

 		case MENSAJE:// recibe mensajes para imprimirlos por pantalla
 		break;
 		case PID: // lo recibe de kernel, al mandarle el proceso nuevo
 		break;
 		case Excepcion: // por falta de memoria
 		break;
 		



}


								

								}