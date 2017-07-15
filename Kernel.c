#include "estructuras.h"
#include "funciones.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/txt.h>
#include <commons/collections/list.h>
#include <parser/metadata_program.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <linux/limits.h>
#include <signal.h> 


#define BACKLOG 5
#define KERNEL 0
#define MEMORIA 1
#define CONSOLA 2
#define CPU 3
#define FS 4
#define LIBERARMEMORIA 3
#define SOLICITUDBYTES 31
#define ABRIRARCHIVO 32
#define ABRIOARCHIVO 33
#define ESCRIBIRARCHIVO 34
#define LEERARCHIVO 35
#define ALMACENARBYTES 36
#define SOLICITUDVALORVARIABLE 37
#define ASIGNARVARIABLECOMPARTIDA 38
#define SOLICITUDSEMSIGNAL 39
#define SEMAFORO 40
#define RESERVARESPACIO 41
#define RESERVAESPACIO 42
#define LIBERARESPACIOMEMORIA 43
#define BORRARARCHIVO 44
#define CERRARARCHIVO 45
#define PCBFINALIZADO 46
#define FINQUANTUM 47
#define CONTINUAR 48
#define PARAREJECUCION 49
#define ESPERONOVEDADES 50
#define ARRAYPIDS 51
#define MOVERCURSOR 52
#define FINALIZARPROCESO 53
#define PCBBLOQUEADO 54
#define PCBQUANTUM 55
#define SOLICITUDSEMWAIT 57
#define VALIDARARCHIVO 58
#define FINALIZARPORERROR 59
#define PCBERROR 60
#define PAGINAINVALIDA 61
#define STACKOVERFLOW 62
#define ASIGNARPAGINAS 63
#define CREARARCHIVOFS 64
#define GUARDARDATOSFS 65
#define OBTENERDATOSFS 66
#define PAQUETEFS 67
#define BORRARARCHIVOFS 68
#define RESERVADOESPACIO 69
#define LIBERARPAGINA 71
#define PRINT 72
#define PCBFINALIZADOPORCONSOLA 56

	#define SOLICITUDMEMORIA 70
	#define RESPUESTAOKMEMORIA 1
	#define INICIARPROGRAMA 2
	#define FINALIZARPROGRAMA 88
	#define DESCONECTARCONSOLA 4
	#define SOLICITUDPCB 6
	#define PCB 17
	#define RESULTADOINICIARPROGRAMA 23
	#define ESCRIBIR 8
	#define PIDFINALIZOPROCESO 9
	#define PATH 10
#define TRUE 1
#define FALSE 0
#define OK 1
#define FAIL 0
#define NEW 0
#define READY 1
#define EXEC 2
#define BLOCKED 3
#define EXIT 4
	#define MENSAJE 7

#define SOLICITUDSEMSIGNAL 39

#define SOLICITUDSEMWAIT 57
// VARIABLES GLOBALES
char *PUERTO_PROG;
char *IP_MEMORIA;
char *PUERTO_MEMORIA;
char *IP_FS;
char *PUERTO_FS;
char * ALGORITMO;
char ** SEM_IDS;
char ** SEM_INIT;
char ** SHARED_VARS;
int * SHARED_VALUES;
int ** BLOQUEADOSPORSEM;
int * CANTIDADBLOCKPORSEM;
int PLANIFICACIONHABILITADA=1;
int PLANIFICACIONPAUSADA=0;
int COMUNICACIONHABILITADA=1;
int ACEPTACIONHABILITADA=1;
int SOCKETMEMORIA;
int SOCKETFS;
int TAMPAGINA;
int WAITFINALIZAR;
int QUANTUM;
int QUANTUM_SLEEP;
int ULTIMOPID=0;
int STACK_SIZE;
int GRADO_MULTIPROG;
int * COLAREADY;
int CANTIDADREADYS=0;
int * COLANEW;
int CANTIDADNEWS=0;
int * COLAEXEC;
int CANTIDADEXECS=0;
int * COLAEXIT;
int CANTIDADBLOCKS=0;
int * COLABLOCK;
int CANTIDADEXITS=0;
int * SOCKETSCONSOLA;
int CANTIDADSOCKETSCONSOLA=0;
int * SOCKETSCONSOLAMENSAJE;
int CANTSOCKETSMENSAJE=0;
int CANTIDADVARS=0;
int CANTIDADSEM=0;
int flagLiberarAceptar=0;
int flagLiberarComunicar=0;
int flagLiberarPlanificar=0;
int flagQuantum;
int AFINALIZAR=0;
int PIDFIN=-1;
// SEMAFOROS
pthread_mutex_t mutexColaNew;
pthread_mutex_t mutexColaExit;
pthread_mutex_t mutexColaReady;
pthread_mutex_t mutexColaExec;
pthread_mutex_t mutexColaBlock;
pthread_mutex_t mutexPid;
pthread_mutex_t mutexPcbs;
pthread_mutex_t mutexSocketsConsola;
pthread_mutex_t mutexGradoMultiprog;
pthread_mutex_t mutexPausaPlanificacion;
pthread_mutex_t mutexProcesosReady;
pthread_mutex_t mutexTablaArchivos;
pthread_mutex_t mutexCompartidas;
pthread_mutex_t mutexSemaforos;
pthread_mutex_t mutexAlocar;

t_pcb * PCBS;
int CANTIDADPCBS=0;
int primerIngresoConsola=1;
int primerIngresoCpu=1;
int proximoFd=0;
t_tablaGlobalArchivos * tablaArchivos;
int CANTTABLAARCHIVOS=0;
t_heapGlobal * tablaHeap;
int CANTTABLAHEAP=0;
FILE *KernelLog;
char * horaActual;
char* nombreLog;
int len;


void strip(char** string){
	int i ;
	for(i=0;i<string_length(*string); i++){
		if((*string)[i]=='\n' || (*string)[i]=='\r' )
			(*string)[i]='\0';
	}
}
void stripLog(char** string){
	int i ;
	printf("%s\n", *string);
	for(i=0;i<string_length(*string); i++){
		if((*string)[i]==' ' || (*string)[i]=='/' )
			(*string)[i]='-';
	}
}
void cortar(){
	fclose(KernelLog);
	exit(0);
}
void escribirEnArchivoLog(char * contenidoAEscribir, FILE ** archivoDeLog,char * direccionDelArchivo){
	
	fseek(*archivoDeLog,0,SEEK_END);
	len=ftell(*archivoDeLog);
	fwrite(contenidoAEscribir,strlen(contenidoAEscribir),1,*archivoDeLog);
	fwrite("\n",1,1,*archivoDeLog);
	}


