#include "desSerializador.h"
#include "estructuras.h"
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
#include <pthread.h>
#include <commons/config.h>
#define BACKLOG 5
extern pcb* procesos;




void comunicarse(dataParaComunicarse){
	int nbytes,status;
	void * paquete;
	int * confirmacion;
	t_header * header;
	char * hashDesSerializador=maloc(33*sizeof(char));
	while(1){
		if ((nbytes = recv(unDataParaComunicarse.socket, header, 100*sizeof(char), 0)) <= 0) { // error o conexión cerrada por el cliente
            if (nbytes == 0) printf("selectserver: socket %d hung up\n", i); // conexión cerrada 
            else perror("recv");
            break;
    	}
        hashDesSerializador = desSerializador(header.seleccionador.unaInterfaz,header.seleccionador.tipoPaquete,dataParaComunicarse.socket, paquete);
        send(dataParaComunicarse.socket,hashDeserializador,33*sizeof(char),0); //agregar la logica de que pasa si los hashes no son iguaels
        switch(header->seleccionador.tipoPaquete){
		case 0:// hacer eco a consola
			break;
			case 1:// mandamos un mensaje a memoria con el tamanio de nuestro programa preguntando si puede guardarlo
				
				int id = 0,tamanioAReservar;
				int rv;
				char * path = (char *)paquete;
				char * buffer = malloc(sizeof(char)*33);
				t_programaSalida * programaDePath;
			
				while(*procesosEnEjecucion != '\0') if (id == procesosEnEjecucion[id]->pid) id++;
				if (send(dataParaComunicarse.socket, id, sizeof(int), 0) == -1) perror("send");
				programaDePath = obtenerPrograma(path);
				tamanioAReservar = programaDePath->tamanio;
				struct addrinfo hints;
				struct addrinfo *serverInfo;
				memset(&hints, 0, sizeof(hints));
				hints.ai_family = AF_INET;
				hints.ai_socktype = SOCK_STREAM;
				getaddrinfo(dataParaComunicarse.ipMemoria,dataParaComunicarse.puertoMemoria,&hints,&serverInfo);
				int socketMemoria;
				socketMemoria = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
				if ((connect(socketMemoria, serverInfo->ai_addr, serverInfo->ai_addrlen)) == -1)
				 {
		            perror("Error de conexión con la memoria");
		            break;
        		}
        		hashSerializador=serializador(KERNEL,SOLICITUDMEMORIA,socketMemoria,&tamanioAReservar);
        		
        		status=recv(socketMemoria,buffer,sizeof(char)*33,0);
        		
        		status=recv(socketMemoria,confirmacion,sizeof(int),0);
        		if (confirmacion)//==1 esta todo OK  ==0 recibo exception 
        		{
        			status=send(socketMemoria,programaDePath->elPrograma,programaDePath->tamanio,0);
        		} else {

        			status=recv(socketMemoria,header,sizeof(header),0);
        			hashDesSerializador=desSerializador(KERNEL,EXCEPCION,socketMemoria,(void*)paquete);
 					
 					send(sockerMemoria,hashDeserializador,33*sizeof(char),0);
 					tamanioPaquete=strlen((char*)paquete);
 					char*mensajeConsola= malloc(tamanioPaquete);
 					strcpy(mensajeConsola,paquete);
 					hashSerializador=serializador(CONSOLA,MENSAJECONSOLA,dataParaComunicarse->socket,(void*)mensajeConsola)}
        		


			break;
			case 2:
			break;
			case 3:// tenemos que buscar el hilo de ese pid y terminarlo
			break;
			case 4:
			break;
			case 5:
			break;
			case 6:
			break;
			case 7:
			break;
			case 8:// como la memoria no tiene espacio, tenemos que dejar el nuevo proceso en bloqueado, hasta que alguien libere memoria
			break;
			case 9:
			break;
			case 10:// 
			break;
			}
	}
}



// fflush(stdout); 
// printf("%s \n",handshakeCliente);//HASTA ACA HACE EL HILO
// estadoRecv= recv(unDataParaComunicarse.socket, header, sizeof(t_header),0); // agregar caso excepcion recv
// hashDesSerializador = desSerializador((header.seleccionador).unaInterfaz,(header.seleccionador).tipoPaquete,unDataParaComunicarse.socket, paquete);
// send(unDataParaComunicarse.socket,hashDesSerializador,33*sizeof(char),0); // enviamos el hash y le delegamos al que envia la responsabilidad del reenvio si fue erroneo.
// //recibir confirmacion de que el paquete fue enviado bien
// //derivar el trabajo que tiene que hacer el kernel  con el mismo header por ej si recibio un path lo siguiente seria consultar si entra en la pag de memoria
// 	if ((header.seleccionador).unaInterfaz==0)


