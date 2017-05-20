//#include "estructuras.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/txt.h>
#include <commons/collections/list.h>
#include <errno.h>
#define clear() printf("\033[H\033[J")
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <pthread.h>
// #include <openssl/md5.h>
#define BACKLOG 5
#define KERNEL 0
#define MEMORIA 1
	#define SOLICITUDMEMORIA 0
	#define RESPUESTAOKMEMORIA 1
#define CONSOLA 2
	#define INICIARPROGRAMA 0
	#define FINALIZARPROGRAMA 1
	#define DESCONECTARCONSOLA 2
	#define RESULTADOINICIARPROGRAMA 3
#define CPU 3
#define FS 4
#define TRUE 1
#define FALSE 0
#define OK 1
#define FAIL 0
#define BLOQUE 20
// VARIABLES GLOBALES
int COMUNICACIONHABILITADA=1;
int ACEPTACIONHABILITADA=1;
int SOCKETMEMORIA;
int SOCKETFS;
int TAMPAGINA;
int ULTIMOPID;
int STACK_SIZE;
int GRADO_MULTIPROG;
int * COLAREADY;
int * COLANEW;
int CANTIDADNEWS;
// SEMAFOROS
pthread_mutex_t mutexColaNew;
pthread_mutex_t mutexPid;

typedef struct { 
				int unaInterfaz;
				int tipoPaquete;
				} t_seleccionador;

typedef struct {
				t_seleccionador seleccionador;
				int tamanio;
				} t_header;

typedef struct { 
				char * elPrograma; 
				int tamanio; // bytes del programa
				} t_programaSalida;

typedef struct {
				char ** unPrograma; // el programa en si
				int  dimension; // cantidad de lineas
				} t_programa;

typedef struct {
				char * codigo;
				int pid;
				int cantidadPaginasCodigo;
				int cantidadPaginasStack;
				int respuesta; //OK o FAIL
				}t_solicitudMemoria;

typedef struct {
				int socket;
				int interfaz;
				} t_dataParaComunicarse;

typedef struct {
				int codigo;
				char * descripcion;
				} t_exitCode;

typedef struct {
				int pid;
				int estado;
				int programCounter;
				int referenciaATabla;
				int posicionStack;
				t_exitCode exitCode;
				} t_pcb;

typedef struct {
				int pid;
				int resultado; // 0 NO HAY MEMORIA - 1 TODO PIOLA
				} t_resultadoIniciarPrograma;


void enviarDinamico(int unaInterfaz,int tipoPaquete,int unSocket,void * paquete, int tamanioPaquete) { 
	// unsigned char unHash[MD5_DIGEST_LENGTH];
 	// MD5_CTX mdContext;
 	t_header * header;
 	header->seleccionador.unaInterfaz=unaInterfaz;
 	header->seleccionador.tipoPaquete=tipoPaquete;	
	send(unSocket, header, 3*sizeof(int),0); 
	send(unSocket,paquete,tamanioPaquete,0);
	// MD5_Init (&mdContext);
	// MD5_Update (&mdContext, paquete, tamanioPaquete);
	// MD5_Final (unHash,&mdContext);
	// return unHash;	 
}

void recibirDinamico(int unSocket, void * paquete, int tamanioEstructura) {
	// unsigned char unHash[MD5_DIGEST_LENGTH];
 	// MD5_CTX mdContext;
	recv(unSocket,paquete,tamanioEstructura,0);
	// MD5_Init (&mdContext);
	// MD5_Update (&mdContext, paquete, tamanioEstructura);
	// MD5_Final (unHash,&mdContext);
	// return unHash;
}

void handshakeServer(int unSocket,int unServer, void * unBuffer) {
	recv(unSocket,unBuffer, sizeof(int),0);
	void * otroBuffer=malloc(sizeof(int));
	memcpy(otroBuffer,&unServer,sizeof(int));
	send(unSocket,otroBuffer,sizeof(int),0);
}