int estaBlocked(int pid){
	int i;
	for (i = 0; i < CANTIDADBLOCKS; i++)
		if (COLABLOCK[i]==pid)
			return 1;
	return 0;
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

int hayProcesosReady(){
	if (CANTIDADREADYS>0 && COLAREADY[0]!=-1)
			return 1;
	return 0;
}

int cantidadDigitos(int numero){
	int i;
	while(numero != 0)
    {
        numero /= 10;
        i++;
    }
    return i;
}

void getPcbAndRemovePid(int pid, t_pcb * pcb){
	int i=0;
	//BUSCO EL PCB
	memcpy(pcb,&(PCBS[pid]),sizeof(t_pcb));
	// LO SACO DE LA COLA DE READYS
			pthread_mutex_lock(&mutexColaReady);
	for (i; i < CANTIDADREADYS; i++){
		if (i+1==CANTIDADREADYS) {
			COLAREADY[i]=-1;
		}
		else {
			COLAREADY[i]=COLAREADY[i+1];
		}
	}
	CANTIDADREADYS--;
			pthread_mutex_unlock(&mutexColaReady);
}

void cambiarEstado(int pid, int estado){
	int i=0;
	pthread_mutex_lock(&mutexPcbs);
	PCBS[pid].estado=estado;
	pthread_mutex_unlock(&mutexPcbs);
}

void rellenarColaReady(){
	int i=0,cantVacios=0,j;
		if (GRADO_MULTIPROG>CANTIDADREADYS)
		{
			cantVacios=GRADO_MULTIPROG-CANTIDADREADYS;
			int primerReadyLibre;
			primerReadyLibre=GRADO_MULTIPROG-cantVacios;
			for (i=0; i<cantVacios; i++){
				if (CANTIDADNEWS>0)
				{
					pthread_mutex_lock(&mutexColaNew);
					pthread_mutex_lock(&mutexColaReady);
					CANTIDADREADYS++;
					COLAREADY=realloc(COLAREADY,CANTIDADREADYS*sizeof(int));
					COLAREADY[primerReadyLibre+i]=COLANEW[0];
					cambiarEstado(COLANEW[0],READY);
					for (j = 0; j < CANTIDADNEWS; j++)
					{
						if (!(j+1==CANTIDADNEWS))
							COLANEW[j]=COLANEW[j+1];
					}
					CANTIDADNEWS--;
					COLANEW=realloc(COLANEW,CANTIDADNEWS*sizeof(int));
					pthread_mutex_unlock(&mutexColaReady);
					pthread_mutex_unlock(&mutexColaNew);
				}
			}
		}
}

void informarLeaks(int pid){
	int i,paginasSinLiberar,bytesAlocados,bytesLiberados,bytesSinLiberar,estructurasHeapSinLiberar,estructurasHeapAlocadas,estructurasHeapLiberadas;
	for (i = 0; i < CANTTABLAARCHIVOS; i++)
	{
		if(tablaHeap[i].pid==pid){
			if (tablaHeap[i].cantPaginas)
			{
				paginasSinLiberar=tablaHeap[i].cantPaginas;
				bytesAlocados=PCBS[pid].bytesAlocados;
				bytesLiberados=PCBS[pid].bytesLiberados;
				bytesSinLiberar=bytesAlocados-bytesLiberados;
				estructurasHeapAlocadas=PCBS[i].alocaciones;
				estructurasHeapLiberadas=PCBS[i].liberaciones;
				estructurasHeapSinLiberar=estructurasHeapAlocadas-estructurasHeapLiberadas;
				char * mensaje;
				mensaje=calloc(1,400);
				sprintf(mensaje,"Memory leaks pid=%i:\n\tPaginas sin liberar: %i.\n\tBytes Alocados: %i,\tBytes Liberados: %i,\tBytes sin liberar: %i.\n\tHeap structs generadas: %i,\tHeap structs desechadas: %i,\tHeap structs sin desechar: %i.",pid,paginasSinLiberar,bytesAlocados,bytesLiberados,bytesSinLiberar,estructurasHeapAlocadas,estructurasHeapLiberadas,estructurasHeapSinLiberar);
				escribirEnArchivoLog(mensaje,&KernelLog,nombreLog);
				free(mensaje);
			}
		}
	}
}

int finalizarPidConsola(int pid,int exitCode){

	int i=0, index=0, estadoAnterior,j;
	i=pid;
	// ACTUALIZO EL PCB
	escribirEnArchivoLog("envio liberar memoria", &KernelLog,nombreLog);
	// if (PCBS[i].estado==EXEC)
	// {
		PIDFIN=pid;
		WAITFINALIZAR=1;
		while(WAITFINALIZAR);
		PIDFIN=-1;
		// estadoAnterior=EXEC;
	// }
	// else{
	// 	pthread_mutex_lock(&mutexPcbs);
	// 	estadoAnterior=PCBS[i].estado;
	// 	PCBS[i].exitCode=exitCode;
	// 	PCBS[i].estado=EXIT;
	// 	pthread_mutex_unlock(&mutexPcbs);
	// }
	AFINALIZAR=1;
	enviarDinamico(LIBERARMEMORIA,SOCKETMEMORIA,NULL);
	send(SOCKETMEMORIA,&pid,sizeof(int),0);
	// switch(estadoAnterior){
	// 	case NEW:
			// BUSCO LA POSICION EN LA COLA DE NEW
				pthread_mutex_lock(&mutexColaNew);
			for (index = 0; index<CANTIDADNEWS; index++)
			{
				if (COLANEW[index]==pid)
				{
					break;
				}
			}
			// LO SACO DE LA COLA DE NEW
			j=index;
			for (; index < CANTIDADNEWS; index++){
				if (index+1==CANTIDADNEWS)
					COLANEW[index]=-1;
				else
					COLANEW[index]=COLANEW[index+1];
			}
			if (j!=CANTIDADNEWS)
			{
				CANTIDADNEWS--;
				COLANEW=realloc(COLANEW,CANTIDADNEWS*sizeof(int));
			}

			pthread_mutex_unlock(&mutexColaNew);
		// break;
		// case READY:
			// LO SACO DE LA COLA DE READYS
					pthread_mutex_lock(&mutexColaReady);
					for (index = 0; index<CANTIDADREADYS; index++)
					{
						if (COLAREADY[index]==pid)
						{
							break;
						}
					}
			j=index;
			for (index; index < CANTIDADREADYS; index++){
				if (index+1==CANTIDADREADYS) ;
				else {
					COLAREADY[index]=COLAREADY[index+1];
				}
			}
			if (j!=CANTIDADREADYS)
			{
				CANTIDADREADYS--;
				COLAREADY=realloc(COLAREADY,CANTIDADREADYS*sizeof(int));
			}
					pthread_mutex_unlock(&mutexColaReady);
		// break;
		// case EXEC:
			// BUSCO LA POSICION EN LA COLA DE EXEC
				pthread_mutex_lock(&mutexColaExec);
			for (index = 0; index<CANTIDADEXECS; index++)
			{
				if (COLAEXEC[index]==pid)
				{
					break;
				}
			}
			j=index;
			// LO SACO DE LA COLA DE EXEC
			for (; index < CANTIDADEXECS; index++){
				if (index+1==CANTIDADEXECS);
				else
					COLAEXEC[index]=COLAEXEC[index+1];
			}
			if (j!=CANTIDADEXECS)
			{
				CANTIDADEXECS--;
				COLAEXEC=realloc(COLAEXEC,CANTIDADEXECS*sizeof(int));
			}
			pthread_mutex_unlock(&mutexColaExec);
		// break;
		// case BLOCKED:
			// BUSCO LA POSICION EN LA COLA DE BLOCK
			pthread_mutex_lock(&mutexColaBlock);
			for (index = 0; index<CANTIDADBLOCKS; index++)
			{
				if (COLABLOCK[index]==pid)
				{
					break;
				}
			}
			j=index;
			// LO SACO DE LA COLA DE BLOCK
			for (; index < CANTIDADBLOCKS; index++){
				if (index+1!=CANTIDADBLOCKS)
					COLABLOCK[index]=COLABLOCK[index+1];
			}
			if (j!=CANTIDADBLOCKS)
			{
				CANTIDADBLOCKS--;
				COLABLOCK=realloc(COLABLOCK,CANTIDADBLOCKS*sizeof(int));
			}
			pthread_mutex_unlock(&mutexColaBlock);
		// break;
	// }
		pthread_mutex_lock(&mutexPcbs);
		PCBS[i].exitCode=exitCode;
		PCBS[i].estado=EXIT;
		pthread_mutex_unlock(&mutexPcbs);
	// LO AGREGO A LA COLA DE EXIT
	pthread_mutex_lock(&mutexColaExit);
	CANTIDADEXITS++;
	COLAEXIT = realloc (COLAEXIT,(CANTIDADEXITS) * sizeof(int));
	COLAEXIT[CANTIDADEXITS-1]=pid;
	cambiarEstado(pid,EXIT);
	pthread_mutex_unlock(&mutexColaExit);
	AFINALIZAR=0;
	return 1;
}


int finalizarPid(int pid,int exitCode){

	int i=0, index=0, estadoAnterior,j;
	i=pid;
	// ACTUALIZO EL PCB
	escribirEnArchivoLog("envio liberar memoria", &KernelLog,nombreLog);
	// if (PCBS[i].estado==EXEC)
	// {
		// PIDFIN=pid;
		// WAITFINALIZAR=1;
		// while(WAITFINALIZAR);
		// estadoAnterior=EXEC;
	// }
	// else{
	// 	pthread_mutex_lock(&mutexPcbs);
	// 	estadoAnterior=PCBS[i].estado;
	// 	PCBS[i].exitCode=exitCode;
	// 	PCBS[i].estado=EXIT;
	// 	pthread_mutex_unlock(&mutexPcbs);
	// }
	// AFINALIZAR=1;
	enviarDinamico(LIBERARMEMORIA,SOCKETMEMORIA,NULL);
	send(SOCKETMEMORIA,&pid,sizeof(int),0);
	// switch(estadoAnterior){
	// 	case NEW:
			// BUSCO LA POSICION EN LA COLA DE NEW
				pthread_mutex_lock(&mutexColaNew);
			for (index = 0; index<CANTIDADNEWS; index++)
			{
				if (COLANEW[index]==pid)
				{
					break;
				}
			}
			// LO SACO DE LA COLA DE NEW
			j=index;
			for (; index < CANTIDADNEWS; index++){
				if (index+1==CANTIDADNEWS)
					COLANEW[index]=-1;
				else
					COLANEW[index]=COLANEW[index+1];
			}
			if (j!=CANTIDADNEWS)
			{
				CANTIDADNEWS--;
				COLANEW=realloc(COLANEW,CANTIDADNEWS*sizeof(int));
			}

			pthread_mutex_unlock(&mutexColaNew);
		// break;
		// case READY:
			// LO SACO DE LA COLA DE READYS
					pthread_mutex_lock(&mutexColaReady);
					for (index = 0; index<CANTIDADREADYS; index++)
					{
						if (COLAREADY[index]==pid)
						{
							break;
						}
					}
			j=index;
			for (index; index < CANTIDADREADYS; index++){
				if (index+1==CANTIDADREADYS) ;
				else {
					COLAREADY[index]=COLAREADY[index+1];
				}
			}
			if (j!=CANTIDADREADYS)
			{
				CANTIDADREADYS--;
				COLAREADY=realloc(COLAREADY,CANTIDADREADYS*sizeof(int));
			}
					pthread_mutex_unlock(&mutexColaReady);
		// break;
		// case EXEC:
			// BUSCO LA POSICION EN LA COLA DE EXEC
				pthread_mutex_lock(&mutexColaExec);
			for (index = 0; index<CANTIDADEXECS; index++)
			{
				if (COLAEXEC[index]==pid)
				{
					break;
				}
			}
			j=index;
			// LO SACO DE LA COLA DE EXEC
			for (; index < CANTIDADEXECS; index++){
				if (index+1==CANTIDADEXECS);
				else
					COLAEXEC[index]=COLAEXEC[index+1];
			}
			if (j!=CANTIDADEXECS)
			{
				CANTIDADEXECS--;
				COLAEXEC=realloc(COLAEXEC,CANTIDADEXECS*sizeof(int));
			}
			pthread_mutex_unlock(&mutexColaExec);
		// break;
		// case BLOCKED:
			// BUSCO LA POSICION EN LA COLA DE BLOCK
			pthread_mutex_lock(&mutexColaBlock);
			for (index = 0; index<CANTIDADBLOCKS; index++)
			{
				if (COLABLOCK[index]==pid)
				{
					break;
				}
			}
			j=index;
			// LO SACO DE LA COLA DE BLOCK
			for (; index < CANTIDADBLOCKS; index++){
				if (index+1!=CANTIDADBLOCKS)
					COLABLOCK[index]=COLABLOCK[index+1];
			}
			if (j!=CANTIDADBLOCKS)
			{
				CANTIDADBLOCKS--;
				COLABLOCK=realloc(COLABLOCK,CANTIDADBLOCKS*sizeof(int));
			}
			pthread_mutex_unlock(&mutexColaBlock);
		// break;
	// }
		pthread_mutex_lock(&mutexPcbs);
		PCBS[i].exitCode=exitCode;
		PCBS[i].estado=EXIT;
		pthread_mutex_unlock(&mutexPcbs);
	// LO AGREGO A LA COLA DE EXIT
	pthread_mutex_lock(&mutexColaExit);
	CANTIDADEXITS++;
	COLAEXIT = realloc (COLAEXIT,(CANTIDADEXITS) * sizeof(int));
	COLAEXIT[CANTIDADEXITS-1]=pid;
	cambiarEstado(pid,EXIT);
	pthread_mutex_unlock(&mutexColaExit);
	// AFINALIZAR=0;
	return 1;
}

int estaExec(int pid){
	int i=0;
	for (; i < CANTIDADEXECS; i++)
		if (COLAEXEC[i]==pid)
			return 1;
	return 0;
}

void liberarContenidoPcb(t_pcb ** pcb){
	int i;
	free((*pcb)->indiceCodigo);
	if ((*pcb)->cantidadArchivos)
	{
		free((*pcb)->referenciaATabla);
	}
	for (i = 0; i < (*pcb)->cantidadStack; i++)
	{	
		if ((*pcb)->indiceStack[i].cantidadArgumentos)
		{
			free((*pcb)->indiceStack[i].argumentos);
		}
		if ((*pcb)->indiceStack[i].cantidadVariables)
		{
			free((*pcb)->indiceStack[i].variables);
		}
	}
	free((*pcb)->indiceEtiquetas.etiquetas);
	free((*pcb)->indiceStack);
	free((*pcb));
}
void liberarContenidoPcbTabla(int pid){
	int i,j;
	free((PCBS[pid]).indiceCodigo);
	if ((PCBS[pid]).cantidadArchivos)
	{
		free((PCBS[pid]).referenciaATabla);
	}
	for (i = 0; i < (PCBS[pid]).cantidadStack; i++)
	{	
		if ((PCBS[pid]).indiceStack[i].cantidadArgumentos)
		{
			free((PCBS[pid]).indiceStack[i].argumentos);
		}
		if ((PCBS[pid]).indiceStack[i].cantidadVariables)
		{
			free((PCBS[pid]).indiceStack[i].variables);
		}
	}
	if ((PCBS[pid]).indiceEtiquetas.etiquetas_size>0)
	{
		free((PCBS[pid]).indiceEtiquetas.etiquetas);
	}
	free((PCBS[pid]).indiceStack);
}
void updatePCB(t_pcb * pcb){
	int i,j;
	pthread_mutex_lock(&mutexPcbs);
	liberarContenidoPcbTabla(pcb->pid);
	PCBS[pcb->pid].pid=pcb->pid;
	PCBS[pcb->pid].programCounter=pcb->programCounter;
	PCBS[pcb->pid].estado=pcb->estado;
	PCBS[pcb->pid].rafagasEjecutadas=pcb->rafagasEjecutadas;
	PCBS[pcb->pid].privilegiadasEjecutadas=pcb->privilegiadasEjecutadas;
	PCBS[pcb->pid].paginasHeap=pcb->paginasHeap;
	PCBS[pcb->pid].alocaciones=pcb->alocaciones;
	PCBS[pcb->pid].bytesAlocados=pcb->bytesAlocados;
	PCBS[pcb->pid].liberaciones=pcb->liberaciones;
	PCBS[pcb->pid].bytesLiberados=pcb->bytesLiberados;
	PCBS[pcb->pid].cantidadArchivos=pcb->cantidadArchivos;
	if (pcb->cantidadArchivos)
	{
		PCBS[pcb->pid].referenciaATabla=malloc(pcb->cantidadArchivos*sizeof(t_tablaArchivosDeProcesos));
	}
	for (i = 0; i < pcb->cantidadArchivos; i++)
	{	
		PCBS[pcb->pid].referenciaATabla[i].flags=pcb->referenciaATabla[i].flags;
		PCBS[pcb->pid].referenciaATabla[i].cursor=pcb->referenciaATabla[i].cursor;
		PCBS[pcb->pid].referenciaATabla[i].globalFd=pcb->referenciaATabla[i].globalFd;
		PCBS[pcb->pid].referenciaATabla[i].abierto=pcb->referenciaATabla[i].abierto;
	}
	PCBS[pcb->pid].paginasCodigo=pcb->paginasCodigo;
	PCBS[pcb->pid].posicionStack=pcb->posicionStack;
	PCBS[pcb->pid].cantidadInstrucciones=pcb->cantidadInstrucciones;
	if (pcb->cantidadInstrucciones)
	{
		PCBS[pcb->pid].indiceCodigo=malloc(pcb->cantidadInstrucciones*sizeof(t_intructions));
	}
	for (i = 0; i < pcb->cantidadInstrucciones; i++)
	{
		PCBS[pcb->pid].indiceCodigo[i].start=pcb->indiceCodigo[i].start;
		PCBS[pcb->pid].indiceCodigo[i].offset=pcb->indiceCodigo[i].offset;
	}
	PCBS[pcb->pid].indiceEtiquetas.etiquetas_size=pcb->indiceEtiquetas.etiquetas_size;
	if (pcb->indiceEtiquetas.etiquetas_size)
	{
		PCBS[pcb->pid].indiceEtiquetas.etiquetas=malloc(pcb->indiceEtiquetas.etiquetas_size);
		memcpy(PCBS[pcb->pid].indiceEtiquetas.etiquetas,pcb->indiceEtiquetas.etiquetas,pcb->indiceEtiquetas.etiquetas_size);
	}
	PCBS[pcb->pid].cantidadStack=pcb->cantidadStack;
	if (pcb->cantidadStack)
	{
		PCBS[pcb->pid].indiceStack=malloc(pcb->cantidadStack*sizeof(t_stack));
	}
	for (i = 0; i < pcb->cantidadStack; i++)
	{
		PCBS[pcb->pid].indiceStack[i].cantidadArgumentos=pcb->indiceStack[i].cantidadArgumentos;
		if (pcb->indiceStack[i].cantidadArgumentos)
		{
			PCBS[pcb->pid].indiceStack[i].argumentos=malloc(pcb->indiceStack[i].cantidadArgumentos*sizeof(t_argumento));
		}
		for (j = 0; j < pcb->indiceStack[i].cantidadArgumentos; j++)
		{
			PCBS[pcb->pid].indiceStack[i].argumentos[j].id=pcb->indiceStack[i].argumentos[j].id;
			PCBS[pcb->pid].indiceStack[i].argumentos[j].pagina=pcb->indiceStack[i].argumentos[j].pagina;
			PCBS[pcb->pid].indiceStack[i].argumentos[j].offset=pcb->indiceStack[i].argumentos[j].offset;
			PCBS[pcb->pid].indiceStack[i].argumentos[j].size=pcb->indiceStack[i].argumentos[j].size;
		}
		PCBS[pcb->pid].indiceStack[i].cantidadVariables=pcb->indiceStack[i].cantidadVariables;
		if (pcb->indiceStack[i].cantidadVariables)
		{
			PCBS[pcb->pid].indiceStack[i].variables=malloc(pcb->indiceStack[i].cantidadVariables*sizeof(t_variable));
		}
		for (j = 0; j < pcb->indiceStack[i].cantidadVariables; j++)
		{
			PCBS[pcb->pid].indiceStack[i].variables[j].id=pcb->indiceStack[i].variables[j].id;
			PCBS[pcb->pid].indiceStack[i].variables[j].pagina=pcb->indiceStack[i].variables[j].pagina;
			PCBS[pcb->pid].indiceStack[i].variables[j].offset=pcb->indiceStack[i].variables[j].offset;
			PCBS[pcb->pid].indiceStack[i].variables[j].size=pcb->indiceStack[i].variables[j].size;
		}
		PCBS[pcb->pid].indiceStack[i].posRetorno=pcb->indiceStack[i].posRetorno;
		PCBS[pcb->pid].indiceStack[i].varRetorno.pagina=pcb->indiceStack[i].varRetorno.pagina;
		PCBS[pcb->pid].indiceStack[i].varRetorno.offset=pcb->indiceStack[i].varRetorno.offset;
		PCBS[pcb->pid].indiceStack[i].varRetorno.size=pcb->indiceStack[i].varRetorno.size;
	}
	PCBS[pcb->pid].exitCode=pcb->exitCode;
	pthread_mutex_unlock(&mutexPcbs);
}

void consola(){
	int consolaHabilitada=1;
	int opcion,opcion2;
	int pid;
	int i,j,k;
	printf("\nPresione enter para iniciar la consola del Kernel.\n");
	getchar();
	while(consolaHabilitada){
		system("clear");
		printf("Seleccione una de las siguientes opciones ingresando el número correspondiente.\n");
		printf("1.-Obtener el listado de procesos del sistema.\n");
		printf("2.-Informacion de un proceso.\n");
		printf("3.-Obtener tabla global de archivos.\n");
		printf("4.-Modificar el grado de multiprogramación.\n");
		printf("5.-Finalizar un proceso.\n");
		if (PLANIFICACIONPAUSADA)
			printf("6.-Reanudar la planificación.\n");
		else
			printf("6.-Detener la planificación.\n");
		printf("7.-Obtener estado de los semáforos.\n");
		printf("8.-Obtener estado de las variables compartidas.\n");
		printf("9.-Ver valores de configuración.\n");
		scanf("%i",&opcion);
		switch(opcion){
			case 1:
				system("clear");
				printf("Seleccione una de las siguientes opciones ingresando el número correspondiente.\n");
				printf("1.-Obtener listado completo de procesos.\n");
				printf("2.-Obtener cola de new.\n");
				printf("3.-Obtener cola de ready.\n");
				printf("4.-Obtener cola de exec.\n");
				printf("5.-Obtener cola de block.\n");
				printf("6.-Obtener cola de exit.\n");
				printf("7.-Volver al menú principal.\n");
				scanf("%i",&opcion2);
				switch(opcion2){
					case 1: // MOSTRAR TODOS LOS PROCESOS
						system("clear");
						if (CANTIDADNEWS==0)
							printf("Cola new vacía.\n");
							
						else{
							printf("Cola new:\n");
							for (i = 0; i < CANTIDADNEWS; i++)
								printf("\t%i\n",COLANEW[i]);
						}
						i=0;
						if (CANTIDADREADYS==0)
							printf("Cola ready vacía.\n");
						else{
							printf("Cola ready:\n");
							i=0;
							for(;i<CANTIDADREADYS;i++){
								printf("\t%i\n",COLAREADY[i]);
							}
						}
						if (CANTIDADEXECS==0)
							printf("Cola exec vacía.\n");
						else{
							printf("Cola exec:\n");
							for (i = 0; i < CANTIDADEXECS; i++)
								printf("\t%i\n",COLAEXEC[i]);
						}
						if (CANTIDADBLOCKS==0)
							printf("Cola block vacía.\n");
						else{
							printf("Cola block:\n");
							for (i = 0; i < CANTIDADBLOCKS; i++)
								printf("\t%i\n",COLABLOCK[i]);
						}
						if (CANTIDADEXITS==0)
							printf("Cola exit vacía.\n");
						else{
							printf("Cola exit:\n");
							for (i = 0; i < CANTIDADEXITS; i++)
								printf("\t%i\n",COLAEXIT[i]);
						}
						printf("Presione enter para volver al menú principal.\n");
						getchar();
						getchar();
					break;
					case 2: // MOSTRAR COLA DE NEW
						system("clear");
						if (CANTIDADNEWS==0)
							printf("Cola new vacía.\n");
						else{
							printf("Cola new:\n");
							for (i = 0; i < CANTIDADNEWS; i++)
								printf("\t%i\n",COLANEW[i]);
						}
						printf("Presione enter para volver al menú principal.\n");
						getchar();
						getchar();
					break;
					case 3: // MOSTRAR COLA DE READY
						system("clear");
						i=0;
						if (CANTIDADREADYS==0)
							printf("Cola ready vacía.\n");
						else{
							printf("Cola ready:\n");
							i=0;
							for(;i<CANTIDADREADYS;i++){
								printf("\t%i\n",COLAREADY[i]);
							}
						}
						printf("Presione enter para volver al menú principal.\n");
						getchar();
						getchar();
					break;
					case 4: // MOSTRAR COLA DE EXEC
						system("clear");
						if (CANTIDADEXECS==0)
							printf("Cola exec vacía.\n");
						else{
							printf("Cola exec:\n");
							for (i = 0; i < CANTIDADEXECS; i++)
								printf("\t%i\n",COLAEXEC[i]);
						}
						printf("Presione enter para volver al menú principal.\n");
						getchar();
						getchar();
					break;
					case 5: // MOSTRAR COLA DE BLOCK
						system("clear");
						if (CANTIDADBLOCKS==0)
							printf("Cola block vacía.\n");
						else{
							printf("Cola block:\n");
							for (i = 0; i < CANTIDADBLOCKS; i++)
								printf("\t%i\n",COLABLOCK[i]);
						}
						printf("Presione enter para volver al menú principal.\n");
						getchar();
						getchar();
					break;
					case 6: // MOSTRAR COLA DE EXIT
						system("clear");
						if (CANTIDADEXITS==0)
							printf("Cola exit vacía.\n");
						else{
							printf("Cola exit:\n");
							for (i = 0; i < CANTIDADEXITS; i++)
								printf("\t%i\n",COLAEXIT[i]);
						}
						printf("Presione enter para volver al menú principal.\n");
						getchar();
						getchar();
					break;
					case 7:
						system("clear");
					break;
				}
			break;
			case 2: // MOSTRAR INFO DE UN PROCESO
				system("clear");
				printf("Ingrese el PID del proceso.\n");
				scanf("%i",&pid);
				system("clear");
				i=0;
				while(i<CANTIDADPCBS && PCBS[i].pid!=pid)
					i++;
				if (i==CANTIDADPCBS)
					printf("No existe el proceso %i.\n", pid);
				else{
					printf("Proceso: %i\n", pid);
					printf("\tEstado: %i\n", PCBS[i].estado);
					printf("\tPC: %i\n", PCBS[i].programCounter);
					printf("\tRafagas Ejecutadas: %i\n", PCBS[i].rafagasEjecutadas);
					printf("\tPriviliegiadas Ejecutadas: %i\n", PCBS[i].privilegiadasEjecutadas);
					printf("\tPaginas Heap: %i\n", PCBS[i].paginasHeap);
					printf("\tAlocaciones: %i\n", PCBS[i].alocaciones);
					printf("\tBytes Alocados:: %i\n", PCBS[i].bytesAlocados);
					printf("\tLiberaciones: %i\n", PCBS[i].liberaciones);
					printf("\tBytes Liberados: %i\n", PCBS[i].bytesLiberados);
					printf("\tCantidad de archivos: %i\n", PCBS[i].cantidadArchivos);
					if (PCBS[i].cantidadArchivos!=0)
					{
						printf("\tReferencia a tabla de archivos:\n");
						for (j = 0; j < PCBS[i].cantidadArchivos; j++)
						{
							printf("\t\tFD[%i]:\tPermisos: '", j+3);
							if (PCBS[i].referenciaATabla[j].flags.creacion==1)
								printf("C");
							if (PCBS[i].referenciaATabla[j].flags.lectura==1)
								printf("L");
							if (PCBS[i].referenciaATabla[j].flags.escritura==1)
								printf("E");
							printf("'\tFD Global: %i\n",PCBS[i].referenciaATabla[j].globalFd);
							printf("'\tAbierto: %i\n",PCBS[i].referenciaATabla[j].abierto);
						}	
					}
					printf("\tPaginas de codigo: %i\n", PCBS[i].paginasCodigo);
					printf("\tCantidad de instrucciones: %i\n", PCBS[i].cantidadInstrucciones);
					printf("\tIndice Codigo:\n");
					for (j = 0; j < PCBS[i].cantidadInstrucciones; j++)
					{
						printf("\t\tInstruccion: %i,\tOffset: %i.\n", PCBS[i].indiceCodigo[j].start, PCBS[i].indiceCodigo[j].offset);
					}
					printf("\tEtiquetas Size: %i\n", PCBS[i].indiceEtiquetas.etiquetas_size);
					if (PCBS[i].indiceEtiquetas.etiquetas_size!=0)
					{
						printf("\tEtiquetas: \n\t%s\n", PCBS[i].indiceEtiquetas.etiquetas);
					}
					printf("\tPosicion stack: %i\n", PCBS[i].posicionStack);
					printf("\tCantidad de stack: %i\n", PCBS[i].cantidadStack);
					printf("\tIndice Stack:\n");
					for (j = 0; j < PCBS[i].cantidadStack; j++)
					{
						printf("\t[%i]\n", j);
						printf("\t\tCantidad Args: %i\n", PCBS[i].indiceStack[j].cantidadArgumentos);
						if (PCBS[i].indiceStack[j].cantidadArgumentos!=0)
						{
							printf("\t\tArgumentos:\n");
							for (k = 0; k < PCBS[i].indiceStack[j].cantidadArgumentos; k++)
							{
								printf("\t\tId: %c,\t",PCBS[i].indiceStack[j].argumentos[k].id);printf("Pagina: %i,\t",PCBS[i].indiceStack[j].argumentos[k].pagina);printf("Offset: %i,\t",PCBS[i].indiceStack[j].argumentos[k].offset);printf("Size: %i.\n",PCBS[i].indiceStack[j].argumentos[k].size);
							}
						}
						printf("\t\tCantidad Vars: %i\n", PCBS[i].indiceStack[j].cantidadVariables);
						if (PCBS[i].indiceStack[j].cantidadVariables!=0)
						{
							printf("\t\tVariables:\n");
							for (k = 0; k < PCBS[i].indiceStack[j].cantidadVariables; k++)
							{
								printf("\t\tId: %c,\t",PCBS[i].indiceStack[j].variables[k].id);printf("Pagina: %i,\t",PCBS[i].indiceStack[j].variables[k].pagina);printf("Offset: %i,\t",PCBS[i].indiceStack[j].variables[k].offset);printf("Size: %i.\n",PCBS[i].indiceStack[j].variables[k].size);
							}
						}
						if (j!=0)
						{
							printf("\t\tPosicion Retorno: %i\n", PCBS[i].indiceStack[j].posRetorno);
							printf("\t\tVariable Retorno:\n");
							printf("\t\t\tPagina: %i,\t",PCBS[i].indiceStack[j].varRetorno.pagina);printf("Offset: %i,\t",PCBS[i].indiceStack[j].varRetorno.offset);printf("Size: %i.\n",PCBS[i].indiceStack[j].varRetorno.size);
						}	
					}
					printf("\tExit Code: %i\n", PCBS[i].exitCode);
				}
				printf("Presione enter para volver al menú principal.\n");
				getchar();
				getchar();
			break;
			case 3: // MOSTRAR TABLA GLOBAL DE ARCHIVOS
				system("clear");
				printf("Tabla Global de Archivos:\n");
				if (CANTTABLAARCHIVOS==0)
				{
					printf("No hay archivos abiertos.\n");
				}
				for (i = 0; i < CANTTABLAARCHIVOS; i++)
				{
					printf("FD=%i:\n",tablaArchivos[i].fd);
					printf("\tPath: %s\n",tablaArchivos[i].path);
					printf("\tVeces abierto: %i.\n", tablaArchivos[i].vecesAbierto);
				}
				printf("Presione enter para volver al menú principal.\n");
				getchar();
				getchar();
			break;
			case 4: // MODIFICAR GRADO DE MULTIPROGRAMACION
				system("clear");
				printf("Grado de multiprogramación actual: %i.\n",GRADO_MULTIPROG );
				printf("Ingrese el nuevo.\n");
				pthread_mutex_lock(&mutexGradoMultiprog);
				scanf("%i",&GRADO_MULTIPROG);
				pthread_mutex_unlock(&mutexGradoMultiprog);
				printf("Grado de multiprogramación modificado correctamente: %i.\n", GRADO_MULTIPROG);
				printf("Presione enter para volver al menú principal.\n");
				getchar();
				getchar();
			break;
			case 5: // FINALIZAR UN PROCESO
				system("clear");
				printf("Ingrese PID del proceso a finalizar.\n");
				scanf("%i", &pid);
				system("clear");
				if (finalizarPid(pid,-6))
					printf("El proceso fue finalizado correctamente.\n");
				else
					printf("El proceso no existe o ya finalizó.\n");
				printf("Presione enter para volver al menú principal.\n");
				getchar();
				getchar();
			break;
			case 6: // DETENER/REANUDAR LA PLANIFICACION
				system("clear");
				pthread_mutex_lock(&mutexPausaPlanificacion);
				if (PLANIFICACIONPAUSADA){
					PLANIFICACIONPAUSADA=0;
				}
				else
					PLANIFICACIONPAUSADA=1;
				pthread_mutex_unlock(&mutexPausaPlanificacion);
				printf("Cambios realizados.\n");
				printf("Presione enter para volver al menú principal.\n");
				getchar();
				getchar();
			break;
			case 7: // SEMAFOROS
				system("clear");
				for (i = 0; i < CANTIDADSEM; i++)
				{
					printf("%s = %s\n", SEM_IDS[i], SEM_INIT[i]);
				}
				printf("Presione enter para volver al menú principal.\n");
				getchar();
				getchar();
			break;
			case 8: // VARS COMPARTIDAS
				system("clear");
				for (i = 0; i < CANTIDADVARS; i++)
				{
					printf("%s = %i\n", SHARED_VARS[i], SHARED_VALUES[i]);
				}
				printf("Presione enter para volver al menú principal.\n");
				getchar();
				getchar();
			break;
			case 9: // CONFIG
				system("clear");
				pthread_mutex_lock(&mutexPausaPlanificacion);
				printf("Puerto del kernel: %s\n", PUERTO_PROG);
				printf("Ip de memoria: %s\n", IP_MEMORIA);
				printf("Puerto de memoria: %s\n", PUERTO_MEMORIA);
				printf("Ip del file system: %s\n", IP_FS);
				printf("Puerto del file system: %s\n", PUERTO_FS);
				printf("Quantum: %i\n", QUANTUM);
				printf("Quantum sleep: %i\n", QUANTUM_SLEEP);
				printf("Algoritmo de planificación: %s\n", ALGORITMO);
				printf("Grado de multiprogramación: %i\n", GRADO_MULTIPROG);
				printf("Páginas de stack: %i\n", STACK_SIZE);
				printf("Presione enter para volver al menú principal.\n");
				getchar();
				getchar();
			break;
		}
	}
}

void quantum(){
	usleep(QUANTUM*(QUANTUM_SLEEP*1000));
	flagQuantum=1;
}

void sacarHeapMetadata(indexProceso,pagina,posicion){
	int i=posicion;
	for (; i < tablaHeap[indexProceso].paginas[pagina].cantidadMetadatas; i++)
	{
		if (!(i+1==tablaHeap[indexProceso].paginas[pagina].cantidadMetadatas))
		{
			memcpy(&(tablaHeap[indexProceso].paginas[pagina].contenido[i]),&(tablaHeap[indexProceso].paginas[pagina].contenido[i+1]),sizeof(t_heapMetaData));
		}
	}
	tablaHeap[indexProceso].paginas[pagina].cantidadMetadatas--;
	tablaHeap[indexProceso].paginas[pagina].contenido=realloc(tablaHeap[indexProceso].paginas[pagina].contenido,tablaHeap[indexProceso].paginas[pagina].cantidadMetadatas*sizeof(t_heapMetaData));
}



void planificar(dataParaComunicarse ** dataDePlanificacion){
	escribirEnArchivoLog("en planificacion", &KernelLog,nombreLog);
	int pid,estaba,_pid,cantBytes,posPagAux,posPagNueva,puntero,bytesLibres;
	int rv,i,j,k,l,globalFd,intAux,flag,cantUsados,usadosRecorridos,primerFree,cantLibres,proximoMetadata;
	char * aux;
	int aux1,aux2,aux3;
	t_path * path;
	int socket=(*dataDePlanificacion)->socket;
	free(*dataDePlanificacion);
	flagLiberarPlanificar=1;
	void * paginaAux,*paginaNueva;
	t_heapMetaData * metadataAux;
	t_pcb * pcb;
	int cpuLibre = 1,exitCode;
	int pagina,offset;
	flagQuantum=89;
	t_seleccionador * seleccionador;
	pthread_t hiloQuantum;
	int primerAcceso=1,error=0;
	int* buffer;
	t_solicitudValorVariable * solicitudVariable;
	t_asignarVariableCompartida * asignarVariable;
	t_solicitudSemaforo * solicitudSemaforo;
	t_reservarEspacioMemoria * reservarMemoria;
	t_liberarMemoria * liberarMemoria;
	t_abrirArchivo * abrirArchivo;
	t_borrarArchivo * borrarArchivo;
	t_cerrarArchivo * cerrarArchivo;
	t_moverCursor * moverCursor;
	t_escribirArchivo * escribirArchivo;
	t_leerArchivo * leerArchivo;
	t_mensaje * mensaje;
	t_fdParaLeer * fd;
	t_escribirArchivoFS * escribirArchivoFS;
	t_leerArchivoFS * leerArchivoFS;
	t_paqueteFS * paqueteFS,*paqueteFS2;
	t_peticionBytes * peticion;
	t_almacenarBytes * bytes;
	t_solicitudAsignar * solicitudPaginaHeap;
	t_reservar * reservar;
	t_liberarPagina * liberarPagina;
	while(PLANIFICACIONHABILITADA)
	{
		while(AFINALIZAR);
		seleccionador=malloc(sizeof(t_seleccionador));
		printf("primerAcceso: %i\n",primerAcceso );
		if (primerAcceso){
			seleccionador->tipoPaquete=PCB;
			primerAcceso=0;}
		else
			while(0>=(rv=recv(socket, seleccionador, sizeof(t_seleccionador), 0))){
				if (rv==0)
				{
					PLANIFICACIONHABILITADA=0;
					seleccionador->tipoPaquete=999;
					break;
				}
			}
		printf("Paquete: %i\n", seleccionador->tipoPaquete);
		switch(seleccionador->tipoPaquete)
		{	
			case 999:
			break;
			case ESPERONOVEDADES:
				if (!strcmp(ALGORITMO,"RR"))
				{	
					flagQuantum--;
				}
				escribirEnArchivoLog("en case espero novedades", &KernelLog,nombreLog);
				if (pid==PIDFIN) // SI FINALIZO EL PROCESO FORZADAMENTE MANDO FINALIZARPROCESO
				 	{printf("%i==%i\n",pid,PIDFIN );

				 		enviarDinamico(FINALIZARPROCESO, socket,NULL);
				 	escribirEnArchivoLog("envio finalizar proceso", &KernelLog,nombreLog);}
				else if (estaBlocked(pid)) // SI ESTA BLOQUEADO MANDO PARAREJECUCION
					{enviarDinamico(PARAREJECUCION,socket, NULL);
					escribirEnArchivoLog("envio parar ejecucion", &KernelLog,nombreLog);}
				else if (error!=0){
					enviarDinamico(FINALIZARPORERROR, socket,NULL);
					escribirEnArchivoLog("envio finalizar por error", &KernelLog,nombreLog);
				}
				else if (PCBS[pid].estado==EXIT && (PCBS[pid].exitCode==-7 || PCBS[pid].exitCode==-6)) // SI FINALIZO EL PROCESO FORZADAMENTE MANDO FINALIZARPROCESO
				 	{enviarDinamico(FINALIZARPROCESO, socket,NULL);
				 	escribirEnArchivoLog("envio finalizar proceso", &KernelLog,nombreLog);}
				else if (PCBS[pid].estado==EXIT && PCBS[pid].exitCode==0)
					{seleccionador->tipoPaquete=PCB;
					primerAcceso=1;}
				else if (flagQuantum==0) // SI TERMINO QUANTUM MANDO FINQUANTUM
					{enviarDinamico(FINQUANTUM, socket,NULL);
					escribirEnArchivoLog("envio fin de quantum", &KernelLog,nombreLog);}
				else // SI NO HAY NOVEDADES MANDO CONTINUAR
					{
					usleep(QUANTUM_SLEEP*1000);
					enviarDinamico(CONTINUAR, socket,NULL);
					escribirEnArchivoLog("envio continuar", &KernelLog,nombreLog);}		
				break;
			case PCB:	
						escribirEnArchivoLog("en case pcb", &KernelLog,nombreLog);
						pcb=malloc(sizeof(t_pcb));
						while(!hayProcesosReady() || PLANIFICACIONPAUSADA){
							rellenarColaReady();
						}
						pthread_mutex_lock(&mutexProcesosReady);
						// OBTENGO EL PRIMER PID DE LA COLA DE LISTOS
						pid = COLAREADY[0];
								pthread_mutex_lock(&mutexColaReady);
						for (i=0; i < CANTIDADREADYS; i++){
							if (i+1==CANTIDADREADYS) ;
							else {
								COLAREADY[i]=COLAREADY[i+1];
							}
						}
						CANTIDADREADYS--;
						COLAREADY=realloc(COLAREADY,CANTIDADREADYS*sizeof(int));
								pthread_mutex_unlock(&mutexColaReady);
						// LO SACO DE DICHA COLA Y OBTENGO SU PCB
						// CAMBIO ESTADO A EJECUTANDO
						cambiarEstado(pid,EXEC);
						memcpy(pcb,&(PCBS[pid]),sizeof(t_pcb));
						// LO SACO DE LA COLA DE READYS
						pthread_mutex_unlock(&mutexProcesosReady);
						// LO PONGO EN LA COLA DE EJECUTANDO
						pthread_mutex_lock(&mutexColaExec);
						CANTIDADEXECS++;
						COLAEXEC = realloc (COLAEXEC,(CANTIDADEXECS) * sizeof(COLAEXEC[0]));
						COLAEXEC[CANTIDADEXECS-1]=pid;
						pthread_mutex_unlock(&mutexColaExec);
						// LE MANDO EL PCB AL CPU
						usleep(QUANTUM_SLEEP*1000);
						enviarDinamico(PCB,socket,pcb);
						escribirEnArchivoLog("envio pcb", &KernelLog,nombreLog);
						if (!strcmp(ALGORITMO,"RR"))
						{	
							flagQuantum=QUANTUM;
						}
						free(pcb);
			break;
			// VARIABLES COMPARTIDAS
			case SOLICITUDVALORVARIABLE: // CPU PIDE EL VALOR DE UNA VARIABLE COMPARTIDA
				escribirEnArchivoLog("en case solicitud valor variable", &KernelLog,nombreLog);
				solicitudVariable=malloc(sizeof(t_solicitudValorVariable));
				recibirDinamico(SOLICITUDVALORVARIABLE,socket,solicitudVariable);
				pcb=malloc(sizeof(t_pcb));
				while(0>recv(socket, seleccionador, sizeof(t_seleccionador), 0));
				recibirDinamico(PCB,socket,pcb);
				pcb->privilegiadasEjecutadas++;
				escribirEnArchivoLog("recibo solicitud valor variable", &KernelLog,nombreLog);
				strip(&(solicitudVariable->variable));
				flag=1;
				for (i = 0; i < CANTIDADVARS; i++)
					if (!strcmp(SHARED_VARS[i]+1,solicitudVariable->variable))
					{
						send(socket,&(SHARED_VALUES[i]),sizeof(int),0);
						flag=0;
					}
				if(flag)
					error=-12; //SE INTENTA ACCEDER A UNA VARIABLE COMPARTIDA INEXISTENTE
				updatePCB(pcb);
				enviarDinamico(PCB,socket,pcb);
				free(pcb);
				free(solicitudVariable->variable);
				free(solicitudVariable);
			break;
			case ASIGNARVARIABLECOMPARTIDA: // CPU QUIERE ASIGNAR UN VALOR A UNA VARIABLE COMPARTIDA
				escribirEnArchivoLog("en case asignar variable compartida", &KernelLog,nombreLog);
				asignarVariable=malloc(sizeof(t_asignarVariableCompartida));
				recibirDinamico(ASIGNARVARIABLECOMPARTIDA,socket,asignarVariable);
				pcb=malloc(sizeof(t_pcb));
				while(0>recv(socket, seleccionador, sizeof(t_seleccionador), 0));
				recibirDinamico(PCB,socket,pcb);
				pcb->privilegiadasEjecutadas++;
				flag=1;
				escribirEnArchivoLog("recibo asignar variable compartida", &KernelLog,nombreLog);
				for (i = 0; i < CANTIDADVARS; i++)
				{
					if (!strcmp(SHARED_VARS[i]+1,asignarVariable->variable))
					{
						pthread_mutex_lock(&mutexCompartidas);
						SHARED_VALUES[i]=asignarVariable->valor;
						pthread_mutex_unlock(&mutexCompartidas);
						flag=0;
					}
				}
				if(flag)
					error=-12; //SE INTENTA ACCEDER A UNA VARIABLE COMPARTIDA INEXISTENTE
				updatePCB(pcb);
				enviarDinamico(PCB,socket,pcb);
				free(pcb);
				free(asignarVariable->variable);
				free(asignarVariable);
			break;
			// SEMAFOROS
			case SOLICITUDSEMWAIT: // CPU ESTA HACIENDO WAIT (VER ESTRUCTURA)
				pthread_mutex_lock(&mutexSemaforos);
				escribirEnArchivoLog("en case solicitud sem wait", &KernelLog,nombreLog);
				solicitudSemaforo=malloc(sizeof(t_solicitudSemaforo));
				recibirDinamico(SOLICITUDSEMWAIT,socket,solicitudSemaforo);
				escribirEnArchivoLog("recibo solicitud sem wait", &KernelLog,nombreLog);
				strip(&(solicitudSemaforo->identificadorSemaforo));
				flag=1;
				for (i = 0; i < CANTIDADSEM; i++)
				{
					if (!strcmp(SEM_IDS[i],solicitudSemaforo->identificadorSemaforo))
					{
						if (atoi(SEM_INIT[i])>0)
						{
							intAux=atoi(SEM_INIT[i]);
							intAux--;
							aux=calloc(1,10);
							sprintf(aux, "%i", intAux);
							strcpy(SEM_INIT[i],aux);
							free(aux);
						}
						else
						{
							pthread_mutex_lock(&mutexColaBlock);
							CANTIDADBLOCKS++;
							COLABLOCK = realloc (COLABLOCK,(CANTIDADBLOCKS) * sizeof(COLABLOCK[0]));
							COLABLOCK[CANTIDADBLOCKS-1]=pid;
							pthread_mutex_unlock(&mutexColaBlock);
							CANTIDADBLOCKPORSEM[i]++;
							BLOQUEADOSPORSEM[i]=realloc(BLOQUEADOSPORSEM[i],CANTIDADBLOCKPORSEM[i]*sizeof(int));
							(BLOQUEADOSPORSEM[i])[CANTIDADBLOCKPORSEM[i]-1]=pid;
							j=0;
							for (; j < CANTIDADEXECS; j++)
							{
								if (COLAEXEC[j]==pid)
								{
									break;
								}
							}
							for (j; j < CANTIDADEXECS; j++)
							{
								pthread_mutex_lock(&mutexColaExec);
								if (j+1==CANTIDADEXECS);
								else
									COLAEXEC[j]=COLAEXEC[j+1];
								pthread_mutex_unlock(&mutexColaExec);
							}
							pthread_mutex_lock(&mutexColaExec);
							CANTIDADEXECS--;
							COLAEXEC=realloc(COLAEXEC,CANTIDADEXECS*sizeof(int));
							pthread_mutex_unlock(&mutexColaExec);
							pthread_mutex_lock(&mutexPcbs);
							PCBS[pid].estado=BLOCKED;
							pthread_mutex_unlock(&mutexPcbs);

						}
					flag=0;
					}
				}
				if(flag)
					error=-13; //SE INTENTA UTILIZAR UN SEMÁFORO INEXISTENTE
				free(solicitudSemaforo->identificadorSemaforo);
				free(solicitudSemaforo);
				pthread_mutex_unlock(&mutexSemaforos);
			break;
			case SOLICITUDSEMSIGNAL: // CPU ESTA HACIENDO SIGNAL (VER ESTRUCTURA)
				pthread_mutex_lock(&mutexSemaforos);
				escribirEnArchivoLog("en case solicitud sem signal", &KernelLog,nombreLog);
				solicitudSemaforo=malloc(sizeof(t_solicitudSemaforo));
				recibirDinamico(SOLICITUDSEMSIGNAL,socket,solicitudSemaforo);
				escribirEnArchivoLog("recibo solicitud sem signal", &KernelLog,nombreLog);
				strip(&(solicitudSemaforo->identificadorSemaforo));
				flag=1;
				for (i = 0; i < CANTIDADSEM; i++)
				{
					if (!strcmp(SEM_IDS[i],solicitudSemaforo->identificadorSemaforo))
					{
						if(CANTIDADBLOCKPORSEM[i]!=0)
						{
							_pid=BLOQUEADOSPORSEM[i][0];
							for (j = 0; j < CANTIDADBLOCKPORSEM[i]; j++)
							{
								if (j+1!=CANTIDADBLOCKPORSEM[i])
								{
									BLOQUEADOSPORSEM[i][j]=BLOQUEADOSPORSEM[i][j+1];
								}
							}
							CANTIDADBLOCKPORSEM[i]--;
							BLOQUEADOSPORSEM[i]=realloc(BLOQUEADOSPORSEM[i],CANTIDADBLOCKPORSEM[i]*sizeof(int));
							j=0;
							for (; j < CANTIDADBLOCKS; j++)
							{
								if (COLABLOCK[j]==_pid)
								{
									break;
								}
							}
							for (; j < CANTIDADBLOCKS; j++)
							{
								pthread_mutex_lock(&mutexColaBlock);
								if (j+1==CANTIDADBLOCKS);
								else
									COLABLOCK[j]=COLABLOCK[j+1];
								pthread_mutex_unlock(&mutexColaBlock);
							}
							pthread_mutex_lock(&mutexColaBlock);
							CANTIDADBLOCKS--;
							COLABLOCK=realloc(COLABLOCK,CANTIDADBLOCKS*sizeof(COLABLOCK[0]));
							pthread_mutex_unlock(&mutexColaBlock);
							pthread_mutex_lock(&mutexColaReady);
							CANTIDADREADYS++;
							COLAREADY=realloc(COLAREADY,CANTIDADREADYS*sizeof(int));
							COLAREADY[CANTIDADREADYS-1]=_pid;
							pthread_mutex_unlock(&mutexColaReady);
							pthread_mutex_lock(&mutexPcbs);
							PCBS[_pid].estado=READY;
							pthread_mutex_unlock(&mutexPcbs);
						}
						else if (CANTIDADBLOCKPORSEM[i]==0)
						{
							intAux=atoi(SEM_INIT[i]);
							intAux++;
							aux=calloc(1,10);
							sprintf(aux, "%i", intAux);
							strcpy(SEM_INIT[i],aux);
							free(aux);
						}
					flag=0;
					}
				}
				if(flag)
					error=-13; //SE INTENTA UTILIZAR UN SEMÁFORO INEXISTENTE
				free(solicitudSemaforo->identificadorSemaforo);
				free(solicitudSemaforo);
				pthread_mutex_unlock(&mutexSemaforos);
			break;
			// HEAP
			case RESERVARESPACIO: // CPU RESERVA LUGAR EN EL HEAP
				escribirEnArchivoLog("en case reservar espacio", &KernelLog,nombreLog);
				reservarMemoria=malloc(sizeof(t_reservarEspacioMemoria));
				
						pcb=malloc(sizeof(t_pcb));
				recibirDinamico(RESERVARESPACIO,socket,reservarMemoria);
				escribirEnArchivoLog("recibo reservar espacio", &KernelLog,nombreLog);
				while(0>recv(socket, seleccionador, sizeof(t_seleccionador), 0));
				recibirDinamico(PCB,socket,pcb);
				escribirEnArchivoLog("recibo pcb", &KernelLog,nombreLog);
				pcb->alocaciones++;
				pcb->bytesAlocados+=reservarMemoria->espacio;
				pcb->privilegiadasEjecutadas++;
				pthread_mutex_lock(&mutexAlocar);
				printf("reservarMemoria->espacio: %i\n", reservarMemoria->espacio);
				if (reservarMemoria->espacio>TAMPAGINA-2*sizeof(t_heapMetaData))
				{
					error=-8;
				}
				else
				{
					for (i = 0; i < CANTTABLAHEAP; i++)
					{
						if (tablaHeap[i].pid==pid)
						{
							break;
						}
					}
					for (j = 0; j < tablaHeap[i].cantPaginas; j++)
					{
						if (tablaHeap[i].paginas[j].espacioLibre>=(reservarMemoria->espacio)+sizeof(t_heapMetaData))
						{
							break;
						}
					}
					if (j!=tablaHeap[i].cantPaginas)
					{	
						flag=1;
						for (k = 0; (k<tablaHeap[i].paginas[j].cantidadMetadatas) && flag; k++)
						{	
							
							if (k==tablaHeap[i].paginas[j].cantidadMetadatas-1)
							{
								aux1=reservarMemoria->espacio;
								aux2=tablaHeap[i].paginas[j].contenido[k].size ;
								aux3=sizeof(t_heapMetaData);
								if ((aux1<=(aux2-aux3)) && tablaHeap[i].paginas[j].contenido[k].isFree)
								{
									tablaHeap[i].paginas[j].contenido[k].isFree=false;
									intAux=tablaHeap[i].paginas[j].contenido[k].size;
									tablaHeap[i].paginas[j].contenido[k].size=reservarMemoria->espacio;
									tablaHeap[i].paginas[j].cantidadMetadatas++;
									tablaHeap[i].paginas[j].contenido=realloc(tablaHeap[i].paginas[j].contenido,tablaHeap[i].paginas[j].cantidadMetadatas*sizeof(t_heapMetaData));
									tablaHeap[i].paginas[j].contenido[k+1].isFree=true;
									tablaHeap[i].paginas[j].contenido[k+1].size=intAux-reservarMemoria->espacio-sizeof(t_heapMetaData);
									tablaHeap[i].paginas[j].espacioLibre+=-sizeof(t_heapMetaData)-reservarMemoria->espacio;
									flag=0;
									}
							}
							else{
								if ((reservarMemoria->espacio<=(tablaHeap[i].paginas[j].contenido[k].size)) && tablaHeap[i].paginas[j].contenido[k].isFree)
								{	if (reservarMemoria->espacio==(tablaHeap[i].paginas[j].contenido[k].size))
									{	tablaHeap[i].paginas[j].contenido[k].isFree=false;
										tablaHeap[i].paginas[j].espacioLibre-=reservarMemoria->espacio;
										flag=0;
									}
									else{
										tablaHeap[i].paginas[j].contenido[k].isFree=false;
										intAux=tablaHeap[i].paginas[j].contenido[k].size;
										tablaHeap[i].paginas[j].contenido[k].size=reservarMemoria->espacio;
										tablaHeap[i].paginas[j].cantidadMetadatas++;
										tablaHeap[i].paginas[j].contenido=realloc(tablaHeap[i].paginas[j].contenido,tablaHeap[i].paginas[j].cantidadMetadatas*sizeof(t_heapMetaData));
										tablaHeap[i].paginas[j].contenido[k+1].isFree=true;
										tablaHeap[i].paginas[j].contenido[k+1].size=intAux-reservarMemoria->espacio-sizeof(t_heapMetaData);
										tablaHeap[i].paginas[j].espacioLibre+=-sizeof(t_heapMetaData)-reservarMemoria->espacio;
										flag=0;
										}
								}
								}
						}
						k--;
						puntero=(tablaHeap[i].paginas[j].pagina-pcb->paginasCodigo)*TAMPAGINA;
						for (l = 0; l < k; l++)
						{
							puntero+=sizeof(t_heapMetaData)+tablaHeap[i].paginas[j].contenido[l].size;
							
						}
						
						puntero+=sizeof(t_heapMetaData);
					}
					else
					{	solicitudPaginaHeap=malloc(sizeof(t_solicitudAsignar));
						solicitudPaginaHeap->pid=pid;
						solicitudPaginaHeap->paginasAAsignar=1;
						enviarDinamico(ASIGNARPAGINAS,SOCKETMEMORIA,solicitudPaginaHeap);
						escribirEnArchivoLog("envio asignar paginas", &KernelLog,nombreLog);
						free(solicitudPaginaHeap);
						while(0>recv(SOCKETMEMORIA,&rv,sizeof(int),0)){
								perror("asd:");
						}
						if (rv==-1)
						{
							error=-9;
						}
						else{
							tablaHeap[i].cantPaginas++;
							tablaHeap[i].paginas=realloc(tablaHeap[i].paginas,tablaHeap[i].cantPaginas*sizeof(t_heapProceso));
							tablaHeap[i].paginas[tablaHeap[i].cantPaginas-1].pagina=pcb->paginasCodigo+STACK_SIZE+pcb->paginasHeap;
							pcb->paginasHeap++;
							tablaHeap[i].paginas[tablaHeap[i].cantPaginas-1].espacioLibre=TAMPAGINA-2*sizeof(t_heapMetaData)-reservarMemoria->espacio;
							tablaHeap[i].paginas[tablaHeap[i].cantPaginas-1].cantidadMetadatas=2;
							tablaHeap[i].paginas[tablaHeap[i].cantPaginas-1].contenido=malloc(2*sizeof(t_heapMetaData));
							tablaHeap[i].paginas[tablaHeap[i].cantPaginas-1].contenido[0].isFree=false;
							tablaHeap[i].paginas[tablaHeap[i].cantPaginas-1].contenido[0].size=reservarMemoria->espacio;
							tablaHeap[i].paginas[tablaHeap[i].cantPaginas-1].contenido[1].isFree=true;
							tablaHeap[i].paginas[tablaHeap[i].cantPaginas-1].contenido[1].size=TAMPAGINA-reservarMemoria->espacio-2*sizeof(t_heapMetaData);
							}
						puntero=(tablaHeap[i].paginas[j].pagina-pcb->paginasCodigo)*TAMPAGINA;
						puntero+=sizeof(t_heapMetaData);
						
						
					}
					
				}
				pthread_mutex_unlock(&mutexAlocar);
				reservar=malloc(sizeof(t_heapMetaData));
				reservar->puntero=puntero;
				enviarDinamico(RESERVADOESPACIO,socket,reservar);
				escribirEnArchivoLog("envio resevar espacio", &KernelLog,nombreLog);
				updatePCB(pcb);
				enviarDinamico(PCB,socket,pcb);
				escribirEnArchivoLog("envio pcb", &KernelLog,nombreLog);
				free(reservarMemoria);
				free(reservar);
						free(pcb);
				
			break;
			case LIBERARESPACIOMEMORIA: // CPU LIBERA MEMORIA DEL HEAP
				escribirEnArchivoLog("en case liberar espacio memoria", &KernelLog,nombreLog);
				liberarMemoria=malloc(sizeof(t_liberarMemoria));
				recibirDinamico(LIBERARESPACIOMEMORIA,socket,liberarMemoria);
				escribirEnArchivoLog("recibo liberar espacio memoria", &KernelLog,nombreLog);
				while(0>recv(socket, seleccionador, sizeof(t_seleccionador), 0));
				
						pcb=malloc(sizeof(t_pcb));
				recibirDinamico(PCB,socket,pcb);
				escribirEnArchivoLog("recibo pcb", &KernelLog,nombreLog);
				pcb->privilegiadasEjecutadas++;
				pcb->liberaciones++;
				pagina=liberarMemoria->direccionMemoria/TAMPAGINA+pcb->paginasCodigo;
				offset=liberarMemoria->direccionMemoria%TAMPAGINA;
				pthread_mutex_lock(&mutexAlocar);
				aux1=-1;
				aux2=-1;
				for (i = 0; i < CANTTABLAHEAP; i++)
				{
					if (tablaHeap[i].pid==pcb->pid)
					{
						for (j = 0; j < tablaHeap[i].cantPaginas; j++)
						{
							if (tablaHeap[i].paginas[j].pagina==pagina)
							{
								for (k = 0; k < tablaHeap[i].paginas[j].cantidadMetadatas; k++)
								{
									offset-=sizeof(t_heapMetaData);
									if (offset==0)
									{
										tablaHeap[i].paginas[j].espacioLibre+=tablaHeap[i].paginas[j].contenido[k].size;
										pcb->bytesLiberados+=tablaHeap[i].paginas[j].contenido[k].size;
										tablaHeap[i].paginas[j].contenido[k].isFree=true;
										if (tablaHeap[i].paginas[j].contenido[k+1].isFree)
										{
											if (k+1==tablaHeap[i].paginas[j].cantidadMetadatas-1)
											{
												if (tablaHeap[i].paginas[j].contenido[k+1].size==0)
												{
													tablaHeap[i].paginas[j].contenido[k].size+=sizeof(t_heapMetaData);
												}
												else{
													tablaHeap[i].paginas[j].contenido[k].size+=tablaHeap[i].paginas[j].contenido[k+1].size+sizeof(t_heapMetaData);
												}
											}
											else{
												tablaHeap[i].paginas[j].contenido[k].size+=tablaHeap[i].paginas[j].contenido[k+1].size+sizeof(t_heapMetaData);
												
											}
											sacarHeapMetadata(i,j,k+1);
											tablaHeap[i].paginas[j].espacioLibre+=sizeof(t_heapMetaData);
										}
										if ( k!=0 && tablaHeap[i].paginas[j].contenido[k-1].isFree)
										{
											tablaHeap[i].paginas[j].contenido[k-1].size+=tablaHeap[i].paginas[j].contenido[k].size+sizeof(t_heapMetaData);
											sacarHeapMetadata(i,j,k);
											tablaHeap[i].paginas[j].espacioLibre+=sizeof(t_heapMetaData);
										}
										break;
									}
									else{
										offset-=tablaHeap[i].paginas[j].contenido[k].size;
									}
								}
							break;
							}
						}
					break;
					}
				}

				if (tablaHeap[i].paginas[j].espacioLibre==TAMPAGINA-sizeof(t_heapMetaData))
				{
					liberarPagina=malloc(sizeof(t_liberarPagina));
					liberarPagina->pid=tablaHeap[i].pid;
					liberarPagina->pagina=tablaHeap[i].paginas[j].pagina;
					enviarDinamico(LIBERARPAGINA,SOCKETMEMORIA,liberarPagina);
					while(0>recv(SOCKETMEMORIA,&rv,sizeof(int),0));
					if (rv==-1)
					{
						error=-5;
					}
					for (k = j; k < tablaHeap[i].cantPaginas; k++)
					{
						if (!(k+1==tablaHeap[i].cantPaginas))
						{
							memcpy(&(tablaHeap[i].paginas[k]),&(tablaHeap[i].paginas[k+1]),sizeof(t_heapProceso));
						}
					}
					tablaHeap[i].cantPaginas--;
					tablaHeap[i].paginas=realloc(tablaHeap[i].paginas,tablaHeap[i].cantPaginas*sizeof(t_heapProceso));
					free(liberarPagina);
				}

				pthread_mutex_unlock(&mutexAlocar);
				updatePCB(pcb);
				enviarDinamico(PCB,socket,pcb);
				escribirEnArchivoLog("envio pcb", &KernelLog,nombreLog);
				free(liberarMemoria);
						free(pcb);
				
			break;
			// FILE SYSTEM
			case ABRIRARCHIVO: // CPU ABRE ARCHIVO
				escribirEnArchivoLog("en case abrir archivo", &KernelLog,nombreLog);
				abrirArchivo=malloc(sizeof(t_abrirArchivo));
				recibirDinamico(ABRIRARCHIVO,socket,abrirArchivo);
				escribirEnArchivoLog("recibo abrir archivo", &KernelLog,nombreLog);
				while(0>recv(socket, seleccionador, sizeof(t_seleccionador), 0));
				path=malloc(sizeof(t_path));
					path->path=calloc(1,abrirArchivo->tamanio+1);
				
						pcb=malloc(sizeof(t_pcb));
				recibirDinamico(PCB,socket,pcb);
				escribirEnArchivoLog("recibo pcb", &KernelLog,nombreLog);
				pcb->privilegiadasEjecutadas++;
				estaba=0;
				flag=0;
				for (i = 0; i < proximoFd; i++)
				{
					
					if (!strcmp(tablaArchivos[i].path,abrirArchivo->direccionArchivo))
					{
						globalFd=i;
						estaba=1;
					}
				}
				if (!estaba){
					strip(&(abrirArchivo->direccionArchivo));
					strcpy(path->path,abrirArchivo->direccionArchivo);
					strip(&(path->path));
					path->tamanio=strlen(path->path);
					enviarDinamico(VALIDARARCHIVO,SOCKETFS,path);
					escribirEnArchivoLog("envio validar archivo", &KernelLog,nombreLog);
					while(0>recv(SOCKETFS,&rv,sizeof(int),0));
					
						pthread_mutex_lock(&mutexTablaArchivos);
						globalFd=proximoFd;
						proximoFd++;
						CANTTABLAARCHIVOS++;
						tablaArchivos=realloc(tablaArchivos,CANTTABLAARCHIVOS*sizeof(t_tablaGlobalArchivos));
						tablaArchivos[CANTTABLAARCHIVOS-1].path=calloc(1,path->tamanio+1);
						tablaArchivos[CANTTABLAARCHIVOS-1].tamanioPath=path->tamanio;
						strip(&(abrirArchivo->direccionArchivo));
						strcpy(tablaArchivos[CANTTABLAARCHIVOS-1].path,abrirArchivo->direccionArchivo);
						tablaArchivos[CANTTABLAARCHIVOS-1].vecesAbierto=1;
						tablaArchivos[CANTTABLAARCHIVOS-1].fd=globalFd;
						pthread_mutex_unlock(&mutexTablaArchivos);

					if (abrirArchivo->flags.creacion && rv==-1)
					{
						enviarDinamico(CREARARCHIVOFS,SOCKETFS,path);
						escribirEnArchivoLog("envio crear archivo fs", &KernelLog,nombreLog);
					}
					else if (rv==-1 && !(abrirArchivo->flags.creacion)){
						flag=1;
						error=-10; // NO EXISTE EL ARCHIVO Y NO SE TIENEN LOS PERMISOS PARA CREARLO
					}
				}
				if(estaba){
					pthread_mutex_lock(&mutexTablaArchivos);
					tablaArchivos[globalFd].vecesAbierto++;
					pthread_mutex_unlock(&mutexTablaArchivos);
				}
				if (!flag)
				{
					pcb->cantidadArchivos++;
					if (pcb->cantidadArchivos==1)
					{
						pcb->referenciaATabla=malloc(sizeof(t_tablaArchivosDeProcesos));
					}
					else
						pcb->referenciaATabla=realloc(pcb->referenciaATabla,pcb->cantidadArchivos*sizeof(t_tablaArchivosDeProcesos));
					memcpy(&(pcb->referenciaATabla[pcb->cantidadArchivos-1].flags),&(abrirArchivo->flags),sizeof(t_banderas));
					pcb->referenciaATabla[pcb->cantidadArchivos-1].globalFd=globalFd;
					pcb->referenciaATabla[pcb->cantidadArchivos-1].cursor=0;
					pcb->referenciaATabla[pcb->cantidadArchivos-1].abierto=1;
				}
				updatePCB(pcb);
				enviarDinamico(PCB,socket,pcb);
				escribirEnArchivoLog("envio pcb", &KernelLog,nombreLog);
				fd=malloc(sizeof(t_fdParaLeer));
				fd->fd=pcb->cantidadArchivos-1+3;
				enviarDinamico(ABRIOARCHIVO,socket,fd);
				escribirEnArchivoLog("envio abrir archivo", &KernelLog,nombreLog);
				free(path->path);
				free(path);
				free(fd);
				free(abrirArchivo->direccionArchivo);
				free(abrirArchivo);
						free(pcb);
				
			break;
			case BORRARARCHIVO: // CPU BORRA ARCHIVO
				escribirEnArchivoLog("en case borrar archivo", &KernelLog,nombreLog);
				borrarArchivo=malloc(sizeof(t_borrarArchivo));
				recibirDinamico(BORRARARCHIVO,socket,borrarArchivo);
				escribirEnArchivoLog("recibo borrar archivo", &KernelLog,nombreLog);
				while(0>recv(socket, seleccionador, sizeof(t_seleccionador), 0));
				
						pcb=malloc(sizeof(t_pcb));
				recibirDinamico(PCB,socket,pcb);
				escribirEnArchivoLog("recibo pcb", &KernelLog,nombreLog);
				pcb->privilegiadasEjecutadas++;
				globalFd=pcb->referenciaATabla[(borrarArchivo->fdABorrar)-3].globalFd;
				if (pcb->referenciaATabla[(borrarArchivo->fdABorrar)-3].abierto==0)
				{
					error=-14;
				}
				else
				{
					for (i = 0; i < CANTTABLAARCHIVOS; i++)
					{
						if (globalFd==tablaArchivos[i].fd)
						{
							break;
						}
					}
					if (tablaArchivos[i].vecesAbierto!=1)
					{
						error=-11;  // SE INTENTA BORRAR UN ARCHIVO QUE OTRO PROCESO TIENE ABIERTO
					}
					else{
						path=malloc(sizeof(t_path));
						path->path=calloc(1,tablaArchivos[i].tamanioPath+1);
						path->tamanio=tablaArchivos[i].tamanioPath;
						strcpy(path->path,tablaArchivos[i].path);
						strip(&(path->path));
						path->tamanio=strlen(path->path);
						enviarDinamico(BORRARARCHIVOFS,SOCKETFS,path);
						escribirEnArchivoLog("envio borrar archivo fs", &KernelLog,nombreLog);
						free(path);
						// pthread_mutex_lock(&mutexTablaArchivos);
						// for (; i < CANTTABLAARCHIVOS; i++)
						// {
						// 	if (!(i+1==CANTTABLAARCHIVOS))
						// 	{
						// 		memcpy(&tablaArchivos[i],&tablaArchivos[i+1],sizeof(t_tablaGlobalArchivos));
						// 	}
						// }
						// CANTTABLAARCHIVOS--;
						// tablaArchivos=realloc(tablaArchivos,CANTTABLAARCHIVOS*sizeof(t_tablaGlobalArchivos));
						// pthread_mutex_unlock(&mutexTablaArchivos);
					}
					// pcb->referenciaATabla[(borrarArchivo->fdABorrar)-3].abierto=0;
				}
				updatePCB(pcb);
				enviarDinamico(PCB,socket,pcb);
				escribirEnArchivoLog("envio pcb", &KernelLog,nombreLog);
				free(borrarArchivo);
						free(pcb);
				
			break;
			case CERRARARCHIVO: // CPU CIERRA ARCHIVO
				escribirEnArchivoLog("en case cerrar archivo", &KernelLog,nombreLog);
				cerrarArchivo=malloc(sizeof(t_cerrarArchivo));
				recibirDinamico(CERRARARCHIVO,socket,cerrarArchivo);
				escribirEnArchivoLog("recibo cerrar archivo", &KernelLog,nombreLog);
				while(0>recv(socket, seleccionador, sizeof(t_seleccionador), 0));
				
						pcb=malloc(sizeof(t_pcb));
				recibirDinamico(PCB,socket,pcb);
				escribirEnArchivoLog("recibo pcb", &KernelLog,nombreLog);
				pcb->privilegiadasEjecutadas++;
				globalFd=pcb->referenciaATabla[(cerrarArchivo->descriptorArchivo)-3].globalFd;
				if (pcb->referenciaATabla[(cerrarArchivo->descriptorArchivo)-3].abierto==0)
				{
					error=-14;
				}
				else{
					for (i = 0; i < CANTTABLAARCHIVOS; i++)
					{
						if (globalFd==tablaArchivos[i].fd)
						{
							break;
						}
					}
					pthread_mutex_lock(&mutexTablaArchivos);
					tablaArchivos[i].vecesAbierto--;
					if (tablaArchivos[i].vecesAbierto==0)
					{
						for (; i < CANTTABLAARCHIVOS; i++)
							{
								if (!(i+1==CANTTABLAARCHIVOS))
								{
									memcpy(&tablaArchivos[i],&tablaArchivos[i+1],sizeof(t_tablaGlobalArchivos));
								}
							}
						CANTTABLAARCHIVOS--;
						proximoFd--;
						tablaArchivos=realloc(tablaArchivos,CANTTABLAARCHIVOS*sizeof(t_tablaGlobalArchivos));
					}
					pthread_mutex_unlock(&mutexTablaArchivos);
				}
				updatePCB(pcb);
				enviarDinamico(PCB,socket,pcb);
				escribirEnArchivoLog("envio pcb", &KernelLog,nombreLog);
				free(cerrarArchivo);
						free(pcb);
				
			break;
			case MOVERCURSOR: // CPU MUEVE EL CURSOR DE UN ARCHIVO
				escribirEnArchivoLog("en case mover cursor", &KernelLog,nombreLog);
				moverCursor=malloc(sizeof(t_moverCursor));
				recibirDinamico(MOVERCURSOR,socket,moverCursor);
				escribirEnArchivoLog("recibo mover cursor", &KernelLog,nombreLog);
				while(0>recv(socket, seleccionador, sizeof(t_seleccionador), 0));
				pcb=malloc(sizeof(t_pcb));
				recibirDinamico(PCB,socket,pcb);
				escribirEnArchivoLog("recibo pcb", &KernelLog,nombreLog);
				pcb->privilegiadasEjecutadas++;
				if (pcb->referenciaATabla[(moverCursor->descriptorArchivo)-3].abierto==0)
				{
					error=-14;
				}
				else
					pcb->referenciaATabla[(moverCursor->descriptorArchivo)-3].cursor=moverCursor->posicion;
				updatePCB(pcb);
				enviarDinamico(PCB,socket,pcb);
				free(moverCursor);
						free(pcb);
				
			break;
			case ESCRIBIRARCHIVO: // CPU ESCRIBE EN UN ARCHIVO
			escribirEnArchivoLog("en case escribir archivo", &KernelLog,nombreLog);
				escribirArchivo=malloc(sizeof(t_escribirArchivo));
				recibirDinamico(ESCRIBIRARCHIVO,socket,escribirArchivo);
				escribirEnArchivoLog("recibo escribir archivo", &KernelLog,nombreLog);
				while(0>recv(socket, seleccionador, sizeof(t_seleccionador), 0));				
				mensaje=malloc(sizeof(t_mensaje));
				mensaje->mensaje=calloc(1,escribirArchivo->tamanio+1);
				pcb=malloc(sizeof(t_pcb));
				recibirDinamico(PCB,socket,pcb);
				escribirEnArchivoLog("recibo pcb", &KernelLog,nombreLog);
					if (escribirArchivo->fdArchivo==1)
					{
						mensaje->tamanio=escribirArchivo->tamanio;
						strcpy(mensaje->mensaje,(char*)escribirArchivo->informacion);
						enviarDinamico(PRINT,SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],mensaje);
						send(SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],&(pcb->pid),sizeof(int),0);
						escribirEnArchivoLog("envio mensaje", &KernelLog,nombreLog);
					}
					else{
						if (pcb->referenciaATabla[(escribirArchivo->fdArchivo)-3].abierto==0)
						{
							error-14;
						}
						pcb->privilegiadasEjecutadas++;
						if (pcb->referenciaATabla[(escribirArchivo->fdArchivo)-3].flags.escritura)
						{
							escribirArchivoFS=malloc(sizeof(t_escribirArchivoFS));
							escribirArchivoFS->size=escribirArchivo->tamanio;
							escribirArchivoFS->buffer=calloc(1,escribirArchivoFS->size+1);
							memcpy(escribirArchivoFS->buffer,escribirArchivo->informacion,escribirArchivoFS->size);
							escribirArchivoFS->offset=pcb->referenciaATabla[(escribirArchivo->fdArchivo)-3].cursor;
							globalFd=pcb->referenciaATabla[(escribirArchivo->fdArchivo)-3].globalFd;
							for (i = 0; i < CANTTABLAARCHIVOS; i++)
							{	if (globalFd==tablaArchivos[i].fd)
								{
									strip(&(tablaArchivos[i].path));
									break;
								}
							}
							escribirArchivoFS->tamanioPath=tablaArchivos[i].tamanioPath;
							escribirArchivoFS->path=calloc(1,escribirArchivoFS->tamanioPath+1);
							strcpy(escribirArchivoFS->path,tablaArchivos[i].path);
							strip(&(escribirArchivoFS->path));
							escribirArchivoFS->tamanioPath=strlen(escribirArchivoFS->path);
							enviarDinamico(GUARDARDATOSFS,SOCKETFS,escribirArchivoFS);
							escribirEnArchivoLog("envio guardar datos fs", &KernelLog,nombreLog);
							while(0>recv(SOCKETFS,&rv,sizeof(int),0));
							if (!rv)
							{
								error=-2;
							}
							free(escribirArchivoFS->path);
							free(escribirArchivoFS->buffer);
							free(escribirArchivoFS);
						}
						else{
							error=-4;
						}
					}
				updatePCB(pcb);
				enviarDinamico(PCB,socket,pcb);
				escribirEnArchivoLog("envio pcb ", &KernelLog,nombreLog);
				free(escribirArchivo->informacion);
				free(escribirArchivo);
				free(mensaje->mensaje);
				free(mensaje);
				free(pcb);
				
			break;
			case LEERARCHIVO: // CPU OBTIENE CONTENIDO DEL ARCHIVO
			escribirEnArchivoLog("en case leer archivo", &KernelLog,nombreLog);
				leerArchivo=malloc(sizeof(t_leerArchivo));
				recibirDinamico(LEERARCHIVO,socket,leerArchivo);
				escribirEnArchivoLog("recibo leer archivo", &KernelLog,nombreLog);
				while(0>recv(socket, seleccionador, sizeof(t_seleccionador), 0));
				pcb=malloc(sizeof(t_pcb));
				recibirDinamico(PCB,socket,pcb);
				escribirEnArchivoLog("recibo pcb", &KernelLog,nombreLog);
				paqueteFS=calloc(1,sizeof(t_paqueteFS));
				paqueteFS->tamanio=0;
				pcb->privilegiadasEjecutadas++;
				if (pcb->referenciaATabla[(leerArchivo->descriptor)-3].abierto==0)
				{
					error-14;
				}
				else{
					if (pcb->referenciaATabla[(leerArchivo->descriptor)-3].flags.lectura)
					{
						leerArchivoFS=malloc(sizeof(t_leerArchivoFS));
						leerArchivoFS->size=leerArchivo->tamanio;
						leerArchivoFS->offset=pcb->referenciaATabla[(leerArchivo->descriptor)-3].cursor;
						globalFd=pcb->referenciaATabla[(leerArchivo->descriptor)-3].globalFd;
						for (i = 0; i < CANTTABLAARCHIVOS; i++)
						{
							if (globalFd==tablaArchivos[i].fd)
							{
								break;
							}
						}
						leerArchivoFS->tamanioPath=strlen(tablaArchivos[i].path);
						leerArchivoFS->path=calloc(1,leerArchivoFS->tamanioPath+1);
						strcpy(leerArchivoFS->path,tablaArchivos[i].path);
						strip(&(leerArchivoFS->path));
						leerArchivoFS->tamanioPath=strlen(leerArchivoFS->path);
						enviarDinamico(OBTENERDATOSFS,SOCKETFS,leerArchivoFS);
						escribirEnArchivoLog("envio obtener datos fs", &KernelLog,nombreLog);
						while(0>recv(SOCKETFS,&rv,sizeof(int),0));
						if (!rv)
						{
							error=-2;
						}
						else{
							buffer=malloc(sizeof(int));
							*buffer=1;
							send(SOCKETFS,buffer,sizeof(int),0);
							while(0>recv(SOCKETFS, seleccionador, sizeof(t_seleccionador), 0));
							recibirDinamico(PAQUETEFS,SOCKETFS,paqueteFS);
							escribirEnArchivoLog("recibo paquete fs", &KernelLog,nombreLog);
						}
						free(leerArchivoFS->path);
						free(leerArchivoFS);
					}
					else{
						error=-3;
					}
				}
				updatePCB(pcb);
				// send(socket,paqueteFS->tamanio,sizeof(int),0);
				// while(0>recv(socket,buffer,sizeof(int),0));
				// send(socket,paqueteFS->paquete,paqueteFS->tamanio,0);
				enviarDinamico(PAQUETEFS,socket,paqueteFS);
				escribirEnArchivoLog("envio paquete fs", &KernelLog,nombreLog);
				free(paqueteFS->paquete);
				free(paqueteFS);
				free(buffer);
				enviarDinamico(PCB,socket,pcb);
				escribirEnArchivoLog("envio pcb", &KernelLog,nombreLog);
				free(leerArchivo);
				free(pcb);
				
			break;
			case PCBFINALIZADO: //FINALIZACION CORRECTA
						// RECIBO EL PCB
						escribirEnArchivoLog("en case pcb finalizado ", &KernelLog,nombreLog);
						pcb=malloc(sizeof(t_pcb));
						recibirDinamico(PCBFINALIZADO,socket,pcb);
						escribirEnArchivoLog("recibo pcb finalizado", &KernelLog,nombreLog);
						while(0>recv(socket,&rv,sizeof(int),0));
						if (rv)
						{
							PLANIFICACIONHABILITADA=0;
						}
						pcb->exitCode=0;
						pcb->rafagasEjecutadas++;
						updatePCB(pcb);
						pid=pcb->pid;
						// LO AGREGO A LA COLA EXIT
						pthread_mutex_lock(&mutexColaExit);
						CANTIDADEXITS++;
						COLAEXIT = realloc (COLAEXIT,(CANTIDADEXITS) * sizeof(COLAEXIT[0]));
						COLAEXIT[CANTIDADEXITS-1]=pid;
						pthread_mutex_unlock(&mutexColaExit);
						cambiarEstado(pid,EXIT);
						// BUSCO LA POSICION EN LA COLA DE EXEC
						int index=0;
						while(COLAEXEC[index]!=pid)
							index++;
						// LO SACO DE LA COLA DE EXEC
						for (index; index < CANTIDADEXECS; index++){
							pthread_mutex_lock(&mutexColaExec);
							if (index+1==CANTIDADEXECS)
								COLAEXEC[index]=-1;
							else
								COLAEXEC[index]=COLAEXEC[index+1];
							pthread_mutex_unlock(&mutexColaExec);
						}
						pthread_mutex_lock(&mutexColaExec);
						CANTIDADEXECS--;
						COLAEXEC=realloc(COLAEXEC,CANTIDADEXECS*sizeof(COLAEXEC[0]));
						pthread_mutex_unlock(&mutexColaExec);
						// LE MANDO EL FINALIZADO A CONSOLA
						aux=calloc(1,100);
						sprintf(aux,"Finalizo correctamente el proceso PID=%i",pid);
						mensaje=malloc(sizeof(t_mensaje));
						mensaje->tamanio=strlen(aux);
						mensaje->mensaje=aux;
						enviarDinamico(MENSAJE,SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],mensaje);
						send(SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],&(pcb->pid),sizeof(int),0);
						informarLeaks(pcb->pid);
						escribirEnArchivoLog("envio mensaje", &KernelLog,nombreLog);
						// free(mensaje->mensaje);
						enviarDinamico(LIBERARMEMORIA,SOCKETMEMORIA,NULL);
						escribirEnArchivoLog("envio liberar memomria ", &KernelLog,nombreLog);
						send(SOCKETMEMORIA,&pid,sizeof(int),0);
						free(mensaje);
						free(aux);
						free(pcb);
						primerAcceso=1;
				
			break;
			case PCBFINALIZADOPORCONSOLA:
				// RECIBO EL PCB
			printf("finalizo por consola pid: %i\n", pid);
						escribirEnArchivoLog("en case finalizado por consola", &KernelLog,nombreLog);
						pcb=malloc(sizeof(t_pcb));
						recibirDinamico(PCBFINALIZADOPORCONSOLA,socket,pcb);
						escribirEnArchivoLog("recibo pcb finalizado por consola", &KernelLog,nombreLog);
						pcb->rafagasEjecutadas++;
						exitCode=PCBS[pcb->pid].exitCode;
						updatePCB(pcb);
						pthread_mutex_lock(&mutexPcbs);
						PCBS[pcb->pid].exitCode=exitCode;
						pthread_mutex_unlock(&mutexPcbs);
						pthread_mutex_lock(&mutexColaExec);
						for (i = 0; i < CANTIDADEXECS; i++)
						{
							if (COLAEXEC[i]==pid)
							{
								break;
							}
						}
						for (; i < CANTIDADEXECS; i++)
						{
							if (i+1!=CANTIDADEXECS)
							{
								COLAEXEC[i]=COLAEXEC[i+1];
							}
						}
						CANTIDADEXECS--;
						COLAEXEC=realloc(COLAEXEC,CANTIDADEXECS*sizeof(int));
						pthread_mutex_unlock(&mutexColaExec);

						informarLeaks(pcb->pid);
						while(0>recv(socket,&rv,sizeof(int),0));
						if (rv)
						{
							PLANIFICACIONHABILITADA=0;
						}
						primerAcceso=1;
						seleccionador->tipoPaquete=PCB;
						free(pcb);
						WAITFINALIZAR=0;
				
			break;
			case PCBERROR:
				// RECIBO EL PCB
						escribirEnArchivoLog("en case pcb error", &KernelLog,nombreLog);
						pcb=malloc(sizeof(t_pcb));
						recibirDinamico(PCBERROR,socket,pcb);
						escribirEnArchivoLog("recibo pcb error", &KernelLog,nombreLog);
						pcb->rafagasEjecutadas++;
						updatePCB(pcb);
						finalizarPid(pcb->pid,error);
						while(0>recv(socket,&rv,sizeof(int),0));
						if (rv)
						{
							PLANIFICACIONHABILITADA=0;
						}
						primerAcceso=1;
						seleccionador->tipoPaquete=PCB;
						aux=calloc(1,100);
						sprintf(aux,"Proceso Pid=%i, finalizado por error=%i",pid,error);
						error=0;
						mensaje=malloc(sizeof(t_mensaje));
						mensaje->tamanio=strlen(aux);
						mensaje->mensaje=aux;
						enviarDinamico(MENSAJE,SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],mensaje);
						send(SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],&(pcb->pid),sizeof(int),0);
						informarLeaks(pcb->pid);
						escribirEnArchivoLog("envio mensaje", &KernelLog,nombreLog);
						free(mensaje);
						free(aux);
						liberarContenidoPcb(&pcb);
				
			break;
			case PAGINAINVALIDA:
						escribirEnArchivoLog("en case pagina invalida", &KernelLog,nombreLog);
						error=-15;
						
			break;
			case STACKOVERFLOW:
						escribirEnArchivoLog("en stack overflow", &KernelLog,nombreLog);
						error=-16;
			break;
			case PCBQUANTUM:
				escribirEnArchivoLog("en case pcb quantum", &KernelLog,nombreLog);
				pcb=malloc(sizeof(t_pcb));
				recibirDinamico(PCBQUANTUM,socket,pcb);
				escribirEnArchivoLog("recibo pcb quamtum", &KernelLog,nombreLog);
				cambiarEstado(pcb->pid,READY);
				pthread_mutex_lock(&mutexColaReady);
				CANTIDADREADYS++;
				COLAREADY=realloc(COLAREADY,CANTIDADREADYS*sizeof(int));
				COLAREADY[CANTIDADREADYS-1]=pcb->pid;
				pthread_mutex_unlock(&mutexColaReady);
				pthread_mutex_lock(&mutexColaExec);
				for (i = 0; i < CANTIDADEXECS; i++)
				{
					if (COLAEXEC[i]==pid)
					{
						break;
					}
				}
				for (; i < CANTIDADEXECS; i++)
				{
					if (i+1!=CANTIDADEXECS)
					{
						COLAEXEC[i]=COLAEXEC[i+1];
					}
				}
				CANTIDADEXECS--;
				COLAEXEC=realloc(COLAEXEC,CANTIDADEXECS*sizeof(int));
				pthread_mutex_unlock(&mutexColaExec);
				pcb->rafagasEjecutadas++;
				updatePCB(pcb);
				primerAcceso=1;
				seleccionador->tipoPaquete=PCB;
				while(0>recv(socket,&rv,sizeof(int),0));
				if (rv)
				{
					PLANIFICACIONHABILITADA=0;
				}
				free(pcb);
				
			break;
			case PCBBLOQUEADO:
			escribirEnArchivoLog("en pcb bloqueado", &KernelLog,nombreLog);
						pcb=malloc(sizeof(t_pcb));
				recibirDinamico(PCBBLOQUEADO,socket,pcb);
				escribirEnArchivoLog("recibo pcb bloqueado", &KernelLog,nombreLog);
				pcb->rafagasEjecutadas++;
				updatePCB(pcb);
				cambiarEstado(pcb->pid,BLOCKED);
				primerAcceso=1;
				seleccionador->tipoPaquete=PCB;
				while(0>recv(socket,&rv,sizeof(int),0));
				if (rv)
				{
					PLANIFICACIONHABILITADA=0;
				}
						free(pcb);
				
			break;
		}
	free(seleccionador);
	}
}

