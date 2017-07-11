#define clear() printf("\033[H\033[J")
#define CREARARCHIVO 1
#define BORRARARCHIVO 2 
#define OBTENERDATOS 3
#define GUARDARDATOS 4
#include "estructuras.h"
#include "funciones.h"
#include <commons/collections/list.h>
#include <commons/string.h>
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
#include <linux/limits.h>
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

#define CREARARCHIVOFS 64
#define GUARDARDATOSFS 65
#define OBTENERDATOSFS 66
#define PAQUETEFS 67
#define BORRARARCHIVOFS 68
char * PUNTO_MONTAJE;
char * BLOQUES_MONTAJE;
char * ARCHIVOS_MONTAJE;
char * METADATA_MONTAJE;
char * bitArray;
char * listaBloques;
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
free(path);
i=0;
return 0;
}
int enQueBloqueEstoy(int*offset, int*tamanioBloque){
	int numeroBloque= *offset/(*tamanioBloque);
	return numeroBloque;
}

int deDondeEmpiezoALeer(int*offset, int*tamanioBloque){
	int inicioLectura= *offset%(*tamanioBloque);
	return inicioLectura;
}
void strip(char** string){
	int i ;
	for(i=0;i<string_length(*string); i++){
		if((*string)[i]=='\n' || (*string)[i]=='\r' )
			(*string)[i]='\0';
	}
}
char* obtenerBloque(char**BLOQUES, int*numeroBloque){
	char* bloque=BLOQUES[*numeroBloque];
	return bloque;
}
FILE* getSiguiente(char**BLOQUES,int index){
	char* numeroBloque=BLOQUES[index];
	char* bloqueConBin=calloc(1,200);
	strcpy(bloqueConBin,numeroBloque);
	bloqueConBin=strcat(bloqueConBin,".bin");
	char*rutaParcial=calloc(1,200);
	strcpy(rutaParcial,PUNTO_MONTAJE);
	rutaParcial=strcat(rutaParcial,"Bloques/");
	char*rutaFinal=calloc(1,200);
	strcpy(rutaFinal,rutaParcial);
	rutaFinal=strcat(rutaFinal,bloqueConBin);
	FILE*f=fopen(rutaFinal,"rb");
	free(bloqueConBin);
	free(rutaParcial);
	free(rutaFinal);
	return f;
}


int cantidadElementos(char* cadena){
	int cont=0,i=0;
	while(cadena[i]!=']')
		{
			if (cadena[i]==',')
				cont++;
			i++;
		}
	return cont+1;
}
int main(){
char* PUERTO;
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
fflush(stdout);
printf("%s \n", "Socket OK");

if(bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen)==-1) {perror("Error en el bind."); exit(1);}

fflush(stdout);
printf("%s \n", "Bind OK\n");




freeaddrinfo(serverInfo);


if(rv=listen(listenningSocket,BACKLOG)==-1) perror("Error en el listen");
printf("%s \n", "El Servidor se encuentra OK para escuchar conexiones.");

 while((socketKernel = accept(listenningSocket, (struct sockaddr *)&addr,&addrlen)) < 0);
 handshakeServer(socketKernel,FILESYSTEM,(void*)unBuffer);
 free(unBuffer);
 t_path *path;

int unBit=0;
char * direccion=calloc(1,200);

bool valorBit;
t_bitarray * bitarray;
t_metadataFS * metadataFS=calloc(1,sizeof(t_metadataFS));

strcpy(direccion,PUNTO_MONTAJE);

direccion=strcat(direccion,"/Metadata/Metadata.bin");printf("dir del metadata%s\n",direccion );

/////////////////////////////////leo el metadata///////////////////////////////////////////
cfgMetadata=config_create(direccion); 
metadataFS->tamanioBloques=config_get_int_value(cfgMetadata,"TAMANIO_BLOQUES");printf(" tamanioBloques %i\n",metadataFS->tamanioBloques );
metadataFS->cantBloques=config_get_int_value(cfgMetadata,"CANTIDAD_BLOQUES");printf("cantidad bloques %i\n", metadataFS->cantBloques);
////////////////inicializo el bitarray///////////////////////////////////////////////////

//no tengo idea si ya lo inicializa con todos en 0

