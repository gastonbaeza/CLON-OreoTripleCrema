#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define PUERTO "7777"
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

int main(){
int bytesRecibidos;

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(IP, PUERTO, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion



	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);


	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);


	freeaddrinfo(serverInfo);




	int enviar = 1;
	int i;
	char message[100];
	for(i=0;i<100;i++){message[i]='\0';}
	if ((bytesRecibidos=recv(serverSocket,message,100,0)) == -1){
	      /* llamada a recv() */
	      printf("Error en recv() \n");
	      exit(-1);
	     	   }
	else {fflush(stdout);printf("%s %i",message,bytesRecibidos);}

	while(enviar){
		fgets(message, PACKAGESIZE, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		if (!strcmp(message,"exit\n")) enviar = 0;			// Chequeo que el usuario no quiera salir
		if (enviar) send(serverSocket, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
	}




	close(serverSocket);
	return 0;


}