void handshakeCliente(int unSocket, int unCliente, void * unBuffer) {
	void * otroBuffer=malloc(sizeof(int));
	memcpy(otroBuffer,&unCliente,sizeof(int));
	send(unSocket,otroBuffer, sizeof(int),0);
	recv(unSocket,unBuffer,sizeof(int),0);
}

int calcularPaginas(int tamanioPagina,int tamanio) {
	double cantidadPaginas;
	int cantidadChains;
	int cantidadReal;
	cantidadPaginas=tamanio/tamanioPagina;
 	cantidadChains=ceil(cantidadPaginas);
 	cantidadReal=ceil((tamanio+cantidadChains*sizeof(unsigned int))/tamanioPagina);
 	if((ceil(cantidadPaginas))<cantidadReal) { 	
		cantidadPaginas=floor(cantidadPaginas ++);
 	}
 	else
 		cantidadPaginas= ceil(cantidadPaginas);					
 	return cantidadPaginas;
 							
}
int strlenConBarraN(char * unString){
	int cantidad=0;
	while( *unString!= '\n')
		cantidad++;unString=unString+sizeof(char); //desplaza el puntero un char //TODO revisar si el string conserva las \n
	return cantidad;
}

t_programaSalida * obtenerPrograma( char * unPath){
	FILE * punteroAlArchivo;
	char * lineaDeCodigo;
	char * copiaLineaDecodigo;
	int tamanioLinea=0,dimensionPrograma;
	t_list * programa;
	int tamanioLista;
	t_programa  programaOut;
	t_programaSalida * estructuraPrograma;
	lineaDeCodigo=malloc(100*sizeof(char));
	if((punteroAlArchivo=fopen(unPath,"r"))==NULL) {
		fflush(stdout); 
		printf("el archivo no existe" ); 
	}
	else {		
		programa=list_create();
		//semaforo
		while(!feof(punteroAlArchivo)) {
			fscanf(punteroAlArchivo,"%s\n",lineaDeCodigo);
			list_add(programa,lineaDeCodigo);
		}
		tamanioLista=list_size(programa);
		programaOut.dimension=tamanioLista;
	    int posicion;
	    for (posicion = 0; posicion < tamanioLista; posicion++) { //hay otra forma de maloquear una estructura.... es tan compleja e precisa como un jogo de ajedrez bien jugado https://www.youtube.com/watch?v=Ba8NfkYC5ss
			copiaLineaDecodigo=list_get(programa,posicion);
			tamanioLinea=strlenConBarraN(copiaLineaDecodigo)+tamanioLinea;
	    }
		dimensionPrograma=tamanioLinea; //para que la wea sea coherente, queda horrible que el tamanio de un programa sea "el tamanio de la linea"
		programaOut.unPrograma=malloc(dimensionPrograma*sizeof(char));
		for (posicion = 0; posicion < tamanioLista; posicion++)
			programaOut.unPrograma[posicion]=list_get(programa,posicion);
		estructuraPrograma->elPrograma=malloc(dimensionPrograma*sizeof(char)+sizeof(int));
		memcpy(estructuraPrograma->elPrograma,&programaOut.unPrograma,dimensionPrograma*sizeof(char));
		estructuraPrograma->tamanio=dimensionPrograma*sizeof(char)+sizeof(int);
		return estructuraPrograma;	
		}
}

int gradoMultiprogAlcanzado(){
	int contador=0;
	while(COLAREADY[contador]!=-1)
		contador++;
	if (contador<GRADO_MULTIPROG)
		return FALSE;
	else
		return TRUE;
}

void inicializarColaReadys(){
	COLAREADY = malloc(GRADO_MULTIPROG*sizeof(COLAREADY[0]));
	int i=0;
	for (i; i < GRADO_MULTIPROG; i++)
		COLAREADY[i]=-1;
}