//////////////////genero el bitarray.bin/////////////////////////////////////////////////
printf("%s\n","comence a generar el bitarray" );
strcpy(direccion,PUNTO_MONTAJE);
ARCHIVOS_MONTAJE=calloc(1,PATH_MAX);
strcpy(ARCHIVOS_MONTAJE,PUNTO_MONTAJE);
strcat(ARCHIVOS_MONTAJE,"/Archivos");
METADATA_MONTAJE=calloc(1,PATH_MAX);
strcpy(METADATA_MONTAJE,PUNTO_MONTAJE);
strcat(METADATA_MONTAJE,"/Metadata");
BLOQUES_MONTAJE=calloc(1,PATH_MAX);
strcpy(BLOQUES_MONTAJE,PUNTO_MONTAJE);
strcat(BLOQUES_MONTAJE,"/Bloques");
int bitmap1;
int i;
int confirmacion;
int size;
int bloques;
int tamBloques=metadataFS->tamanioBloques;
if(metadataFS->cantBloques%8==0)
{bloques=metadataFS->cantBloques;}
else {bloques=metadataFS->cantBloques+1;}

char* bmap;
direccion=strcat(direccion,"/Metadata/Bitmap.bin"); 


 FILE * bitmap = fopen(direccion,"wb"); 
if (bitmap!=NULL)
{
	fseek(bitmap, 0L, SEEK_END);
	size = ftell(bitmap);
	if(size<1)
	{truncate(direccion,bloques/8);}
	fclose(bitmap);
}
else{printf("%s\n","que carajos esta pasando" );}
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

char * soloDir;
int * bloquesAsignados;
char * pegador=calloc(1,200);


strcpy(direccion,PUNTO_MONTAJE);

