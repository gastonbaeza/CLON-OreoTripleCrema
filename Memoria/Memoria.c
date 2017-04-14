#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#define IP "127.0.0.1"
#define PUERTO "7777"
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

int main(){
int bytesRecibidos,iof;
char* message=malloc(100*sizeof(char));
char *handshakeCliente="Hola soy la Memoria, queria comprar una casa en este terreno";
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(IP, PUERTO, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion



	int serverSocket;
	int flagHandshake=1;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);


	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);



	freeaddrinfo(serverInfo);

		int enviar = 1;
	int i;

	char *messageRecv;

	messageRecv = malloc(100*(sizeof(char)));

	while(flagHandshake){bytesRecibidos=recv(serverSocket,messageRecv,100*sizeof(char),0);
	if(bytesRecibidos>0){flagHandshake=0;fflush(stdout); printf("%s\n", messageRecv);send(serverSocket, handshakeCliente, 60, 0);}
	}

	while(enviar){


		if ((bytesRecibidos=recv(serverSocket,messageRecv,100*sizeof(char),0)) == -1){

			      // llamada a recv()						perror("recv");
			      exit(-1);
						fflush(stdout);
			     	   }

	    printf("%s\n", messageRecv);


	 }

	free(message);
	free(messageRecv);


	close(serverSocket);
	return 0;


}
