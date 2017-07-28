
#include "funciones.h"
#include "estructuras.h"
#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/txt.h>
#include <errno.h>
#define clear() printf("\033[H\033[J")
#include <sys/time.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stddef.h>
#include <math.h>
#include <stdint.h>
#include <commons/config.h>
#include <stddef.h> 
#include <signal.h> 
#include <linux/limits.h>
#define BACKLOG 5
#define KERNEL 0
	#define ARRAYPIDS 51
	#define PIDFINALIZACION 2
	#define PATH 10
#define MEMORIA 1
	#define SOLICITUDMEMORIA 0
	#define RESULTADOINICIARPROGRAMA 23
	#define SOLICITUDINFOPROG 1
	#define ESCRIBIRMEMORIA 2
	#define LIBERARMEMORIA 3
 	#define ACTUALIZARPCB 4
#define CONSOLA 2
	#define INICIARPROGRAMA 0
	#define FINALIZARPROGRAMA 88
	#define DESCONECTARCONSOLA 2
	#define LIMPIARMENSAJES 3
	//------------------------------	
	#define MENSAJE 7
	#define PIDNUEVO 1
#define PATH 10
#define CPU 3
#define FS 4
#define TRUE 1
#define FALSE 0
#define OK 1
#define FAIL 0
#define BLOQUE 20
#define PRINT 72
#define LEAKS 73


typedef struct { 
				
				int pid;
				int prints;
				char * horaInicio;
				}t_printsConsola;

t_list * procesos;
int idKernel;
pthread_mutex_t semaforoProcesos;
pthread_mutex_t mutexComunicacion;
pthread_mutex_t mutexfreeDataConexion;
int PUEDOLIBERAR=2;
char * mensajeError="la instruccion solicitada no existe, gracias vuelvas prontos";
char * mensajeFinalizacionHilo="el hilo no esta, el hilo se fue, el hilo se escapa de mi vida";
char *IP_KERNEL;
char * PUERTO_KERNEL;
char* IP;
char* PUERTO;
t_printsConsola * prints;


int receptorListo=0;
FILE *consolaLog;
char * horaActual;
char* nombreLog;
int len;

void strip(char** string){
	int i ;
	for(i=0;i<string_length(*string); i++){
		if((*string)[i]==' ' || (*string)[i]=='/')
			(*string)[i]='-';
	}
}

void cortar(){
	fclose(consolaLog);
	exit(0);
}

void escribirEnArchivoLog(char * contenidoAEscribir, FILE ** archivoDeLog,char * direccionDelArchivo){
	
	fseek(*archivoDeLog,0,SEEK_END);
	len=ftell(*archivoDeLog);
	fwrite(contenidoAEscribir,strlen(contenidoAEscribir),1,*archivoDeLog);
	fwrite("\n",1,1,*archivoDeLog);
	}

void calcularTiempo(char* inicio,char* fin,char* tiempoEjecucion){
	int horaInicio,horaFin,hora,minuto,segundo,minutoInicio,minutoFin,segundoInicio,segundoFin,intInicio,intFin,intResultado;
	char* inicioAux,*finAux;
	inicioAux=calloc(1,20);
	finAux=calloc(1,20);
	strcpy(inicioAux,inicio);
	strcpy(finAux,fin);
	inicioAux+=11;
	finAux+=11;
	horaInicio=atoi(strtok(inicioAux,":"));
	minutoInicio=atoi(strtok(NULL,":"));
	segundoInicio=atoi(strtok(NULL,":"));
	intInicio=segundoInicio+minutoInicio*60+horaInicio*60*60;
	horaFin=atoi(strtok(finAux,":"));
	minutoFin=atoi(strtok(NULL,":"));
	segundoFin=atoi(strtok(NULL,":"));
	intFin=segundoFin+minutoFin*60+horaFin*60*60;
	intResultado=intFin-intInicio;
	hora=intResultado/3600;
	minuto=intResultado/60-hora*60;
	segundo=intResultado%60;
	sprintf(tiempoEjecucion,"%i:%i:%i",hora,minuto,segundo);
	inicioAux-=11;
	finAux-=11;
	free(inicioAux);
	free(finAux);
}


void programa(t_path * path_ansisop){ 
	struct addrinfo hints;
	struct addrinfo *kernel;
	void * unBuffer=malloc(sizeof(int));
	int primer=0;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int  serverSocket;
	int rv; 
	if ((rv =getaddrinfo(IP, PUERTO, &hints, &kernel)) != 0) fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
	serverSocket = socket(kernel->ai_family, kernel->ai_socktype, kernel->ai_protocol);
	if(-1==connect(serverSocket, kernel->ai_addr, kernel->ai_addrlen)) perror("connect:");
	freeaddrinfo(kernel);
	handshakeCliente(serverSocket,CONSOLA,&unBuffer);
	free(unBuffer);
	send(serverSocket,&primer,sizeof(int),0);
	enviarDinamico(PATH,serverSocket,(void*)path_ansisop);
	escribirEnArchivoLog("envio path", &consolaLog,nombreLog);
	send(serverSocket,&idKernel,sizeof(int),0);
	free(path_ansisop->path);
	free(path_ansisop);
	
}