int index,bloquesNecesarios,offset;
char*auxCant;
char* bloqueDondeEstoy,*bloquesAsignadosChar;
char* bloqueConBin;
char*rutaParcial;
char*rutaFinal;
char*bloquesArray,*bloquesFinal;
int tamanio;
char*unBloque;
int TAMANIO,totalBloques;
int cantidadBloques,primerBloqueAEscribir,cantAPedir,offsetAEscribir;
t_paqueteFS* resultado;
char * auxChar,*ptoMont;
int sizeAux;
int restoUltimoBloque;
void*auxResultado;
int valorRespuesta;
t_config * CFGarchivo;		
FILE*f;
char**BLOQUES;
t_escribirArchivoFS * escribirArchivoFS;
t_leerArchivoFS * leerArchivoFS;
while(1){
	while(0>recv(socketKernel,seleccionador,sizeof(t_seleccionador),0));
	
		switch (seleccionador->tipoPaquete)
		{
			case VALIDARARCHIVO:
						
						
						path=calloc(1,sizeof(t_path));
						recibirDinamico(VALIDARARCHIVO,socketKernel,path);
						auxChar=calloc(1,PATH_MAX);
						strip(&ARCHIVOS_MONTAJE);
						strcpy(auxChar,ARCHIVOS_MONTAJE);
						strip(&path->path);
						if( access( strcat(auxChar,path->path), F_OK ) != -1 ) {
							confirmacion=1;
							printf("%s \n", "Validado, troesma.\n");
						} else {
							confirmacion=-1;
							printf("%s \n", "El archivo no existe o no se encuentra disponible\n");
						}
						send(socketKernel,&confirmacion,sizeof(int),0);
						free(path->path);
						free(path);
						free(auxChar);
		
			break;
			case CREARARCHIVOFS: 
			/*Parámetros: [Path]

			Cuando el Proceso Kernel reciba la operación de abrir un archivo deberá, en caso que el
			archivo no exista y este sea abierto en modo de creación (“c”), llamar a esta operación que
			creará el archivo dentro del path solicitado. Por default todo archivo creado se le debe
			asignar al menos 1 bloque de datos*/
			
			path=calloc(1,sizeof(t_path));
			recibirDinamico(CREARARCHIVOFS,socketKernel,path);
			strip(&path->path);
			direccion=strcat(direccion,"/Archivos"); printf("direccion archivos %s\n",direccion );
			direccion=strcat(direccion,path->path);
			ultimoDirectorio(direccion,&soloDir); printf("%s\n",soloDir ); 

			makeDir(soloDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			asignarBloques(&bloquesAsignados,1,&bitarray,bloques);
			enlistadorDeBloques(&listaBloques,bloquesAsignados,1);
			crearBloques(listaBloques,PUNTO_MONTAJE,tamBloques);
			strcpy(pegador,"BLOQUES=");
			listaBloques=strcat(pegador,listaBloques);printf("lo que va en la conf %s\n",listaBloques );
			
			 bitmap= fopen(direccion,"ab+"); printf("final 4k %s\n",direccion );
			if (bitmap!=NULL)
			{	fseek(bitmap,0,SEEK_END);
				fwrite("TAMANIO=0",strlen("TAMANIO=0"),1,bitmap);
				fwrite("\n",1,1,bitmap);
				fwrite(listaBloques,strlen(listaBloques),1,bitmap);
				fclose(bitmap);
				printf("%s\n", "fue creado el archivo");
			}
			else{perror ("Error opening threshold file");
				printf("%s\n","se rompio algo" );}
			free(path->path);
			free(path);
			free(soloDir);
			free(bloquesAsignados);
			free(listaBloques);

			break;
			case BORRARARCHIVO: printf("%s \n", "Borrado, papu.\n");
			/*Parámetros: [Path]
			Borrará el archivo en el path indicado, eliminando su archivo de metadata y marcando los
			bloques como libres dentro del bitmap*/
				path=malloc(sizeof(t_path));
				recibirDinamico(BORRARARCHIVO,socketKernel,path);
				auxChar=calloc(1,PATH_MAX);
				strcpy(auxChar,ARCHIVOS_MONTAJE);
				strcat(auxChar,"/");
				strcat(auxChar,path->path);
				CFG = config_create(auxChar);
				TAMANIO= config_get_int_value(CFG ,"TAMANIO");
				BLOQUES= config_get_array_value(CFG ,"BLOQUES");
				auxCant=config_get_string_value(CFG,"BLOQUES");
				cantidadBloques=cantidadElementos(auxCant);
				for(i=0;i<cantidadBloques;i++){
				bitarray_clean_bit(bitarray,atoi(BLOQUES[i]));
				}
				if( remove(auxChar) != 0 )

					perror("Error al borrar el archivo");
				else
					printf("Borrado, papu.");
				free(path->path);
				free(path);
				free(auxChar);
				config_destroy(CFG);
			break;
			
			case OBTENERDATOSFS: printf("%s \n", "Tomá, comela.\n");
			/*Parámetros: [Path, Offset, Size]
			Ante un pedido de datos el File System devolverá del path enviado por parámetro, la
			cantidad de bytes definidos por el Size a partir del offset solicitado. Requiere que el archivo
			se encuentre abierto en modo lectura (“r”)*/
				leerArchivoFS=malloc(sizeof(t_leerArchivoFS));
				recibirDinamico(OBTENERDATOSFS,socketKernel,leerArchivoFS);
				printf("leerArchivoFS->path: %s.\n", leerArchivoFS->path);
				auxChar=calloc(1,PATH_MAX);
				strcpy(auxChar,ARCHIVOS_MONTAJE);
				strcat(auxChar,"/");
				strcat(auxChar,leerArchivoFS->path);
				CFG = config_create(auxChar);
				printf("llegamos aca: %p\n",CFG);
				TAMANIO= config_get_int_value(CFG ,"TAMANIO");
				printf("tamaño: %i.\n", TAMANIO);
				BLOQUES= config_get_array_value(CFG ,"BLOQUES");
				printf("config leido\n");
				bloqueDondeEstoy=calloc(1,200);
				index=enQueBloqueEstoy(&(leerArchivoFS->offset),&tamBloques);
				bloqueDondeEstoy=obtenerBloque(BLOQUES,&index);
				bloqueConBin=calloc(1,200);
				strcpy(bloqueConBin,bloqueDondeEstoy);
				bloqueConBin=strcat(bloqueConBin,".bin");
				rutaParcial=calloc(1,PATH_MAX);
				strcpy(rutaParcial,BLOQUES_MONTAJE);
				strcat(rutaParcial,"/");
				rutaFinal=calloc(1,PATH_MAX);
				strcpy(rutaFinal,rutaParcial);
				rutaFinal=strcat(rutaFinal,bloqueConBin);
				printf("ruta final: %s.\n", rutaFinal);
				FILE*f=fopen(rutaFinal,"rb");
				offset=deDondeEmpiezoALeer(&(leerArchivoFS->offset),&tamBloques);
				fseek(f,deDondeEmpiezoALeer(&(leerArchivoFS->offset),&tamBloques),SEEK_SET);
				resultado=calloc(1,sizeof(t_paqueteFS));
				printf("leerArchivoFS->size: %i.\n", leerArchivoFS->size);
				resultado->paquete=malloc(leerArchivoFS->size);
				auxResultado=resultado->paquete;
				if (offset+leerArchivoFS->size<tamBloques)
				{
					fread(auxResultado,leerArchivoFS->size,1,f);
					resultado->tamanio=leerArchivoFS->size;
				}
				else{
					fread(auxResultado,tamBloques-offset,1,f);
					auxResultado+=(tamBloques-offset);
					resultado->tamanio=tamBloques-offset;
				}
				fclose(f);
				while(resultado->tamanio<leerArchivoFS->size){
					index++;
					f=getSiguiente(BLOQUES,index);

					if (leerArchivoFS->size-resultado->tamanio<tamBloques)
					{
						fread(auxResultado,leerArchivoFS->size-resultado->tamanio,1,f);
						resultado->tamanio+=(leerArchivoFS->size-resultado->tamanio);
					}
					else{
						fread(auxResultado,tamBloques,1,f);
						auxResultado+=tamBloques;
						resultado->tamanio+=tamBloques;
					}
					fclose(f);
				}
				i=1;
				printf("aca\n");
				send(socketKernel,&i,sizeof(int),0);
				unBuffer=malloc(sizeof(int));
				*unBuffer=1;
				while(0>recv(socketKernel,unBuffer,sizeof(int),0));
				free(unBuffer);
				printf("kk\n");
				printf("resultado->paquete: %s\n",(char*)resultado->paquete);
				printf("resultado->tamaño: %i.\n", resultado->tamanio);
				if (i==1)
				{
					enviarDinamico(PAQUETEFS,socketKernel,resultado);
				}
				printf("kk2\n");
				free(leerArchivoFS->path);
				free(leerArchivoFS);
				free(auxChar);
				free(bloqueDondeEstoy);
				free(resultado->paquete);
				free(resultado);
				free(bloqueConBin);
				free(rutaParcial);
				free(rutaFinal);
				config_destroy(CFG);

			break;
			case GUARDARDATOSFS: printf("%s \n", "Guardados, lince.\n");
			/*Parámetros: [Path, Offset, Size, Buffer]
			Ante un pedido de escritura el File System almacenará en el path enviado por parámetro, los
			bytes del buffer, definidos por el valor del Size y a partir del offset solicitado. Requiere que el
			archivo se encuentre abierto en modo escritura (“w”)*/
			escribirArchivoFS=calloc(1,sizeof(t_escribirArchivoFS));
			recibirDinamico(GUARDARDATOSFS,socketKernel,escribirArchivoFS);
			printf("escribir:\n");
			printf("path: %s\n", escribirArchivoFS->path);
			printf("buffer: %s\n", (char*)escribirArchivoFS->buffer);
			printf("size: %i\n", escribirArchivoFS->size);
			printf("offset: %i\n",escribirArchivoFS->offset);
			auxChar=calloc(1,PATH_MAX);
			strip(&ARCHIVOS_MONTAJE);
			strcpy(auxChar,ARCHIVOS_MONTAJE);
			strip(&escribirArchivoFS->path);
			strcat(auxChar,escribirArchivoFS->path);
			strip(&auxChar);
			printf("La ruta del archivo es: %s\n", auxChar);
			CFG = config_create(auxChar);
			rv=1;
				// TAMANIO= config_get_int_value(CFG ,"TAMANIO");
				bloquesArray= config_get_string_value(CFG ,"BLOQUES");
				printf("Actualmente el archivo tiene los bloques: %s\n",bloquesArray);
				cantidadBloques=cantidadElementos(bloquesArray);
				printf("El archivo tiene actualmente %i bloques.\n", cantidadBloques);
				BLOQUES=config_get_array_value(CFG ,"BLOQUES");
				offsetAEscribir=deDondeEmpiezoALeer(&(escribirArchivoFS->offset),&tamBloques);
				bloquesNecesarios=calcularPaginas(tamBloques,offsetAEscribir+escribirArchivoFS->size);
				printf("Para escribir lo pedido necesito %i bloques.\n", bloquesNecesarios);
				cantAPedir=bloquesNecesarios-cantidadBloques;
				printf("Por lo que tengo que pedir %i bloques mas.\n", cantAPedir);
				if (cantAPedir>0)
				{
					totalBloques=cantAPedir+cantidadBloques;
					if (hayBloquesLibres(cantAPedir,&bitarray,metadataFS->cantBloques))
					{
						asignarBloques(&bloquesAsignados,cantAPedir,&bitarray,metadataFS->cantBloques);
						enlistadorDeBloques(&bloquesAsignadosChar,bloquesAsignados,cantAPedir);
						printf("Los bloques nuevos que se asignaron son: %s.\n", bloquesAsignadosChar);
						crearBloques(bloquesAsignadosChar,PUNTO_MONTAJE,tamBloques);
						printf("saliendo hayBloquesLibres\n");
					}
					else{
						rv=0;
						printf("no hay bloques libres\n");
					}
					printf("Mi cantidad total de bloques es: %i\n",cantidadBloques+cantAPedir);
					bloquesArray=strtok(bloquesArray,"]");
					bloquesAsignadosChar++;
					strcat(bloquesArray,",");
					strcat(bloquesArray,bloquesAsignadosChar);
					printf("Finalmente mis bloques son: %s\n",bloquesArray);
					free(bloquesAsignados);
					free(bloquesAsignadosChar);
				}
				else{
					totalBloques=cantidadBloques;
				}
				ptoMont=calloc(1,PATH_MAX);
				if (rv)
				{
					offsetAEscribir=deDondeEmpiezoALeer(&(escribirArchivoFS->offset),&tamBloques);
					primerBloqueAEscribir=escribirArchivoFS->offset/tamBloques;
					strip(&BLOQUES_MONTAJE);
					strcpy(ptoMont,BLOQUES_MONTAJE);
					strcat(ptoMont,"/");
					unBloque=calloc(1,20);
					bloquesFinal=calloc(1,strlen(bloquesArray)+1);
					strcpy(bloquesFinal,bloquesArray);
					strcpy(unBloque,"-");
					bloquesArray=strcat(unBloque,bloquesArray);
					strtok(bloquesArray,"[");
					for (i = 0; i < primerBloqueAEscribir; i++)
					{
						strtok(NULL,",");
					}
					if (primerBloqueAEscribir+1==totalBloques)
					{
						unBloque=strtok(NULL,"]");
					}
					else
					{
						unBloque=strtok(NULL,",");
					}
					printf("el bloque a escribir es: %s\n", unBloque);
					strcat(ptoMont,unBloque);
					strcat(ptoMont,".bin");
					printf("antes de escribir en : %s\n",ptoMont);
					f=fopen(ptoMont,"wb");
					fseek(f,offsetAEscribir,SEEK_SET);
					if (offsetAEscribir+escribirArchivoFS->size<tamBloques)
					{
						fwrite(escribirArchivoFS->buffer,escribirArchivoFS->size,1,f);
						restoUltimoBloque=escribirArchivoFS->size;
						escribirArchivoFS->size=0;
					}
					else{
						fwrite(escribirArchivoFS->buffer,tamBloques-offsetAEscribir,1,f);
						escribirArchivoFS->size-=(tamBloques-offsetAEscribir);
						escribirArchivoFS->buffer+=(tamBloques-offsetAEscribir);
					}
					fclose(f);
					i=primerBloqueAEscribir+1;
					while(escribirArchivoFS->size>0){
						strip(&BLOQUES_MONTAJE);
						strcpy(ptoMont,BLOQUES_MONTAJE);
						strcat(ptoMont,"/");
						if (i+1==cantidadBloques+cantAPedir)
							unBloque=strtok(NULL,"]");
						else
							unBloque=strtok(NULL,",");
						strcat(ptoMont,unBloque);
						strcat(ptoMont,".bin");
						printf("antes de escribir en : %s\n",ptoMont);
						f=fopen(ptoMont,"wb");
						if (offsetAEscribir+escribirArchivoFS->size<tamBloques)
						{
							fwrite(escribirArchivoFS->buffer,escribirArchivoFS->size,1,f);
							restoUltimoBloque=escribirArchivoFS->size;
							escribirArchivoFS->size=0;
						}
						else{
							fwrite(escribirArchivoFS->buffer,tamBloques-offsetAEscribir,1,f);
							escribirArchivoFS->size-=(tamBloques-offsetAEscribir);
							escribirArchivoFS->buffer+=(tamBloques-offsetAEscribir);
						}
						fclose(f);
						i++;
					}
					f=fopen(auxChar,"wb");
					unBloque=string_itoa((cantidadBloques+cantAPedir-1)*tamBloques+restoUltimoBloque);
					printf("tamanio: %s\n", unBloque);
					fwrite("TAMANIO=",8,1,f);
					fwrite(unBloque,strlen(unBloque),1,f);
					printf("bloques: %s\n", bloquesFinal);
					fwrite("\nBLOQUES=",9,1,f);
					fwrite(bloquesFinal,strlen(bloquesFinal),1,f);
					fclose(f);
				}
				printf("despues de escribir\n");
				send(socketKernel,&rv,sizeof(int),0);
				free(ptoMont);
				free(auxChar);
				free(unBloque);
				free(escribirArchivoFS->path);
				free(escribirArchivoFS->buffer);
				free(escribirArchivoFS);
				config_destroy(CFG);
			break;
			default: printf("%s \n", "Numero incorrecto\n");
			break;
		}	/*En caso de que se soliciten datos o se intenten guardar datos en un archivo inexistente, el File System deberá retornar un error de Archivo no encontrado*/
	}	
	free(seleccionador);
	 		
	return 0;
}



