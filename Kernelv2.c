#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/txt.h>
#include <errno.h>
#define PUERTO "7777"
#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar
#define clear() printf("\033[H\033[J")
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
int main(){
char * bienvenida="Bienevenido a jardines marvin, soy el señor servidor";
fflush(stdout); printf("%s",bienvenida);
int status = 1;
int nbytes;
// TODO modularizar, protocolos, crear los logs, cargar las config.
fd_set fdParaConectar, fdParaLeer;
char package[100];
int fdMayor;
int a,listenningSocket, socketNuevaConexion, j,n, resultadoSelect, result1, addrlen;

struct addrinfo hints;
struct addrinfo *serverInfo;

memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_UNSPEC;
hints.ai_flags = AI_PASSIVE;
hints.ai_socktype = SOCK_STREAM;

getaddrinfo(NULL, PUERTO, &hints, &serverInfo);
fflush(stdout);
printf("%s \n", "El server esta configurado");
sleep(1);



listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
sleep(1);
fflush(stdout);
printf("%s \n", "Socket Listo");
	
	
if(bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen)==-1)
			{perror("la manqueo el bind"); exit(1);}
fflush(stdout);
sleep(1);
printf("%s \n", "bind Listo");


freeaddrinfo(serverInfo);
sleep(1);
fflush(stdout);



printf("%s \n", "El server se encuentra listo para escuchar conexiones");
fflush(stdout);


listen(listenningSocket, BACKLOG);

struct sockaddr_in addr;


FD_ZERO(&fdParaConectar);
FD_SET(listenningSocket, &fdParaConectar);

fdMayor = listenningSocket;
for(;;) {
	     fdParaLeer = fdParaConectar;
	     resultadoSelect=select(fdMayor+1, &fdParaLeer, NULL, NULL, NULL);
	     if ( resultadoSelect== -1)
	     	 {
	         perror("la manqueo el select");
	         exit(1);
	     	 }



	    for(j = 0; j <= fdMayor; j++) // Busca en las conexiones alguna que mande un request
	    				{

	    				if (FD_ISSET(j, &fdParaLeer))
	    						{  //si hay alguna con request...
	    						if (j == listenningSocket) {
	    													// se manejan las nuevas conexiones a partir de los SETS
	    													addrlen = sizeof(addr);
	    													if ((socketNuevaConexion = accept(listenningSocket, (struct sockaddr *)&addr,&addrlen)) == -1) { perror("accept");}
	    													else {
	    														FD_SET(socketNuevaConexion, &fdParaConectar); // lo mando a la "cola" de pendientes de conexion
	    														if (socketNuevaConexion > fdMayor) {fdMayor = socketNuevaConexion;}
	    														printf("Server dice:  hay una nueva conexion de %s en el socket %i\n", inet_ntoa(addr.sin_addr), socketNuevaConexion);
	    														send(socketNuevaConexion, bienvenida, 100, 0);
	    														}
	    													}
	    						else {
	    							// tramito los request del cliente
	    							if ((nbytes = recv(j, package, 100, 0)) <= 0)
	    												{

	    												if (nbytes == 0) {printf("El cliente %i se cayo  \n", j);}
	    												else {perror("la manqueo el recv");} fflush(stdout); printf("hubo algun tipo de error");
	    												close(j);
	    												FD_CLR(j, &fdParaConectar); // lo saco de los pendientes de conexion
	    												}
	    							else {
	    									// tenemos datos de algún cliente
	    									for(n = 0; n <= fdMayor; n++) {
	    									// ESTO HAY QUE CAMBIARLO PARA LOS PROTOCOLOS; SINO ES UN ECHO MULTICLIENTE
	    											if (FD_ISSET(n, &fdParaConectar)) {

	    															if (n != listenningSocket && n != j) {
	    																		if (send(n, package, nbytes, 0) == -1) {perror("la manqueo el send");}

	    																								 }
	    																				}
	    																	}
	    								}
	    							}
	                }
	            }
	        }

	        return 0;
	    }


	