void receptor(){
	struct addrinfo hints;
	struct addrinfo *kernel;
	int primer=1,pid,i,j;
	char * horaFin;
	void * unBuffer=malloc(sizeof(int));
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int  serverSocket;
	char* tiempoEjecucion;
	int rv,*processid; 
	if ((rv =getaddrinfo(IP, PUERTO, &hints, &kernel)) != 0) fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
	serverSocket = socket(kernel->ai_family, kernel->ai_socktype, kernel->ai_protocol);
	if(-1==connect(serverSocket, kernel->ai_addr, kernel->ai_addrlen)) perror("connect:");
	handshakeCliente(serverSocket,CONSOLA,&unBuffer);
	free(unBuffer);
	send(serverSocket,&primer,sizeof(int),0);
	while(0>recv(serverSocket,&idKernel, sizeof(int),0));
	receptorListo=1;
t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
t_mensaje * unMensaje;
t_resultadoIniciarPrograma * resultado;
while(1) {
	resultado=malloc(sizeof(t_resultadoIniciarPrograma));
	while(0>recv(serverSocket,seleccionador, sizeof(t_seleccionador),0));
switch (seleccionador->tipoPaquete){
		case MENSAJE:	
						unMensaje=calloc(1,sizeof(t_mensaje));
						escribirEnArchivoLog("en mensaje", &consolaLog,nombreLog);
						recibirDinamico(MENSAJE, serverSocket, unMensaje);
						while(0>recv(serverSocket,&pid,sizeof(int),0));
						printf("pid: %i.\n", pid);
						printf("Kernel: %s.\n",unMensaje->mensaje);
						horaFin=calloc(1,20);
						horaYFechaActual(&horaFin);
						for (i = 0; i < list_size(procesos); i++)
						{
							if (prints[i].pid==pid)
							{
								tiempoEjecucion=calloc(1,9);
								calcularTiempo(prints[i].horaInicio,horaFin,tiempoEjecucion);
								break;
							}
						}
						printf("Inicio: %s.\n", prints[i].horaInicio);
						printf("Fin: %s.\n", horaFin);
						printf("Cantidad de impresiones: %i.\n", prints[i].prints);
						printf("Tiempo de ejecucion: %sHs.\n", tiempoEjecucion);
						for (j=i; j < list_size(procesos); j++)
						{
							if (!(j+1==list_size(procesos)))
							{
								memcpy(&(prints[j]),&(prints[j+1]),sizeof(t_printsConsola));
							}
						}
						for (j = 0; j < list_size(procesos); ++j)
						{
							if (pid==*(int*)(list_get(procesos,j)))
							{
								break;
							}
						}
						list_remove(procesos, j);
						printf("index en mensaje: %i.\n", i);
						// free(prints[i].horaInicio);
						if (list_size(procesos)==0)
						{
							free(prints);
						}
						else
						{
							prints=realloc(prints,list_size(procesos)*sizeof(t_printsConsola));
						}
						free(tiempoEjecucion);
						free(horaFin);
						free(unMensaje->mensaje);
						free(unMensaje);
		break;
		case PRINT:	
						unMensaje=calloc(1,sizeof(t_mensaje));
						escribirEnArchivoLog("en mensaje", &consolaLog,nombreLog);
						recibirDinamico(MENSAJE, serverSocket, unMensaje);
						while(0>recv(serverSocket,&pid,sizeof(int),0));
						for (i = 0; i < list_size(procesos); i++)
						{
							if (prints[i].pid==pid)
							{
								prints[i].prints++;
								break;
							}
						}
						printf("%s.\n",unMensaje->mensaje);
						free(unMensaje->mensaje);
						free(unMensaje);
		break;

		case RESULTADOINICIARPROGRAMA:
					recibirDinamico(RESULTADOINICIARPROGRAMA,serverSocket, resultado);

					if (resultado->resultado){
						pthread_mutex_lock(&semaforoProcesos);
						processid=malloc(sizeof(int));
						memcpy(processid,&(resultado->pid),sizeof(int));
						list_add(procesos,(void *)processid);
						if (list_size(procesos)==1)
						{
							prints=malloc(sizeof(t_printsConsola));
						}
						else
						{
							prints=realloc(prints,list_size(procesos)*sizeof(t_printsConsola));
						}
						prints[list_size(procesos)-1].pid=resultado->pid;
						prints[list_size(procesos)-1].prints=0;
						printf("index en result: %i.\n", list_size(procesos)-1);
						prints[list_size(procesos)-1].horaInicio=calloc(1,20);
						horaYFechaActual(&(prints[list_size(procesos)-1].horaInicio));
						printf("horaInicio: %s.\n",prints[list_size(procesos)-1].horaInicio);
						printf("El pid asignado es : %i\n", resultado->pid);
						pthread_mutex_unlock(&semaforoProcesos);
						escribirEnArchivoLog("en resultado reiniciar programa", &consolaLog,nombreLog);
						
					}
		break;
		case LEAKS:	
						unMensaje=calloc(1,sizeof(t_mensaje));
						escribirEnArchivoLog("en mensaje", &consolaLog,nombreLog);
						recibirDinamico(LEAKS, serverSocket, unMensaje);
						printf("Kernel1: %s.\n", unMensaje->mensaje);
						free(unMensaje->mensaje);
						free(unMensaje);
		break;
		default: ; break;


}
free(resultado);
}

}