void comunicarse(dataParaComunicarse ** dataDeConexion){
	escribirEnArchivoLog("en comunicarse", &KernelLog,nombreLog);
	t_solicitudMemoria * respuestaSolicitud;
	t_path * path;
	int PidAFinalizar;
	t_metadata_program * metadata;
	t_resultadoIniciarPrograma * resultado;
	int socket=(*dataDeConexion)->socket;
	int interfaz=(*dataDeConexion)->interfaz;
	free(*dataDeConexion);
	flagLiberarComunicar=1;
	// RETURN VALUES
	int pid;
	int rv;
	int idConsola;
	int i;
	t_arrayPids * arrayPids;
	// NUMERO DE BYTES
	int nbytes;
	t_solicitudMemoria * solicitudMemoria;
	// FLAG DE ESTE HILO
	int estaComunicacion=1;
	// SELECCIONADOR
	t_seleccionador * seleccionador;
	// HILO PLANIFICADOR
	pthread_t hiloPlanificacion;
	// DATA PARA PLANIFICACION EN CASO DE NECESITARSE
    dataParaComunicarse ** dataDePlanificacion;
	// CICLO PRINCIPAL
	while(COMUNICACIONHABILITADA && estaComunicacion){
		// RECIBO EL SELECCIONADOR (SI ES CPU, DIRECTAMENTE LE MANDO PCBS)
		seleccionador=malloc(sizeof(t_seleccionador));
		if (interfaz==CPU){
			seleccionador->tipoPaquete=SOLICITUDPCB;
			rv=1;
		}
		else
			while(0>(rv=recv(socket, seleccionador, sizeof(t_seleccionador), 0)));
		if (rv==0)
		{
			printf("Cliente desconectado.\n");
			estaComunicacion=0;
		}
		else
		{
			switch(seleccionador->tipoPaquete)
			{
				case PATH:	// RECIBIMOS EL PATH DE UN PROGRAMA ANSISOP A EJECUTAR Y SU PID
					// RECIBO EL PATH
					escribirEnArchivoLog("en case path", &KernelLog,nombreLog);
					path = malloc (sizeof(t_path));
					recibirDinamico(PATH, socket, path);
					escribirEnArchivoLog("recibo path", &KernelLog,nombreLog);
					while(0>recv(socket,&idConsola,sizeof(int),0));
					// GENERO EL PID
					pid = ULTIMOPID;
					pthread_mutex_lock(&mutexPid);
					ULTIMOPID++;
					pthread_mutex_unlock(&mutexPid);
					// GUARDO SOCKET DE LA CONSOLA INDEXADO CON EL PID
					pthread_mutex_lock(&mutexSocketsConsola);
					CANTIDADSOCKETSCONSOLA++;
					SOCKETSCONSOLA = realloc (SOCKETSCONSOLA,(CANTIDADSOCKETSCONSOLA) * sizeof(SOCKETSCONSOLA[0]));
					SOCKETSCONSOLA[CANTIDADSOCKETSCONSOLA-1]=idConsola;
					pthread_mutex_unlock(&mutexSocketsConsola);
					// CALCULO LA CANTIDAD DE PAGINAS
					int cantPaginasCodigo = calcularPaginas(TAMPAGINA,path->tamanio);
					// GENERO LA METADATA DEL SCRIPT
					metadata=metadata_desde_literal(path->path);
					// CREO EL PCB
					t_pcb * pcb;
					pcb=malloc(sizeof(t_pcb));
					pcb->pid = pid;
					pcb->estado = NEW;
					pcb->rafagasEjecutadas=0;
					pcb->privilegiadasEjecutadas=0;
					pcb->paginasHeap=0;
					pcb->alocaciones=0;
					pcb->bytesAlocados=0;
					pcb->liberaciones=0;
					pcb->bytesLiberados=0;
					pcb->cantidadArchivos=0;
					pcb->programCounter=0;
					pcb->paginasCodigo=cantPaginasCodigo;
					pcb->posicionStack=0;
					pcb->cantidadInstrucciones=metadata->instrucciones_size;
					pcb->indiceCodigo=malloc(metadata->instrucciones_size*sizeof(t_intructions));
					memcpy(pcb->indiceCodigo,metadata->instrucciones_serializado,metadata->instrucciones_size*sizeof(t_intructions));
					pcb->indiceEtiquetas.etiquetas_size=-1;
					pcb->indiceEtiquetas.etiquetas_size=metadata->etiquetas_size;
					pcb->indiceEtiquetas.etiquetas=calloc(1,metadata->etiquetas_size);
					memcpy(pcb->indiceEtiquetas.etiquetas,metadata->etiquetas,metadata->etiquetas_size);
					
					pthread_mutex_lock(&mutexAlocar);
					CANTTABLAHEAP++;
					tablaHeap=realloc(tablaHeap,CANTTABLAHEAP*sizeof(t_heapGlobal));
					tablaHeap[CANTTABLAHEAP-1].pid=pcb->pid;
					tablaHeap[CANTTABLAHEAP-1].cantPaginas=0;
					tablaHeap[CANTTABLAHEAP-1].paginas=malloc(tablaHeap[CANTTABLAHEAP-1].cantPaginas*sizeof(t_heapProceso));
					pthread_mutex_unlock(&mutexAlocar);
					pcb->cantidadStack=1;
					pcb->indiceStack=malloc(sizeof(t_stack));
					pcb->indiceStack[0].cantidadVariables=0;
					pcb->indiceStack[0].cantidadArgumentos=0;
					pcb->exitCode=1;
					// LO AGREGO A LA TABLA
					metadata_destruir(metadata);
					pthread_mutex_lock(&mutexPcbs);
					CANTIDADPCBS++;
					PCBS = realloc (PCBS,(CANTIDADPCBS) * sizeof(PCBS[0]));
					memcpy(&(PCBS[CANTIDADPCBS-1]),pcb,sizeof(t_pcb));
					pthread_mutex_unlock(&mutexPcbs);
					// SOLICITUD DE MEMORIA
					solicitudMemoria=calloc(1,sizeof(t_solicitudMemoria));
					solicitudMemoria->tamanioCodigo=path->tamanio;
					solicitudMemoria->codigo=calloc(1,solicitudMemoria->tamanioCodigo+1);
					strcpy(solicitudMemoria->codigo,path->path);
					solicitudMemoria->cantidadPaginasCodigo=cantPaginasCodigo;
					solicitudMemoria->cantidadPaginasStack=STACK_SIZE;
					solicitudMemoria->pid=pid;
					enviarDinamico(SOLICITUDMEMORIA,SOCKETMEMORIA,solicitudMemoria);
					escribirEnArchivoLog("envio solicitud memoria", &KernelLog,nombreLog);
					//LIBERO MEMORIA
					free(path->path);
					free(path);
					free(solicitudMemoria->codigo);
					free(solicitudMemoria);
					free(pcb);
					
					// RECIBO LA RESPUESTA DE MEMORIA
					respuestaSolicitud=malloc(sizeof(t_solicitudMemoria));
					while(0>recv(SOCKETMEMORIA, &(respuestaSolicitud->respuesta), sizeof(int), 0));
					// PREPARO LA RESPUESTA A CONSOLA
					resultado=malloc(sizeof(t_resultadoIniciarPrograma));
					resultado->pid=pid;
					// VERIFICO SI PUEDO PASAR A NEW
					if (respuestaSolicitud->respuesta==OK){
						// AGREGO EL PID A LA COLA DE NEWS
						pthread_mutex_lock(&mutexColaNew);
						CANTIDADNEWS++;
						COLANEW = realloc (COLANEW,(CANTIDADNEWS) * sizeof(COLANEW[0]));
						COLANEW[CANTIDADNEWS-1]=pid;
						pthread_mutex_unlock(&mutexColaNew);
						resultado->resultado=1;
					}
					else if (respuestaSolicitud->respuesta==FAIL){
						resultado->resultado=0;
					 	// AGREGO EL PID A LA COLA DE EXIT
						pthread_mutex_lock(&mutexColaExit);
						CANTIDADEXITS++;
						COLAEXIT = realloc (COLAEXIT,(CANTIDADEXITS) * sizeof(int));
						COLAEXIT[CANTIDADEXITS-1]=pid;
						pthread_mutex_unlock(&mutexColaExit);
						pthread_mutex_lock(&mutexPcbs);
						PCBS[pid].exitCode=-1;
						pthread_mutex_unlock(&mutexPcbs);
						cambiarEstado(pid,EXIT);
						resultado->resultado=1;
					}
					rellenarColaReady();
					// ENVIO RESPUESTA A CONSOLA
					enviarDinamico(RESULTADOINICIARPROGRAMA,SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],resultado);
					escribirEnArchivoLog("envio resultado iniciar programa", &KernelLog,nombreLog);
					// LIBERO MEMORIA
					free(respuestaSolicitud);
					free(resultado);
				break;
				case FINALIZARPROGRAMA: // RECIBIMOS EL PID DE UN PROGRAMA ANSISOP A FINALIZAR
					escribirEnArchivoLog("en case finalizar programa", &KernelLog,nombreLog);
					recibirDinamico(FINALIZARPROGRAMA,socket,&PidAFinalizar);
					escribirEnArchivoLog("recibo finalizar programa", &KernelLog,nombreLog);
					finalizarPidConsola(PidAFinalizar,-7);

				break;
				case ARRAYPIDS:
					escribirEnArchivoLog("en case arrays pids", &KernelLog,nombreLog); 
					arrayPids=malloc(sizeof(t_arrayPids));
					recibirDinamico(ARRAYPIDS,socket,arrayPids);
					escribirEnArchivoLog("recibo arrays pids", &KernelLog,nombreLog);
					for (i = 0; i < arrayPids->cantidad; i++)
					{
						printf("pid: %i\n",arrayPids->pids[i]);
						finalizarPidConsola(arrayPids->pids[i],-6);
					}
					free(arrayPids);
				break;
				case SOLICITUDPCB:
					escribirEnArchivoLog("en case solicitud pcb", &KernelLog,nombreLog); 
					// INICIO DE HILO PLANIFICADOR
					dataDePlanificacion=malloc(sizeof(dataParaComunicarse*));
					*dataDePlanificacion=malloc(sizeof(dataParaComunicarse));
	        		(*dataDePlanificacion)->socket=socket;
	        		pthread_create(&hiloPlanificacion,NULL,(void *)planificar,dataDePlanificacion);
	        		while(!flagLiberarPlanificar);
	        		flagLiberarPlanificar=0;
	        		free(dataDePlanificacion);
					estaComunicacion=0;
	        	break;
	        	case SOLICITUDMEMORIA:
	        	
	        	break;
			}
		}
	free(seleccionador);
	}
}