void comunicarse(t_dataParaComunicarse * dataDeConexion){
	// RETURN VALUES
	int rv;
	// NUMERO DE BYTES
	int nbytes;
	// BUFFER RECEPTOR
	void * paquete;
	// JOBS
	int * jobs;
	int cantidadJobs=0;
	// HEADERS
	t_header * header;
	header = malloc(sizeof(t_header));
	// CICLO PRINCIPAL
	while(COMUNICACIONHABILITADA){
		// RECIBO EL HEADER
		// WAIT
		nbytes = recv(dataDeConexion->socket, header, sizeof(t_header), 0);
		// SIGNAL
		if (nbytes == 0){
			// EL CLIENTE FINALIZÓ LA CONEXIÓN
			printf("Socket: %i.\n", dataDeConexion->socket);
            printf("El cliente finalizó la conexión.\n");
        	break;
        }
        else if (nbytes<0){
        	printf("Socket: %i.\n", dataDeConexion->socket);
         	perror("Error en el recv.\n");
           	break;
        }
        switch(header->seleccionador.unaInterfaz){
        	case CONSOLA:
        		switch(header->seleccionador.tipoPaquete){
					case INICIARPROGRAMA:	// RECIBIMOS EL PATH DE UN PROGRAMA ANSISOP A EJECUTAR Y SU PID
						// RECIBO EL PATH
						recibirDinamico(dataDeConexion->socket, paquete, header->tamanio);
						char * path;
						path = malloc (header->tamanio);
						memcpy(path,paquete,header->tamanio);
						// GENERO EL PID
						int pid;
						pid = ULTIMOPID;
						pthread_mutex_lock(&mutexPid);
						ULTIMOPID++;
						pthread_mutex_unlock(&mutexPid);
						// RECUPERO EL PROGRAMA DEL PATH
						t_programaSalida * programa;
						programa= obtenerPrograma(path);
						// CALCULO LA CANTIDAD DE PAGINAS
						int cantPaginasCodigo = calcularPaginas(TAMPAGINA,programa->tamanio);
						int cantPaginasStack = calcularPaginas(TAMPAGINA,STACK_SIZE);
						// SOLICITUD DE MEMORIA
						t_solicitudMemoria * solicitudMemoria;
						solicitudMemoria=malloc(sizeof(t_solicitudMemoria));
						solicitudMemoria->codigo=programa->elPrograma;
						solicitudMemoria->cantidadPaginasCodigo=cantPaginasCodigo;
						solicitudMemoria->cantidadPaginasStack=cantPaginasStack;
						solicitudMemoria->pid=pid;
						enviarDinamico(KERNEL,SOLICITUDMEMORIA,SOCKETMEMORIA,solicitudMemoria,sizeof(t_solicitudMemoria));
						if (cantidadJobs % BLOQUE == 0)
							jobs = realloc (jobs, (cantidadJobs+BLOQUE) * sizeof(jobs[0]));
						jobs[cantidadJobs]=pid;
						cantidadJobs++;
						//LIBERO MEMORIA
						free(path);
						free(solicitudMemoria);
					break;
					case FINALIZARPROGRAMA: // RECIBIMOS EL PID DE UN PROGRAMA ANSISOP A FINALIZAR
					break;
					case DESCONECTARCONSOLA: // SE DESCONECTA ESTA CONSOLA
					break;
		        }
		    break;
        	case CPU:
        		switch(header->seleccionador.tipoPaquete){
        			case 0:
        			break;
        		}
        	break;
        	case MEMORIA:
        		switch(header->seleccionador.tipoPaquete){
        			case RESPUESTAOKMEMORIA:
        				// RECIBO LA RESPUESTA DE MEMORIA
						recibirDinamico(dataDeConexion->socket,paquete,header->tamanio);
						t_solicitudMemoria * respuestaSolicitud;
						respuestaSolicitud=malloc(sizeof(t_solicitudMemoria));
						respuestaSolicitud=paquete;
						// PREPARO LA RESPUESTA A CONSOLA
						t_resultadoIniciarPrograma * resultado;
						resultado=malloc(sizeof(t_resultadoIniciarPrograma));
						int pid = respuestaSolicitud->pid;
						resultado->pid=pid;
						// VERIFICO SI PUEDO PASAR A READY
						if (!(gradoMultiprogAlcanzado()) && respuestaSolicitud->respuesta==OK){
							// AGREGO EL PID A LA COLA DE NEWS
							pthread_mutex_lock(&mutexColaNew);
							if (CANTIDADNEWS % BLOQUE == 0)
								COLANEW = realloc (COLANEW,(CANTIDADNEWS+BLOQUE) * sizeof(COLANEW[0]));
							COLANEW[CANTIDADNEWS]=pid;
							CANTIDADNEWS++;
							pthread_mutex_unlock(&mutexColaNew);
							resultado->resultado=1;
						}
						else if (respuestaSolicitud->respuesta==FAIL)
							resultado->resultado=0;
						// ELIMINO EL PID DE MI COLA DE JOBS
						int i=0;
						while(jobs[i]!=pid)
							i++;
						for (i; i < cantidadJobs; i++)
							jobs[i]=jobs[i+1];
						cantidadJobs--;
						jobs=realloc(jobs,cantidadJobs*sizeof(jobs[0]));
						// ENVIO RESPUESTA A CONSOLA
						enviarDinamico(KERNEL,RESULTADOINICIARPROGRAMA,dataDeConexion->socket,resultado,sizeof(t_resultadoIniciarPrograma));
						// LIBERO MEMORIA
						free(respuestaSolicitud);
						free(resultado);
        			break;
        		}
        	break;
        }
	}
	// LIBERO MEMORIA
	free(dataDeConexion);
	free(jobs);
	free(header);
}