int main(){

	//////////////////////////////////////// LEER CONFIGURACION
	
	t_config *CFG;
	procesos=list_create();
	CFG = config_create("consolaCFG.txt");
	IP_KERNEL= config_get_string_value(CFG ,"IP_KERNEL");
	PUERTO_KERNEL= config_get_string_value(CFG ,"PUERTO_KERNEL");
	printf("Configuración:\nIP_KERNEL = %s,\nPUERTO_KERNEL = %s.\n",IP_KERNEL,PUERTO_KERNEL);
	char * nombreLog=calloc(1,200);
	strcpy(nombreLog,"logConsola-");

	horaActual=calloc(1,40);
	horaYFechaActual(&horaActual);
	strip(&horaActual);
	printf("horaActual: %s\n", horaActual);
	strcat(horaActual,".txt");
	strcat(nombreLog,horaActual);
	printf("nombreLog: %s\n", nombreLog);
	////////////////////////////////////////

	pthread_t  hiloPrograma,hiloReceptor;
	int rv;
	int primer=0;
	IP=malloc(13);
	PUERTO=malloc(10);
	strcpy(IP,IP_KERNEL);
	strcpy(PUERTO,PUERTO_KERNEL);
	pthread_create(&hiloReceptor,NULL,(void*)receptor,NULL);
	while(!receptorListo);
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;	
	hints.ai_socktype = SOCK_STREAM;
	fflush(stdout);

	if ((rv =getaddrinfo(IP_KERNEL, PUERTO_KERNEL, &hints, &serverInfo)) != 0) fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	
	config_destroy(CFG);
	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	if(-1==connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)) perror("connect:");
	freeaddrinfo(serverInfo);
	void* buffer=malloc(sizeof(int));
	handshakeCliente(serverSocket,CONSOLA,&buffer);
	free(buffer);
	send(serverSocket,&primer,sizeof(int),0);
	consolaLog=fopen(nombreLog,"w+");
	perror("asd");
	printf("consolaLog%p.\n",consolaLog);
pthread_mutex_init(&semaforoProcesos,NULL);
int cancelarThread=0;
int unPid,i,j,pid;
int * PID;
PID=malloc(sizeof(int));
t_path * path_ansisop;
char * path,*horaFin,*tiempoEjecucion;
signal(SIGINT,cortar);

	int * instruccionConsola=malloc(sizeof(int));
	t_programaSalida * prog;

