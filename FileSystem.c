#define clear() printf("\033[H\033[J")
#define CREARARCHIVO 1
#define BORRARARCHIVO 2 
#define OBTENERDATOS 3
#define GUARDARDATOS 4
#include "estructuras.h"
#include "funciones.h"
#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/txt.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stddef.h>
#include <math.h>
#include <stdint.h>
#include <sys/stat.h>
#include <commons/config.h>
#include <assert.h>
#define FILESYSTEM 84
#define BACKLOG 5
#define LIBRE 0
#define OCUPADO 1
#define KERNEL 0
	#define ARRAYPIDS 5
	#define PIDFINALIZACION 2
	#define PATH 10
	#define PIDINFO 4
	#define RESPUESTAOKMEMORIA 1

	#define SOLICITUDMEMORIA 0
	#define SOLICITUDINFOPROG 1
	#define ESCRIBIRMEMORIA 2
	#define LIBERARMEMORIA 3
 	#define ACTUALIZARPCB 4
	
	//-------------------------------
	#define DELAY 0
	#define DUMP 1
	#define FLUSH 2
	#define SIZE 3
#define CONSOLA 2
	#define INICIARPROGRAMA 0
	#define FINALIZARPROGRAMA 1
	#define DESCONECTARCONSOLA 2
	#define LIMPIARMENSAJES 3
	//------------------------------	
	#define MENSAJES 0
	#define PIDNUEVO 1

#define CPU 3
#define FS 4
#define TRUE 1
#define FALSE 0
#define OK 1
#define FAIL 0
#define BLOQUE 20

#define CACHE 0
#define MEMORIA 1 
#define TABLA 2
#define PID 3
#define PCB 3
#define PIDSIZE 1
#define MEMORIASIZE 0
#define VALIDARARCHIVO 58

int makeDir(char *fullpath, mode_t permissions){
int i=0;
char *arrDirs[20];
char aggrpaz[255];
char * path=calloc(1,255);
memcpy(path,fullpath,strlen(fullpath)+1);
arrDirs[i] = strtok(path,"/");
strcpy(aggrpaz, "/");
while(arrDirs[i]!=NULL)
{
    arrDirs[++i] = strtok(NULL,"/");
    strcat(aggrpaz, arrDirs[i-1]);
    mkdir(aggrpaz,permissions);
    strcat(aggrpaz, "/");
}
i=0;
return 0;
}


int main(){
char* PUERTO,*PUNTO_MONTAJE;
t_config *CFG;
CFG = config_create("fileSystemCFG.txt");
PUERTO= config_get_string_value(CFG ,"PUERTO");
PUNTO_MONTAJE= config_get_string_value(CFG ,"PUNTO_MONTAJE");
printf("Configuración:\nPUERTO = %s\nPUNTO_MONTAJE = %s\n",PUERTO,PUNTO_MONTAJE);

struct sockaddr_in addr;
int addrlen= sizeof(addr);
int socketKernel,rv;
struct addrinfo hints;
struct addrinfo *serverInfo;
int listenningSocket;
int * unBuffer=malloc(sizeof(int));
t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));

memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_flags = AI_PASSIVE;
hints.ai_socktype = SOCK_STREAM;

if ((rv =getaddrinfo(NULL, PUERTO, &hints, &serverInfo)) != 0) fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));

listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
sleep(1);
fflush(stdout);
printf("%s \n", "Socket OK");

if(bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen)==-1) {perror("Error en el bind."); exit(1);}

fflush(stdout);
sleep(1);
printf("%s \n", "Bind OK\n");

makeDir("/home/utnso/tp-2017-1c-Oreo-Triple-Crema/hola/chau",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);


freeaddrinfo(serverInfo);


if(rv=listen(listenningSocket,BACKLOG)==-1) perror("Error en el listen");
printf("%s \n", "El Servidor se encuentra OK para escuchar conexiones.");

 while((socketKernel = accept(listenningSocket, (struct sockaddr *)&addr,&addrlen)) < 0);
 handshakeServer(socketKernel,FILESYSTEM,(void*)unBuffer);
 t_path *path;

 

while(1) {

while(0>=recv(socketKernel,seleccionador,sizeof(t_seleccionador),0));

	switch (seleccionador->tipoPaquete){
		case VALIDARARCHIVO:
					path=malloc(sizeof(t_path));
					path->path=calloc(1,150);
					recibirDinamico(PATH,socketKernel,path);
					if( access( path->path, F_OK ) != -1 ) {
						//FALTA COMUNICARSELO AL KERNEL
						printf("%s \n", "Validado, troesma.\n");
					} else {
						printf("%s \n", "El archivo no existe o no se encuentra disponible\n");
					}
					free(path);
	
		break;
		case CREARARCHIVO: printf("%s \n", "Creado, mostro.\n");
		/*Parámetros: [Path]
		Cuando el Proceso Kernel reciba la operación de abrir un archivo deberá, en caso que el
		archivo no exista y este sea abierto en modo de creación (“c”), llamar a esta operación que
		creará el archivo dentro del path solicitado. Por default todo archivo creado se le debe
		asignar al menos 1 bloque de datos*/
		break;
		case BORRARARCHIVO: printf("%s \n", "Borrado, papu.\n");
		/*Parámetros: [Path]
		Borrará el archivo en el path indicado, eliminando su archivo de metadata y marcando los
		bloques como libres dentro del bitmap*/
		break;
		case OBTENERDATOS: printf("%s \n", "Tomá, comela.\n");
		/*Parámetros: [Path, Offset, Size]
		Ante un pedido de datos el File System devolverá del path enviado por parámetro, la
		cantidad de bytes definidos por el Size a partir del offset solicitado. Requiere que el archivo
		se encuentre abierto en modo lectura (“r”)*/
		break;
		case GUARDARDATOS: printf("%s \n", "Guardados, lince.\n");
		/*Parámetros: [Path, Offset, Size, Buffer]
		Ante un pedido de escritura el File System almacenará en el path enviado por parámetro, los
		bytes del buffer, definidos por el valor del Size y a partir del offset solicitado. Requiere que el
		archivo se encuentre abierto en modo escritura (“w”)*/
		break;
		default: printf("%s \n", "Numero incorrecto\n");
		break;
			}	/*En caso de que se soliciten datos o se intenten guardar datos en un archivo inexistente,
				 el File System deberá retornar un error de Archivo no encontrado*/
		}	
		free(seleccionador);
 		
return 0;
}