void aceptar(dataParaComunicarse ** dataParaAceptar){
	// VARIABLES PARA LAS CONEXIONES ENTRANTES
	escribirEnArchivoLog("en aceptar ", &KernelLog,nombreLog); 
	int socketAceptar=(*dataParaAceptar)->socket;
	free(*dataParaAceptar);
	flagLiberarAceptar=1;
	int * interfaz;
	int flag=0;
	pthread_t hiloComunicador;
	dataParaComunicarse ** dataParaConexion;
	int socketNuevaConexion;
	// RETURN VALUES
	int rv;
	// CONFIGURACION PARA ACCEPT
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	// INICIO LA ESCUCHA
	if(rv=listen(socketAceptar,BACKLOG)==-1)
		perror("Error en el listen");
	// CICLO PRINCIPAL
	while(ACEPTACIONHABILITADA){
		if ((socketNuevaConexion = accept(socketAceptar, (struct sockaddr *)&addr,&addrlen)) == -1)
			perror("Error con conexion entrante");
		else if (socketNuevaConexion >= 0){
			interfaz = malloc(sizeof(int));
			// ME INFORMO SOBRE LA INTERFAZ QUE SE CONECTÓ
			handshakeServer(socketNuevaConexion,KERNEL,interfaz);
			if (*interfaz==CPU && primerIngresoCpu)
			{}
			if (*interfaz==CONSOLA)
			{
				while(0>recv(socketNuevaConexion,&rv,sizeof(int),0));
				if (rv)
				{
					send(socketNuevaConexion,&CANTSOCKETSMENSAJE,sizeof(int),0);
					CANTSOCKETSMENSAJE++;
					SOCKETSCONSOLAMENSAJE=realloc(SOCKETSCONSOLAMENSAJE,sizeof(int)*CANTSOCKETSMENSAJE);
					SOCKETSCONSOLAMENSAJE[CANTSOCKETSMENSAJE-1]=socketNuevaConexion;
					flag=1;
				}
			}
			// CONFIGURACION E INICIO DE HILO 
			if(!flag) {
						dataParaConexion=malloc(sizeof(dataParaComunicarse*));
						*dataParaConexion = malloc(sizeof(dataParaComunicarse));
						(*dataParaConexion)->interfaz=*interfaz; 
						(*dataParaConexion)->socket=socketNuevaConexion;
						pthread_create(&hiloComunicador,NULL,(void *)comunicarse,dataParaConexion);
						pthread_detach(hiloComunicador);
						while(!flagLiberarComunicar);
						flagLiberarComunicar=0;
						free(dataParaConexion);
						}
			else flag=0;
						free(interfaz);
		}
	}
}