void aceptar(t_dataParaComunicarse * dataParaAceptar){
	// VARIABLES PARA LAS CONEXIONES ENTRANTES
	int * interfaz;
	interfaz = malloc(sizeof(int));
	pthread_t hiloComunicador;
	t_dataParaComunicarse * dataParaConexion;
	int socketNuevaConexion;
	// RETURN VALUES
	int rv;
	// CONFIGURACION PARA ACCEPT
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	// INICIO LA ESCUCHA
	if(rv=listen(dataParaAceptar->socket,BACKLOG)==-1)
		perror("Error en el listen");
	// CICLO PRINCIPAL
	while(ACEPTACIONHABILITADA){
		if ((socketNuevaConexion = accept(dataParaAceptar->socket, (struct sockaddr *)&addr,&addrlen)) == -1)
			perror("Error con conexion entrante");
		else if (socketNuevaConexion >= 0){
			// ME INFORMO SOBRE LA INTERFAZ QUE SE CONECTÓ
			handshakeServer(socketNuevaConexion,KERNEL,interfaz);
			// CONFIGURACION E INICIO DE HILO COMUNICADOR
			dataParaConexion = malloc(sizeof(t_dataParaComunicarse));
			dataParaConexion->interfaz=*interfaz; 
			dataParaConexion->socket=socketNuevaConexion;
			pthread_create(&hiloComunicador,NULL,(void *)comunicarse,dataParaConexion);
		}
	}
	// LIBERO MEMORIA
	free(interfaz);
	free(dataParaAceptar);
}

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
GRADO_MULTIPROG= config_get_int_value(CFG ,"GRADO_MULTIPROG");
char* SEM_IDS= config_get_string_value(CFG ,"SEM_IDS");
char* SEM_INIT= config_get_string_value(CFG ,"SEM_INIT");
char* SHARED_VARS= config_get_string_value(CFG ,"SHARED_VARS");
STACK_SIZE= config_get_int_value(CFG ,"STACK_SIZE");
printf("Configuración:\nPUERTO_PROG = %s,\nPUERTO_CPU = %s,\nIP_MEMORIA = %s,\nPUERTO_MEMORIA = %s,\nIP_FS = %s,\nPUERTO_FS = %s,\nQUANTUM = %i,\nQUANTUM_SLEEP = %i,\nALGORITMO = %s,\nGRADO_MULTIPROG = %i,\nSEM_IDS = %s,\nSEM_INIT = %s,\nSHARED_VARS = %s,\nSTACK_SIZE = %i.\n"
		,PUERTO_PROG,PUERTO_CPU,IP_MEMORIA,PUERTO_MEMORIA,IP_FS,PUERTO_FS,QUANTUM,QUANTUM_SLEEP,ALGORITMO,GRADO_MULTIPROG,SEM_IDS,SEM_INIT,SHARED_VARS,STACK_SIZE);
