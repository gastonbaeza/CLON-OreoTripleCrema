#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <commons/config.h>


int main(){
	/* LEER CONFIGURACION
	*
	*/

	t_config *CFG;
	CFG = config_create("/home/utnso/tp-2017-1c-Oreo-Triple-Crema/CPU/cpuCFG.txt");
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
char *handshakeCliente="Hola soy el cpu.";
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


		if ((bytesRecibidos=recv(serverSocket,messageRecv,100*sizeof(char),0)) == -1){

			      // llamada a recv()						perror("recv");
			      exit(-1);
						fflush(stdout);
			     	   }

	    printf("%s\n", messageRecv);

	    /*scanf("%s", message);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		if (!strcmp(message,"exit\n")) enviar = 0;			// Chequeo que el usuario no quiera salir
		if (enviar) send(serverSocket, message,100*sizeof(char), 0); 	// Solo envio si el usuario no quiere salir.
	/*
	 *
	 */}

	free(message);
	free(messageRecv);


	close(serverSocket);
	return 0;


}