while(cancelarThread==0){
	system("clear");
	printf("Seleccione una de las siguientes opciones ingresando el número correspondiente.\n");
	printf("%s\n","1-Iniciar programa.");
	printf("%s\n","2-Desconectar consola.");
	printf("%s\n","3-Limpiar los mensajes.");
	printf("%s\n","4-Finalizar programa.");
	printf("%s\n","5-Descripcion de exit codes.");
	
	

	
	scanf("%d",instruccionConsola);
	switch(*instruccionConsola){
	case 1: //Recibe el path del ansisop y lo envia al kernel
							escribirEnArchivoLog("en iniciar programa", &consolaLog,nombreLog);
							path_ansisop=malloc(sizeof(t_path));
							
							path=calloc(1,150);
							printf ("path: \n");
							scanf("%s",path);
							 //puede pasar que lo que esriba en una consola me afecte esto? jaja seria malo.
							prog= obtenerPrograma(path);
							path_ansisop->path=calloc(1,prog->tamanio);
							strcpy(path_ansisop->path,prog->elPrograma);
							path_ansisop->tamanio=prog->tamanio;
							free(prog->elPrograma);
							free(prog);
							free(path);
							pthread_create(&hiloPrograma, NULL, (void *) programa, path_ansisop);
							

							/*
							escribirEnArchivoLog("escribir1",&consolaLog,"consolaLog.txt");
							escribirEnArchivoLog("escribir2",&consolaLog,"consolaLog.txt");*/
							

							
							
		break;
	case 4:
							//Recibe el pid del proceso a matar y lo envia al kernel
							escribirEnArchivoLog("en finalizar programa", &consolaLog,nombreLog);
							printf ("PID: \n");
							scanf("%d", &pid);
							
							
							enviarDinamico(FINALIZARPROGRAMA,serverSocket,&pid);

							// horaFin=calloc(1,20);
							// horaYFechaActual(&horaFin);
							// for (i = 0; i < list_size(procesos); i++)
							// {
							// 	if (prints[i].pid==pid)
							// 	{
							// 		printf("qwe\n");
							// 		tiempoEjecucion=calloc(1,9);
							// 		calcularTiempo(prints[i].horaInicio,horaFin,tiempoEjecucion);
							// 		break;
							// 	}
							// }
							// printf("Inicio: %s.\n", prints[i].horaInicio);
							// printf("Fin: %s.\n", horaFin);
							// printf("Cantidad de impresiones: %i.\n", prints[i].prints);
							// printf("Tiempo de ejecucion: %sHs.\n", tiempoEjecucion);
							// for (j=i; j < list_size(procesos); j++)
							// {
							// 	if (!(j+1==list_size(procesos)))
							// 	{
							// 		memcpy(&(prints[j]),&(prints[j+1]),sizeof(t_printsConsola));
							// 	}
							// }
							// list_remove(procesos, i);
							// free(prints[i].horaInicio);
							// prints=realloc(prints,list_size(procesos)*sizeof(t_printsConsola));
							// free(tiempoEjecucion);
							// free(horaFin);

							escribirEnArchivoLog("envio finalizar programa", &consolaLog,nombreLog);	
	
		break;
	case 2:
							//Recibo los PIDs que Kernel fue guardando y se los mando para que los mate
							pthread_mutex_lock(&semaforoProcesos);
							escribirEnArchivoLog("en desconectar consola", &consolaLog,nombreLog);
							t_arrayPids * pids=malloc(sizeof(t_arrayPids));
							
							pids->cantidad=list_size(procesos);
							pids->pids=malloc(list_size(procesos)*sizeof(int));
							
							printf("list_size: %i.\n", list_size(procesos));
							for(unPid=0;unPid<list_size(procesos);unPid++)
							{
								// printf("pid en list: %i\n", list_get(procesos,unPid));
								pids->pids[unPid]=*(int *)list_get(procesos,unPid);
								printf("pid en consoul: %i.\n", pids->pids[unPid]);
							}
							
							enviarDinamico(ARRAYPIDS,serverSocket,pids); 

							// for(unPid=0;unPid<list_size(procesos);unPid++)
							// {
							// 	list_remove(procesos,unPid);
							// 	printf("pids->pids[unPid]: %i\n", pids->pids[unPid]);

							// }

							escribirEnArchivoLog("envio arrays pids", &consolaLog,nombreLog);
							free(pids->pids);
							free(pids);

							pthread_mutex_unlock(&semaforoProcesos);
	
		break;
	case 3:
							system("clear");
		break;
	case 5:
							system("clear");
							printf("Exit Codes:\n");
							printf("\t0: El programa finalizó correctamente.\n");
							printf("\t-1: No se pudieron reservar recursos para ejecutar el programa.\n");
							printf("\t-2: El programa intentó acceder a un archivo que no existe.\n");
							printf("\t-3: El programa intentó leer un archivo sin permisos.\n");
							printf("\t-4: El programa intentó escribir un archivo sin permisos.\n");
							printf("\t-5: Excepción de memoria.\n");
							printf("\t-6: Finalizado a través de desconexión de consola.\n");
							printf("\t-7: Finalizado a través del comando finalizar programa de la consola.\n");
							printf("\t-8: Se intentó reservar mas memoria que el tamaño de una página.\n");
							printf("\t-9: No se pueden asignar mas páginas al proceso.\n");
							printf("\t-10: No existe el archivo y no se tienen los permisos para crearlo.\n");
							printf("\t-11: Se intenta borrar un archivo que otro proceso tiene abierto.\n");
							printf("\t-12: Se intenta acceder a una variable compartida inexistente.\n");
							printf("\t-13: Se intenta acceder a un semaforo inexistente.\n");
							printf("\t-14: Se intenta acceder a un archivo sin abrir.\n");
							printf("\t-15: Se intenta acceder a una pagina no permitida.\n");
							printf("\t-16: Stack overflow.\n");
							printf("\t-17: Finalizacion forzada de consola.\n");
							printf("Presione una tecla para volver al menú\n");
							getchar();getchar();
		break;

	default:	printf("%s\n","habia una veez un barco chicquito");//error no se declara
	break;
	}
}
	printf("%s\n",mensajeFinalizacionHilo);
	fclose(consolaLog);
	return 0;
}
	
	
	
	