void chequeoQuantum(){
	t_config *CFG;
	while(1){
		CFG = config_create("kernelCFG.txt");
		QUANTUM_SLEEP= config_get_int_value(CFG ,"QUANTUM_SLEEP");
		config_destroy(CFG);
	}
}

int countElementos(char* cadena){
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
/* LEER CONFIGURACION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
signal(SIGINT,cortar);
t_config *CFG;
int i;
CFG = config_create("kernelCFG.txt");
PUERTO_PROG= config_get_string_value(CFG ,"PUERTO_PROG");
IP_MEMORIA= config_get_string_value(CFG ,"IP_MEMORIA");
PUERTO_MEMORIA= config_get_string_value(CFG ,"PUERTO_MEMORIA");
IP_FS= config_get_string_value(CFG ,"IP_FS");
PUERTO_FS= config_get_string_value(CFG ,"PUERTO_FS");
QUANTUM= config_get_int_value(CFG ,"QUANTUM");
QUANTUM_SLEEP= config_get_int_value(CFG ,"QUANTUM_SLEEP");
ALGORITMO= config_get_string_value(CFG ,"ALGORITMO");
GRADO_MULTIPROG= config_get_int_value(CFG ,"GRADO_MULTIPROG");
CANTIDADSEM=countElementos(config_get_string_value(CFG, "SEM_IDS"));
SEM_IDS= config_get_array_value(CFG ,"SEM_IDS");
SEM_INIT= config_get_array_value(CFG ,"SEM_INIT");
CANTIDADVARS=countElementos(config_get_string_value(CFG, "SHARED_VARS"));
SHARED_VARS= config_get_array_value(CFG ,"SHARED_VARS");
SHARED_VALUES=calloc(CANTIDADVARS,sizeof(int));
STACK_SIZE= config_get_int_value(CFG ,"STACK_SIZE");
BLOQUEADOSPORSEM=calloc(CANTIDADSEM,sizeof(int*));
CANTIDADBLOCKPORSEM=calloc(CANTIDADSEM,sizeof(int));
for (i = 0; i < CANTIDADSEM; i++)
{
	CANTIDADBLOCKPORSEM[i]=0;
}
system("clear");
// printf("Configuración:\nPUERTO_PROG = %s,\nIP_MEMORIA = %s,\nPUERTO_MEMORIA = %s,\nIP_FS = %s,\nPUERTO_FS = %s,\nQUANTUM = %i,\nQUANTUM_SLEEP = %i,\nALGORITMO = %s,\nGRADO_MULTIPROG = %i,\nSEM_IDS = %s,\nSEM_INIT = %s,\nSHARED_VARS = %s,\nSTACK_SIZE = %i.\n"
		// ,PUERTO_PROG,IP_MEMORIA,PUERTO_MEMORIA,IP_FS,PUERTO_FS,QUANTUM,QUANTUM_SLEEP,ALGORITMO,GRADO_MULTIPROG,SEM_IDS,SEM_INIT,SHARED_VARS,STACK_SIZE);
system("clear");
/* LEER CONFIGURACION

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/

	char * nombreLog=calloc(1,200);
	strcpy(nombreLog,"logKernel-");
	horaActual=calloc(1,200);
	horaYFechaActual(&horaActual);
	printf("%s\n", horaActual);
	stripLog(&horaActual);
	printf("horaActual: %s\n", horaActual);
	strcat(horaActual,".txt");
	strcat(nombreLog,horaActual);
	printf("nombreLog: %s\n", nombreLog);
pthread_t hiloChequeoQuantum;
pthread_create(&hiloChequeoQuantum,NULL,(void *)chequeoQuantum,NULL);
// INICIO SEMAFOROS
pthread_mutex_init(&mutexColaNew,NULL);
pthread_mutex_init(&mutexColaReady,NULL);
pthread_mutex_init(&mutexColaExit,NULL);
pthread_mutex_init(&mutexPid,NULL);
pthread_mutex_init(&mutexPcbs,NULL);
pthread_mutex_init(&mutexSocketsConsola,NULL);
pthread_mutex_init(&mutexGradoMultiprog,NULL);
pthread_mutex_init(&mutexColaExec,NULL);
pthread_mutex_init(&mutexColaBlock,NULL);
pthread_mutex_init(&mutexPausaPlanificacion,NULL);
pthread_mutex_init(&mutexProcesosReady,NULL);
pthread_mutex_init(&mutexTablaArchivos,NULL);
pthread_mutex_init(&mutexCompartidas,NULL);
pthread_mutex_init(&mutexSemaforos,NULL);
pthread_mutex_init(&mutexAlocar,NULL);
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
int socketMemoria=socket(memoria->ai_family,memoria->ai_socktype,memoria->ai_protocol);

void ** buffer=malloc(sizeof(int*));
(*buffer)=malloc(sizeof(int));
if ((rv = connect(socketMemoria,memoria->ai_addr,memoria->ai_addrlen)) == -1) 
	perror("No se pudo conectar con memoria.\n");
else if (rv == 0){
	printf("Se conectó con memoria correctamente.\n");
	handshakeCliente(socketMemoria,KERNEL,buffer);
	free(*buffer);
	free(buffer);
}
SOCKETMEMORIA=socketMemoria;
// RECIBO EL TAMAÑO DE PAGINA
int nbytes;
int tamPagina;
// while( 0>recv(SOCKETMEMORIA, tamPagina, sizeof(int), 0));
recv(SOCKETMEMORIA, &tamPagina, sizeof(int), 0);
printf("MARCO SIZE: %i\n", tamPagina);
TAMPAGINA=tamPagina;
freeaddrinfo(memoria);
// CONEXION CON FILESYSTEM (NO ES NECESARIO HACER HANDSHAKE, KERNEL ES EL ÚNICO QUE SE CONECTA A FS)
struct addrinfo *fs;
getaddrinfo(IP_FS,PUERTO_FS,&hints,&fs);
int socketFS=socket(fs->ai_family, fs->ai_socktype, fs->ai_protocol);
buffer=malloc(sizeof(int*));
(*buffer)=malloc(sizeof(int));
if ((rv = connect(socketFS,fs->ai_addr,fs->ai_addrlen)) == -1) 
	perror("No se pudo conectar con filesystem.\n");
else if (rv == 0){
	printf("Se conectó con filesystem correctamente.\n");
	handshakeCliente(socketFS,KERNEL,buffer);
}
KernelLog=fopen(nombreLog,"w+");
signal(SIGINT,cortar);
SOCKETFS=socketFS;
freeaddrinfo(fs);
// CONFIGURACION DEL SERVIDOR
tablaArchivos=malloc(1);
struct addrinfo *serverInfo;
if ((rv =getaddrinfo(NULL, PUERTO_PROG, &hints, &serverInfo)) != 0)
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
int socketEscuchador;
socketEscuchador = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
if(bind(socketEscuchador,serverInfo->ai_addr, serverInfo->ai_addrlen)==-1) 
	{perror("Error en el bind.\n");
		return 1;}
freeaddrinfo(serverInfo);
// CONFIGURACION DE HILO ACEPTADOR
dataParaComunicarse **dataParaAceptar;
dataParaAceptar=malloc(sizeof(dataParaComunicarse*));
(*dataParaAceptar)=malloc(sizeof(dataParaComunicarse));
(*dataParaAceptar)->socket = socketEscuchador;
// INICIO DE HILO ACEPTADOR
pthread_t hiloAceptador;
pthread_create(&hiloAceptador,NULL,(void *)aceptar,dataParaAceptar);
while(!flagLiberarAceptar);
free(dataParaAceptar);
// INICIO DE HILO DE CONSOLA
pthread_t hiloConsola;
pthread_create(&hiloConsola,NULL,(void *)consola,NULL);
// ESPERO FIN DE HILO CONSOLA
pthread_join(hiloConsola,NULL);
pthread_join(hiloAceptador,NULL);
//LIBERO MEMORIA
free(COLAREADY);
free(COLANEW);
free(COLAEXEC);
free(COLABLOCK);
free(COLAEXIT);
free(SOCKETSCONSOLA);
return 0;
}