// while(1)
// {
// if ((nbytes = recv(unSocket, tipoDato, sizeof(int), 0)) <= 0)
// {
// 	if (nbytes == 0) printf("El cliente %i se ha desconectado.\n", unSocket);
// 	else perror("Error en el recv.\n"); 
// 	fflush(stdout); printf("Hubo algun tipo de error.\n");
// 	close(unSocket);
// 	return 0;
// }
// else {

// 	recv(unSocket, tamanioDato, sizeof(long int), 0);
// 	desSerializador(tipoDato,tamanioDato);

// }
// }


int main(){
	
/* LEER CONFIGURACION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
t_config *CFG;
CFG = config_create("kernelCFG.txt");
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
/* LEER CONFIGURACION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/

char * bienvenida="Bienevenido, soy el kernel. \n";
char *handshakeCliente=malloc(100*sizeof(char));
int nbytes;
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

if ((rv =getaddrinfo(NULL, PUERTO_PROG, &hints, &serverInfo)) != 0) fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));

fflush(stdout);
printf("%s \n", "El Servidor esta configurado.\n");
sleep(1);
listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
sleep(1);
fflush(stdout);
printf("%s \n", "Socket Listo.");

if(bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen)==-1) perror("Error en el bind."); exit(1);

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
	if ( resultadoSelect== -1){
		perror("Error en el select.\n");
		exit(1);
	}
	for(unSocket = 0; unSocket <= fdMayor; unSocket++) { // Busca en las conexiones alguna que mande un request
	    if (FD_ISSET(unSocket, &fdParaLeer)) {  //si hay alguna con request...
	    	if (unSocket == listenningSocket) { // se manejan las nuevas conexiones a partir de los SETS
	    		addrlen = sizeof(addr);
	    		if ((socketNuevaConexion = accept(listenningSocket, (struct sockaddr *)&addr,&addrlen)) == -1) perror("accept");
	    		else {
	    			dataParaComunicarse dataParaThread;
	    			dataParaThread.socket=socketNuevaConexion;
					FD_SET(dataParaThread.socket, fdParaConectar);
					printf("Hay una nueva conexion de %s en el socket %i.\n", inet_ntoa(address.sin_addr), dataParaThread.socket);
					send(dataParaThread.socket, bienvenida, 100*sizeof(char), 0);
					recv(dataParaThread.socket, unaInterfaz, sizeof(int),0);
					dataParaThread.unaInterfaz = unaInterfaz;
					dataParaThread.puertoMemoria = PUERTO_MEMORIA;
					dataParaThread.ipMemoria = IP_MEMORIA;
	    			pthread_t hilo;
	    			pthread_create(&hilo,NULL,(void *)comunicarse,&dataParaThread); //pasar por parametro
	    			if (socketNuevaConexion > fdMayor) fdMayor = socketNuevaConexion;
	    			}
	    	}
	    	else { // tramito los request del cliente
	    		if ((nbytes = recv(unSocket, package, 100*sizeof(char), 0)) <= 0) {
	    			if (nbytes == 0) printf("El cliente %i se ha desconectado.\n", unSocket);
	    			else perror("Error en el recv.\n"); 
	    			fflush(stdout); 
	    			printf("Hubo algun tipo de error.\n");
	    			close(unSocket);
	    			FD_CLR(unSocket, &fdParaConectar); // lo saco de los pendientes de conexion
				}
	    		else { // tenemos datos de algún cliente
	    			printf("%s\n",package );
	    			for(otroSocket = 0; otroSocket <= fdMayor; otroSocket++) { // ESTO HAY QUE CAMBIARLO PARA LOS PROTOCOLOS; SINO ES UN ECHO MULTICLIENTE
	    				if (FD_ISSET(otroSocket, &fdParaConectar)) {
							if (otroSocket != listenningSocket && otroSocket != unSocket) {
	    						if (send(otroSocket, package, 100*sizeof(char), 0) == -1) perror("Error en el send.");
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
	    

	
void enviarPCBCpu(pcb *, int socketCPU){
	send(socketKernel,pcb *, 100*sizeof(pcb), 0)
}