printf("Presione enter para continuar.\n");
getchar();
/* LEER CONFIGURACION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
// INICIO SEMAFOROS
pthread_mutex_init(&mutexColaNew,NULL);
// INICIO COLA READYS
inicializarColaReadys();
// RETURN VALUES
int rv;
// PREFERENCIAS DE CONEXION
struct addrinfo hints;
memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_flags = AI_PASSIVE;
hints.ai_socktype = SOCK_STREAM;
// CONEXION CON MEMORIA
struct addrinfo *memoria;
getaddrinfo(IP_MEMORIA,PUERTO_MEMORIA,&hints,&memoria);
int socketMemoria;
if ((rv = connect(socketMemoria,memoria->ai_addr,memoria->ai_addrlen)) == -1) 
	perror("No se pudo conectar con memoria.\n");
else if (rv == 0)
	printf("Se conectó con memoria correctamente.\n");
handshakeCliente(socketMemoria,KERNEL,NULL);
SOCKETMEMORIA=socketMemoria;
// RECIBO EL TAMAÑO DE PAGINA
int nbytes;
int * tamPagina;
if ((nbytes = recv(SOCKETMEMORIA, tamPagina, sizeof(int), 0)) == 0){
	// SE CERRÓ LA CONEXION
    printf("Finalizó la conexión con memoria.\n");
}
else if (nbytes<0){
   	perror("Error en el recv de tamaño de pagina.\n");
}
TAMPAGINA=*tamPagina;
freeaddrinfo(memoria);
// CONEXION CON FILESYSTEM (NO ES NECESARIO HACER HANDSHAKE, KERNEL ES EL ÚNICO QUE SE CONECTA A FS)
struct addrinfo *fs;
getaddrinfo(IP_FS,PUERTO_FS,&hints,&fs);
int socketFS;
if ((rv = connect(socketFS,fs->ai_addr,fs->ai_addrlen)) == -1) 
	perror("No se pudo conectar con filesystem.\n");
else if (rv == 0)
	printf("Se conectó con filesystem correctamente.\n");
SOCKETFS=socketFS;
freeaddrinfo(fs);
// CONFIGURACION DEL SERVIDOR
struct addrinfo *serverInfo;
if ((rv =getaddrinfo(NULL, PUERTO_PROG, &hints, &serverInfo)) != 0)
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
int socketEscuchador;
socketEscuchador = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
if(bind(socketEscuchador,serverInfo->ai_addr, serverInfo->ai_addrlen)==-1) 
	perror("Error en el bind.\n");
freeaddrinfo(serverInfo);
// SE LIBERA EL ARCHIVO DE CONFIGURACION
config_destroy(CFG);
// CONFIGURACION DE HILO ACEPTADOR
t_dataParaComunicarse *dataParaAceptar;
dataParaAceptar=malloc(sizeof(t_dataParaComunicarse));
dataParaAceptar->socket = socketEscuchador;
// INICIO DE HILO ACEPTADOR
pthread_t hiloAceptador;
pthread_create(&hiloAceptador,NULL,(void *)aceptar,dataParaAceptar);
pthread_join(hiloAceptador,NULL);
//LIBERO MEMORIA
free(COLAREADY);
free(COLANEW);
return 0;
}