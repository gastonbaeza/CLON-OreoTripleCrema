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
#include <fcntl.h>
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
#include <commons/bitarray.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
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
char * bitArray;

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
t_config * cfgMetadata;
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




freeaddrinfo(serverInfo);


if(rv=listen(listenningSocket,BACKLOG)==-1) perror("Error en el listen");
printf("%s \n", "El Servidor se encuentra OK para escuchar conexiones.");

 while((socketKernel = accept(listenningSocket, (struct sockaddr *)&addr,&addrlen)) < 0);
 handshakeServer(socketKernel,FILESYSTEM,(void*)unBuffer);
 t_path *path;

int unBit=0;
char * direccion=calloc(1,200);

bool valorBit;
t_bitarray * bitarray;
t_metadataFS * metadataFS=calloc(1,sizeof(t_metadataFS));

strcpy(direccion,PUNTO_MONTAJE);

direccion=strcat(direccion,"Metadata/Metadata.bin");

/////////////////////////////////leo el metadata///////////////////////////////////////////
cfgMetadata=config_create(direccion); 
metadataFS->tamanioBloques=config_get_int_value(cfgMetadata,"TAMANIO_BLOQUES");printf(" tamanioBloques %i\n",metadataFS->tamanioBloques );
metadataFS->cantBloques=config_get_int_value(cfgMetadata,"CANTIDAD_BLOQUES");printf("cantidad bloques %i\n", metadataFS->cantBloques);
////////////////inicializo el bitarray///////////////////////////////////////////////////

//no tengo idea si ya lo inicializa con todos en 0

//////////////////genero el bitarray.bin/////////////////////////////////////////////////

strcpy(direccion,PUNTO_MONTAJE);
int bitmap1;
int i;
int confirmacion;
int size;
int bloques;
if(metadataFS->cantBloques%8==0)
{bloques=metadataFS->cantBloques;}
else {bloques=metadataFS->cantBloques+1;}

char* bmap;
direccion=strcat(direccion,"Metadata/Bitmap.bin"); 


 FILE * bitmap = fopen(direccion,"wb"); 
if (bitmap!=NULL)
{
	fseek(bitmap, 0L, SEEK_END);
	size = ftell(bitmap);
	if(size<1)
{	
truncate(direccion,bloques/8);
}fclose(bitmap);
}
	bitmap1=open(direccion,O_RDWR);printf("el bitmap es %i\n", bitmap1);
	
	bmap = mmap(0, bloques, PROT_WRITE | PROT_READ, MAP_SHARED, bitmap1, 0); 

	if (bmap == MAP_FAILED) {
			printf("Error al mapear a memoria: %s\n", strerror(errno));
			close(bitmap1);
		return 0;
	}
close(bitmap1);
bitarray = bitarray_create_with_mode(bmap, bloques/8, MSB_FIRST);

/*for ( i = 0; i < bloques-1; ++i)
{	valorBit=bitarray_test_bit(bitarray,i);
	fflush(stdout);printf("%i",(int)valorBit);
}
*/





makeDir("/home/utnso/tp-2017-1c-Oreo-Triple-Crema/hola/chau/archivo.bin",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);


while(1) {
path=calloc(1,sizeof(t_path));
while(0>=recv(socketKernel,seleccionador,sizeof(t_seleccionador),0));

	switch (seleccionador->tipoPaquete){
		case VALIDARARCHIVO:
					
					
					recibirDinamico(PATH,socketKernel,path);
					if( access( path->path, F_OK ) != -1 ) {
						confirmacion=1;
						send(socketKernel,&confirmacion,sizeof(int),0);
						printf("%s \n", "Validado, troesma.\n");
					} else {
						confirmacion=-1;
						send(socketKernel,&confirmacion,sizeof(int),0);
						printf("%s \n", "El archivo no existe o no se encuentra disponible\n");
					}
					free(path);
	
		break;
		case CREARARCHIVO: 
		/*Parámetros: [Path]

		Cuando el Proceso Kernel reciba la operación de abrir un archivo deberá, en caso que el
		archivo no exista y este sea abierto en modo de creación (“c”), llamar a esta operación que
		creará el archivo dentro del path solicitado. Por default todo archivo creado se le debe
		asignar al menos 1 bloque de datos*/

		recibirDinamico(PATH,socketKernel,path);
		FILE * bitmap = fopen(direccion,"wb"); 
		if (bitmap!=NULL)
		{
			printf("%s\n", "fue creado el archivo");
		}
		fclose(bitmap);
		asignarBloques(1,&bitarray,bloques);

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


