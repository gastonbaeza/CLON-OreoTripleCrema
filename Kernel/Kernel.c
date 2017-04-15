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
#include <commons/config.h>
#define BACKLOG 5

int main(){
	/* LEER CONFIGURACION
	*
	*/
	


	t_config *CFG;
	CFG = config_create("/home/utnso/tp-2017-1c-Oreo-Triple-Crema/Kernel/kernelCFG.txt");
	char *PUERTO_PROG= config_get_string_value(CFG ,"PUERTO_PROG");
	char *PUERTO_CPU= config_get_string_value(CFG ,"PUERTO_CPU");
	char *IP_MEMORIA= config_get_string_value(CFG ,"IP_MEMORIA");
	char *PUERTO_MEMORIA= config_get_string_value(CFG ,"PUERTO_MEMORIA");
	char *IP_FS= config_get_string_value(CFG ,"IP_FS");
	char *PUERTO_FS= config_get_string_value(CFG ,"PUERTO_FS");
	int QUANTUM= config_get_int_value(CFG ,"QUANTUM");
	int QUANTUM_SLEEP= config_get_int_value(CFG ,"QUANTUM_SLEEP");
	char *ALGORITMO= config_get_string_value(CFG ,"ALGORITMO");
	int GRADO_MULTIPROG= config_get_int_value(CFG ,"GRADO_MULTIPROG");
	char* SEM_IDS= config_get_string_value(CFG ,"SEM_IDS");
	char* SEM_INIT= config_get_string_value(CFG ,"SEM_INIT");
	char* SHARED_VARS= config_get_string_value(CFG ,"SHARED_VARS");
	int STACK_SIZE= config_get_int_value(CFG ,"STACK_SIZE");
	printf("Configuración:\nPUERTO_PROG = %s,\nPUERTO_CPU = %s,\nIP_MEMORIA = %s,\nPUERTO_MEMORIA = %s,\nIP_FS = %s,\nPUERTO_FS = %s,\nQUANTUM = %i,\nQUANTUM_SLEEP = %i,\nALGORITMO = %s,\nGRADO_MULTIPROG = %i,\nSEM_IDS = %s,\nSEM_INIT = %s,\nSHARED_VARS = %s,\nSTACK_SIZE = %i.\n"
			,PUERTO_PROG,PUERTO_CPU,IP_MEMORIA,PUERTO_MEMORIA,IP_FS,PUERTO_FS,QUANTUM,QUANTUM_SLEEP,ALGORITMO,GRADO_MULTIPROG,SEM_IDS,SEM_INIT,SHARED_VARS,STACK_SIZE);
	printf("Presione enter para continuar.\n");
	getchar();
	
	/*
	*
	*/
char * bienvenida="Bienevenido, soy el servidor. \n";
char *handshakeCliente=malloc(100*sizeof(char));
int nbytes;
// TODO modularizar, protocolos, crear los logs, cargar las config.
fd_set fdParaConectar, fdParaLeer;
char package[100];
int fdMayor;
int a,listenningSocket, socketNuevaConexion, unSocket,otroSocket, resultadoSelect,addrlen,rv,iof,i;

struct addrinfo hints;
struct addrinfo *serverInfo;

for(i=0;i<100;i++){
	package[i]='\0';
}

memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_flags = AI_PASSIVE;
hints.ai_socktype = SOCK_STREAM;

if ((rv =getaddrinfo(NULL, PUERTO_PROG, &hints, &serverInfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
}


fflush(stdout);
printf("%s \n", "El Servidor esta configurado.\n");
sleep(1);



listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
sleep(1);
fflush(stdout);
printf("%s \n", "Socket Listo.");


if(bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen)==-1)
			{perror("Error en el bind."); exit(1);}
fflush(stdout);
sleep(1);
printf("%s \n", "Bind Listo.\n");


freeaddrinfo(serverInfo);
sleep(1);
fflush(stdout);
config_destroy(CFG);



clear();
printf("%s \n", "El Servidor se encuentra listo para escuchar conexiones.");
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
	         perror("Error en el select.\n");
	         exit(1);
	     	 }



	    for(unSocket = 0; unSocket <= fdMayor; unSocket++) // Busca en las conexiones alguna que mande un request
	    				{

	    				if (FD_ISSET(unSocket, &fdParaLeer))
	    						{  //si hay alguna con request...
	    						if (unSocket == listenningSocket) {
	    													// se manejan las nuevas conexiones a partir de los SETS
	    													addrlen = sizeof(addr);
	    													if ((socketNuevaConexion = accept(listenningSocket, (struct sockaddr *)&addr,&addrlen)) == -1) { perror("accept");}
	    													else {
	    														FD_SET(socketNuevaConexion, &fdParaConectar);
	    														printf("Hay una nueva conexion de %s en el socket %i.\n", inet_ntoa(addr.sin_addr), socketNuevaConexion);


	    														send(socketNuevaConexion, bienvenida, 100*sizeof(char), 0);
	    														recv(socketNuevaConexion,handshakeCliente,100*sizeof(char),0); fflush(stdout); printf("%s \n",handshakeCliente);

	    														// lo mando a la "cola" de pendientes de conexion
	    														if (socketNuevaConexion > fdMayor) {fdMayor = socketNuevaConexion;}

	    														}
	    													}
	    						else {
	    							// tramito los request del cliente
	    							if ((nbytes = recv(unSocket, package, 100*sizeof(char), 0)) <= 0)
	    												{
	    												if (nbytes == 0) {printf("El cliente %i se ha desconectado.\n", unSocket);}
	    												else {perror("Error en el recv.\n");} fflush(stdout); printf("Hubo algun tipo de error.\n");
	    												close(unSocket);
	    												FD_CLR(unSocket, &fdParaConectar); // lo saco de los pendientes de conexion
														}
	    							else {
	    									// tenemos datos de algún cliente
	    									printf("%s\n",package );
	    									for(otroSocket = 0; otroSocket <= fdMayor; otroSocket++) {
	    									// ESTO HAY QUE CAMBIARLO PARA LOS PROTOCOLOS; SINO ES UN ECHO MULTICLIENTE
	    											if (FD_ISSET(otroSocket, &fdParaConectar)) {

	    															if (otroSocket != listenningSocket && otroSocket != unSocket) {
	    																		if (send(otroSocket, package, 100*sizeof(char), 0) == -1) {perror("Error en el send.");}

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
