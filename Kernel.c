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
#include <semaphore.h>
#define BACKLOG 5
#define KERNEL 0
#define MEMORIA 1
#define CONSOLA 2
#define CPU 3
#define FS 4
#define RESPUESTAOKMEMORIA 1
#define INICIARPROGRAMA 2
#define LIBERARMEMORIA 3
#define DESCONECTARCONSOLA 4
#define SOLICITUDPCB 6
#define MENSAJE 7
#define ESCRIBIR 8
#define PIDFINALIZOPROCESO 9
#define PATH 10
#define PCB 17
#define RESULTADOINICIARPROGRAMA 23
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
#define PCBFINALIZADOPORCONSOLA 56
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
#define SOLICITUDMEMORIA 70
#define LIBERARPAGINA 71
#define PRINT 72
#define LEAKS 73
#define FINALIZARPROGRAMA 88
#define FALSE 0
#define TRUE 1
#define FAIL 0
#define OK 1
#define NEW 0
#define READY 1
#define EXEC 2
#define BLOCKED 3
#define EXIT 4
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
int * SOCKETPID;
int CANTSOCKETSPID=0;
int CANTIDADVARS=0;
int CANTIDADSEM=0;
int flagLiberarAceptar=0;
int flagLiberarData=0;
int AFINALIZAR=0;
int PIDFIN=-1;
int EXITCODE;
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
pthread_mutex_t mutexFinConsola;
pthread_mutex_t mutexFinPid;
pthread_mutex_t mutexSocketPid;
pthread_mutex_t mutexDespuesFin;
pthread_mutex_t mutexSolicitudMem;
sem_t semReadys;

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
int CONSOLAHABILITADA=1;



void add (int valor, int ** cola, int * tamanio, pthread_mutex_t * mutex)
{
	pthread_mutex_lock(mutex);
	(*tamanio)++;
	if ((*tamanio)==1)
	{
		(*cola)=calloc(1,sizeof(int));
	}
	else
	{
		(*cola)=realloc((*cola),(*tamanio)*sizeof(int));
	}
	(*cola)[(*tamanio)-1]=valor;
	pthread_mutex_unlock(mutex);
}
void oust(int valor, int ** cola, int * tamanio, pthread_mutex_t * mutex)
{
	int i;
	pthread_mutex_lock(mutex);
	for (i = 0; i < (*tamanio); i++)
	{
		if ((*cola)[i]==valor)
		{
			break;
		}
	}
	if (i!=(*tamanio))
	{
		for (; i < (*tamanio); i++)
		{
			if (i!=(*tamanio)-1)
			{
				(*cola)[i]=(*cola)[i+1];
			}
		}
		(*tamanio)--;
		if ((*tamanio)==0)
		{
			free((*cola));
		}
		else
		{
			(*cola)=realloc((*cola),(*tamanio)*sizeof(int));
		}
	}
	pthread_mutex_unlock(mutex);
}
int head(int ** cola, int * tamanio, pthread_mutex_t * mutex)
{
	int head,i;
	if ((*tamanio)>0)
	{
		head=(*cola)[0];
		oust(head,&(*cola),&(*tamanio),mutex);
		return head;
	}
	return -1;
}
int lugaresColaReady()
{
	if (GRADO_MULTIPROG-CANTIDADREADYS>0)
	{
		return GRADO_MULTIPROG-CANTIDADREADYS;
	}
	return 0;
}
void cambiarEstado(int pid, int estado)
{
	pthread_mutex_lock(&mutexPcbs);
	PCBS[pid].estado=estado;
	pthread_mutex_unlock(&mutexPcbs);
}
void rellenarColaReady()
{
	int espacios,i,pid;
	if (espacios=lugaresColaReady())
	{
		for (i = 0; i < espacios; ++i)
		{
			if (CANTIDADNEWS>0)
			{
				pid=head(&COLANEW,&CANTIDADNEWS,&mutexColaNew);
				add(pid,&COLAREADY,&CANTIDADREADYS,&mutexColaReady);
				cambiarEstado(pid,READY);
				sem_post(&semReadys);
			}
		}
	}
}
void setExitCode(int pid, int exitCode)
{
	pthread_mutex_lock(&mutexPcbs);
	PCBS[pid].exitCode=exitCode;
	pthread_mutex_unlock(&mutexPcbs);
}
int nextPid()
{
	pthread_mutex_lock(&mutexPid);
	ULTIMOPID++;
	pthread_mutex_unlock(&mutexPid);
	return ULTIMOPID-1;
}
void strip(char** string)
{
	int i;
	for(i=0;i<string_length(*string); i++){
		if((*string)[i]=='\n' || (*string)[i]=='\r' )
			(*string)[i]='\0';
	}
}
int valorCompartida(char ** variable)
{
	int i;
	strip(variable);
	for (i = 0; i < CANTIDADVARS; i++)
	{
		if (!strcmp(SHARED_VARS[i]+1,*variable))
		{
			return SHARED_VALUES[i];
		}
	}
}
int existeCompartida(char ** variable)
{
	int i;
	strip(variable);
	for (i = 0; i < CANTIDADVARS; i++)
	{
		if (!strcmp(SHARED_VARS[i]+1,*variable))
		{
			return 1;
		}
	}
	return 0;
}
void asignarCompartida(char ** variable,int valor)
{
	int i;
	strip(variable);
	for (i = 0; i < CANTIDADVARS; i++)
	{
		if (!strcmp(SHARED_VARS[i]+1,*variable))
		{
			pthread_mutex_lock(&mutexCompartidas);
			SHARED_VALUES[i]=valor;
			pthread_mutex_unlock(&mutexCompartidas);
		}
	}
}
int existeSemaforo(char ** semaforo)
{
	int i;
	for (i = 0; i < CANTIDADSEM; i++)
	{
		strip(semaforo);
		if (!strcmp(SEM_IDS[i],*semaforo))
		{
			return 1;
		}
	}
	return 0;
}
void wait(char ** semaforo, int pid)
{
	int valorInt,i;
	char * valorString;
	for (i = 0; i < CANTIDADSEM; i++)
	{
		strip(semaforo);
		if (!strcmp(SEM_IDS[i],*semaforo))
		{
			if (atoi(SEM_INIT[i])>0)
			{
				valorInt=atoi(SEM_INIT[i]);
				valorInt--;
				valorString=calloc(1,10);
				sprintf(valorString, "%i", valorInt);
				strcpy(SEM_INIT[i],valorString);
				free(valorString);
			}
			else
			{
				oust(pid,&COLAEXEC,&CANTIDADEXECS,&mutexColaExec);
				add(pid,&COLABLOCK,&CANTIDADBLOCKS,&mutexColaBlock);
				add(pid,&BLOQUEADOSPORSEM[i],&(CANTIDADBLOCKPORSEM[i]),&mutexSemaforos);
			}
		}
	}
}
void mandarAExit(int pid){
	int i;
	oust(pid,&COLANEW,&CANTIDADNEWS,&mutexColaNew);
	oust(pid,&COLAREADY,&CANTIDADREADYS,&mutexColaReady);
	oust(pid,&COLAEXEC,&CANTIDADEXECS,&mutexColaExec);
	oust(pid,&COLABLOCK,&CANTIDADBLOCKS,&mutexColaBlock);
	oust(pid,&COLAEXIT,&CANTIDADEXITS,&mutexColaExit);
	add(pid,&COLAEXIT,&CANTIDADEXITS,&mutexColaExit);
	if (PCBS[pid].estado==BLOCKED)
	{
		for (i = 0; i < CANTIDADSEM; i++)
		{
			oust(pid,&(BLOQUEADOSPORSEM[i]),&(CANTIDADBLOCKPORSEM[i]),&mutexSemaforos);
		}
	}
}
void warn(char ** semaforo)
{
	strip(semaforo);
	int pid,valorInt,i,j;
	char * valorString;
	for (i = 0; i < CANTIDADSEM; i++)
	{
		if (!strcmp(SEM_IDS[i],*semaforo))
		{
			for (j= 0; j < CANTIDADBLOCKPORSEM[i]; ++j)
			{
				if(PCBS[(BLOQUEADOSPORSEM[i])[0]].estado!=EXIT)
				{
					pid=head(&(BLOQUEADOSPORSEM[i]),&(CANTIDADBLOCKPORSEM[i]),&mutexSemaforos);
					oust(pid,&COLABLOCK,&CANTIDADBLOCKS,&mutexColaBlock);
					add(pid,&COLAREADY,&CANTIDADREADYS,&mutexColaReady);
					cambiarEstado(pid,READY);
					sem_post(&semReadys);
					return;
				}
				else
				{
					pid=head(&(BLOQUEADOSPORSEM[i]),&(CANTIDADBLOCKPORSEM[i]),&mutexSemaforos);
					mandarAExit(pid);
				}
			}
			valorInt=atoi(SEM_INIT[i]);
			valorInt++;
			valorString=calloc(1,10);
			sprintf(valorString, "%i", valorInt);
			strcpy(SEM_INIT[i],valorString);
			free(valorString);
		}
	}
}
int alcanzaHeap(int espacio, int pid)
{
	int j;
	for (j = 0; j < tablaHeap[pid].cantPaginas; j++)
	{
		printf("libre: %i.\n", tablaHeap[pid].paginas[j].espacioLibre);
		printf("espacio a reservar: %i.\n", espacio);
		if (tablaHeap[pid].paginas[j].espacioLibre>=espacio+sizeof(t_heapMetaData))
		{
			return j;
		}
	}
	return -1;
}
int alocar(int espacio, int pid, int pagina)
{
	int k,l,espacioAnterior,puntero,number;	
	pthread_mutex_lock(&mutexAlocar);
	printf("metadatas: %i.\n",tablaHeap[pid].paginas[pagina].cantidadMetadatas );					
	number=tablaHeap[pid].paginas[pagina].cantidadMetadatas;
	for (k = 0; k<number; k++)
	{	
		if (tablaHeap[pid].paginas[pagina].contenido[k].isFree)
		{
			printf("encontre libre\n");
			if (k==tablaHeap[pid].paginas[pagina].cantidadMetadatas-1)
			{
				printf("es el ultimo metadata\n");
				printf("size: %i.\n", tablaHeap[pid].paginas[pagina].contenido[k].size-sizeof(t_heapMetaData));
				printf("espacio a reservar: %i.\n",espacio );
				if ((espacio<=(tablaHeap[pid].paginas[pagina].contenido[k].size-sizeof(t_heapMetaData))))
				{
					tablaHeap[pid].paginas[pagina].contenido[k].isFree=false;
					espacioAnterior=tablaHeap[pid].paginas[pagina].contenido[k].size;
					tablaHeap[pid].paginas[pagina].contenido[k].size=espacio;
					tablaHeap[pid].paginas[pagina].cantidadMetadatas++;
					tablaHeap[pid].paginas[pagina].contenido=realloc(tablaHeap[pid].paginas[pagina].contenido,tablaHeap[pid].paginas[pagina].cantidadMetadatas*sizeof(t_heapMetaData));
					tablaHeap[pid].paginas[pagina].contenido[k+1].isFree=true;
					tablaHeap[pid].paginas[pagina].contenido[k+1].size=espacioAnterior-espacio-sizeof(t_heapMetaData);
					printf("espacioLibre anteasdasfe: %i.\n", tablaHeap[pid].paginas[pagina].espacioLibre);
					tablaHeap[pid].paginas[pagina].espacioLibre-=(sizeof(t_heapMetaData)+espacio);
					printf("espacioLibre anteasdasfe: %i.\n", tablaHeap[pid].paginas[pagina].espacioLibre);
					break;

				}
			}
			else
			{
				if (espacio==(tablaHeap[pid].paginas[pagina].contenido[k].size))
				{	
					tablaHeap[pid].paginas[pagina].contenido[k].isFree=false;
					tablaHeap[pid].paginas[pagina].espacioLibre-=espacio;
					break;
				}
				else
				{
					printf("no es el ultimo y sobra espacio\n");
					tablaHeap[pid].paginas[pagina].contenido[k].isFree=false;
					espacioAnterior=tablaHeap[pid].paginas[pagina].contenido[k].size;
					tablaHeap[pid].paginas[pagina].contenido[k].size=espacio;
					tablaHeap[pid].paginas[pagina].cantidadMetadatas++;
					tablaHeap[pid].paginas[pagina].contenido=realloc(tablaHeap[pid].paginas[pagina].contenido,tablaHeap[pid].paginas[pagina].cantidadMetadatas*sizeof(t_heapMetaData));
					tablaHeap[pid].paginas[pagina].contenido[k+1].isFree=true;
					tablaHeap[pid].paginas[pagina].contenido[k+1].size=espacioAnterior-espacio-sizeof(t_heapMetaData);
					tablaHeap[pid].paginas[pagina].espacioLibre-=(sizeof(t_heapMetaData)+espacio);
					break;
				}
			}
		}
		printf("no esta libre\n");
	}
	pthread_mutex_unlock(&mutexAlocar);
	// k--;
	puntero=(tablaHeap[pid].paginas[pagina].pagina-PCBS[pid].paginasCodigo)*TAMPAGINA;
	printf("puntero: %i\n", puntero);
	for (l = 0; l < k; l++)
	{
		printf("puntero: %i\n", puntero);
		puntero+=sizeof(t_heapMetaData)+tablaHeap[pid].paginas[pagina].contenido[l].size;
		printf("puntero: %i\n", puntero);
	}
	printf("puntero: %i\n", puntero);
	puntero+=sizeof(t_heapMetaData);
	printf("puntero: %i\n", puntero);
	printf("espacio libre en alocar: %i.\n", tablaHeap[pid].paginas[pagina].espacioLibre);
	return puntero;
}
int nuevaPaginaHeap(int pid)
{
	int rv;
	t_solicitudAsignar * solicitudPaginaHeap;
	solicitudPaginaHeap=malloc(sizeof(t_solicitudAsignar));
	solicitudPaginaHeap->pid=pid;
	solicitudPaginaHeap->paginasAAsignar=1;
	pthread_mutex_lock(&mutexSolicitudMem);
	enviarDinamico(ASIGNARPAGINAS,SOCKETMEMORIA,solicitudPaginaHeap);
	free(solicitudPaginaHeap);
	while(0>recv(SOCKETMEMORIA,&rv,sizeof(int),0))
	{
		perror("error en nuevaPaginaHeap:");
	}
	pthread_mutex_unlock(&mutexSolicitudMem);
	if (rv==-1)
	{
		return -1;
	}
	else
	{
		pthread_mutex_lock(&mutexAlocar);
		tablaHeap[pid].cantPaginas++;
		tablaHeap[pid].paginas=realloc(tablaHeap[pid].paginas,tablaHeap[pid].cantPaginas*sizeof(t_heapProceso));
		tablaHeap[pid].paginas[tablaHeap[pid].cantPaginas-1].pagina=PCBS[pid].paginasCodigo+STACK_SIZE+PCBS[pid].paginasHeap;
		tablaHeap[pid].paginas[tablaHeap[pid].cantPaginas-1].espacioLibre=TAMPAGINA-sizeof(t_heapMetaData);
		tablaHeap[pid].paginas[tablaHeap[pid].cantPaginas-1].cantidadMetadatas=1;
		tablaHeap[pid].paginas[tablaHeap[pid].cantPaginas-1].contenido=malloc(sizeof(t_heapMetaData));
		tablaHeap[pid].paginas[tablaHeap[pid].cantPaginas-1].contenido[0].isFree=true;
		tablaHeap[pid].paginas[tablaHeap[pid].cantPaginas-1].contenido[0].size=TAMPAGINA-sizeof(t_heapMetaData);
		pthread_mutex_unlock(&mutexAlocar);
		return tablaHeap[pid].cantPaginas-1;
	}
}
void sacarHeapMetadata(indexProceso,pagina,posicion)
{
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
int liberarHeap(int pid, int pagina, int offset)
{
	t_liberarPagina * liberarPagina;
	int j,k,rv;
	pthread_mutex_lock(&mutexAlocar);
	for (j = 0; j < tablaHeap[pid].cantPaginas; j++)
	{
		printf("pagina tabla: %i.\n",tablaHeap[pid].paginas[j].pagina);
		printf("pagina busca3: %i.\n", pagina);
		if (tablaHeap[pid].paginas[j].pagina==pagina)
		{
			printf("encontre la pagina\n");
			for (k = 0; k < tablaHeap[pid].paginas[j].cantidadMetadatas; k++)
			{
				offset-=sizeof(t_heapMetaData);
				if (offset==0)
				{
					printf("encontre la metadata\n");
					tablaHeap[pid].paginas[j].espacioLibre+=tablaHeap[pid].paginas[j].contenido[k].size;
					tablaHeap[pid].paginas[j].contenido[k].isFree=true;
					if (k!=tablaHeap[pid].paginas[j].cantidadMetadatas-1 && tablaHeap[pid].paginas[j].contenido[k+1].isFree)
					{
						printf("el siguiente esta libre\n");
						if (k+1==tablaHeap[pid].paginas[j].cantidadMetadatas-1)
						{
							if (tablaHeap[pid].paginas[j].contenido[k+1].size==0)
							{
								tablaHeap[pid].paginas[j].contenido[k].size+=sizeof(t_heapMetaData);
							}
							else
							{
								tablaHeap[pid].paginas[j].contenido[k].size+=tablaHeap[pid].paginas[j].contenido[k+1].size+sizeof(t_heapMetaData);
							}
						}
						else
						{
							tablaHeap[pid].paginas[j].contenido[k].size+=tablaHeap[pid].paginas[j].contenido[k+1].size+sizeof(t_heapMetaData);
						}
						sacarHeapMetadata(pid,j,k+1);
						tablaHeap[pid].paginas[j].espacioLibre+=sizeof(t_heapMetaData);
					}
					if ( k!=0 && tablaHeap[pid].paginas[j].contenido[k-1].isFree)
					{
						printf("el anterior esta free\n");
						tablaHeap[pid].paginas[j].contenido[k-1].size+=tablaHeap[pid].paginas[j].contenido[k].size+sizeof(t_heapMetaData);
						sacarHeapMetadata(pid,j,k);
						tablaHeap[pid].paginas[j].espacioLibre+=sizeof(t_heapMetaData);
					}
					break;
				}
				else
				{
					offset-=tablaHeap[pid].paginas[j].contenido[k].size;
				}
			}
			break;
		}
	}
	pthread_mutex_unlock(&mutexAlocar);
	if (tablaHeap[pid].paginas[j].espacioLibre==TAMPAGINA-sizeof(t_heapMetaData))
	{
		liberarPagina=malloc(sizeof(t_liberarPagina));
		liberarPagina->pid=pid;
		liberarPagina->pagina=tablaHeap[pid].paginas[j].pagina;
		pthread_mutex_lock(&mutexSolicitudMem);
		enviarDinamico(LIBERARPAGINA,SOCKETMEMORIA,liberarPagina);
		while(0>recv(SOCKETMEMORIA,&rv,sizeof(int),0));
		if (rv==-1)
		{
			return -1;
		}
		pthread_mutex_unlock(&mutexSolicitudMem);
		pthread_mutex_lock(&mutexAlocar);
		for (k = j; k < tablaHeap[pid].cantPaginas; k++)
		{
			if (!(k+1==tablaHeap[pid].cantPaginas))
			{
				memcpy(&(tablaHeap[pid].paginas[k]),&(tablaHeap[pid].paginas[k+1]),sizeof(t_heapProceso));
			}
		}
		tablaHeap[pid].cantPaginas--;
		if (tablaHeap[pid].cantPaginas==0)
		{
			free(tablaHeap[pid].paginas);
		}
		else
		{
			tablaHeap[pid].paginas=realloc(tablaHeap[pid].paginas,tablaHeap[pid].cantPaginas*sizeof(t_heapProceso));
		}
		pthread_mutex_unlock(&mutexAlocar);
		free(liberarPagina);
	}
	return 1;
}
int alocationSize(int pid, int pagina, int offset)
{
	int j,k;
	for (j = 0; j < tablaHeap[pid].cantPaginas; j++)
	{
		if (tablaHeap[pid].paginas[j].pagina==pagina)
		{
			for (k = 0; k < tablaHeap[pid].paginas[j].cantidadMetadatas; k++)
			{
				offset-=sizeof(t_heapMetaData);
				if (offset==0)
				{
					return tablaHeap[pid].paginas[j].contenido[k].size;
				}
				else
				{
					offset-=tablaHeap[pid].paginas[j].contenido[k].size;
				}
			}
		}
	}
}
int estaAbierto(char * path)
{
	int i;
	for (i = 0; i < CANTTABLAARCHIVOS; i++)
	{
		if (!strcmp(tablaArchivos[i].path,path))
		{
			return i;
		}
	}
	return -1;
}
int nuevaEntradaTablaArchivos(char * path)
{
	int globalFd;
	pthread_mutex_lock(&mutexTablaArchivos);
	printf("proximoFd: %i.\n", proximoFd);
	globalFd=proximoFd;
	proximoFd++;
	CANTTABLAARCHIVOS++;
	if (CANTTABLAARCHIVOS==1)
	{
		tablaArchivos=malloc(CANTTABLAARCHIVOS*sizeof(t_tablaGlobalArchivos));
	}
	else
	{
		tablaArchivos=realloc(tablaArchivos,CANTTABLAARCHIVOS*sizeof(t_tablaGlobalArchivos));
	}
	tablaArchivos[CANTTABLAARCHIVOS-1].path=calloc(1,string_length(path)+1);
	tablaArchivos[CANTTABLAARCHIVOS-1].tamanioPath=string_length(path);
	strcpy(tablaArchivos[CANTTABLAARCHIVOS-1].path,path);
	tablaArchivos[CANTTABLAARCHIVOS-1].vecesAbierto=1;
	tablaArchivos[CANTTABLAARCHIVOS-1].fd=globalFd;
	pthread_mutex_unlock(&mutexTablaArchivos);
	return CANTTABLAARCHIVOS-1;
}
int puedeBorrarse(int globalFd)
{
	int i;
	for (i = 0; i < CANTTABLAARCHIVOS; i++)
	{
		if (globalFd==tablaArchivos[i].fd && tablaArchivos[i].vecesAbierto==1)
		{
			return i;
		}
	}
	return 0;
}
int getPosicionTabla(int globalFd)
{
	int i;
	for (i = 0; i < CANTTABLAARCHIVOS; i++)
	{
		printf("fd de tabla: %i.\n", tablaArchivos[i].fd);
		printf("fd busca3: %i.\n", globalFd);
		if (globalFd==tablaArchivos[i].fd)
		{
			return i;
		}
	}
	return -1;
}
int closeFile(int globalFd)
{	int i;
	i=getPosicionTabla(globalFd);
	pthread_mutex_lock(&mutexTablaArchivos);
	tablaArchivos[i].vecesAbierto--;
	if (tablaArchivos[i].vecesAbierto==0)
	{
		free(tablaArchivos[i].path);
		for (; i < CANTTABLAARCHIVOS; i++)
		{
			if (!(i+1==CANTTABLAARCHIVOS))
			{
				memcpy(&tablaArchivos[i],&tablaArchivos[i+1],sizeof(t_tablaGlobalArchivos));
			}
		}
		CANTTABLAARCHIVOS--;
		tablaArchivos=realloc(tablaArchivos,CANTTABLAARCHIVOS*sizeof(t_tablaGlobalArchivos));
	}
	pthread_mutex_unlock(&mutexTablaArchivos);
}
int getPidReady(int socket){
	int value,a=0;
	sem_getvalue(&semReadys,&value);
	printf("Valor semaforo: %i\n", value);
	sem_wait(&semReadys);
	printf("hice wait\n");
	send(socket,&a,sizeof(int),0);
	while(0>recv(socket,&a,sizeof(int),0));
	printf("a: %i.\n", a);
	if (a==0)
	{
		sem_post(&semReadys);
		return -1;
	}
	return head(&COLAREADY,&CANTIDADREADYS,&mutexColaReady);
}
void escribirEnArchivoLog(char * contenidoAEscribir, FILE ** archivoDeLog,char * direccionDelArchivo){
	
	fseek(*archivoDeLog,0,SEEK_END);
	len=ftell(*archivoDeLog);
	fwrite(contenidoAEscribir,strlen(contenidoAEscribir),1,*archivoDeLog);
	fwrite("\n",1,1,*archivoDeLog);
}
void informarLeaks(int pid){
	int i,paginasSinLiberar,bytesAlocados,bytesLiberados,bytesSinLiberar,estructurasHeapSinLiberar,estructurasHeapAlocadas,estructurasHeapLiberadas;
	for (i = 0; i < CANTTABLAHEAP; i++)
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
				t_mensaje * mensaje;
				mensaje=malloc(sizeof(t_mensaje));
				mensaje->mensaje=calloc(1,400);
				printf("infoleaks\n");
				sprintf(mensaje->mensaje,"Memory leaks pid=%i:\n\tPaginas sin liberar: %i.\n\tBytes Alocados: %i,\tBytes Liberados: %i,\tBytes sin liberar: %i.\n\tHeap structs generadas: %i,\tHeap structs desechadas: %i,\tHeap structs sin desechar: %i.",pid,paginasSinLiberar,bytesAlocados,bytesLiberados,bytesSinLiberar,estructurasHeapAlocadas,estructurasHeapLiberadas,estructurasHeapSinLiberar);
				mensaje->tamanio=string_length(mensaje->mensaje);
				enviarDinamico(LEAKS,SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],mensaje);
				//send(SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],&(pid),sizeof(int),0);
				escribirEnArchivoLog(mensaje->mensaje,&KernelLog,nombreLog);
				free(mensaje->mensaje);
				free(mensaje);
			}
		}
	}
}
void finalizarPid(int pid, int exitCode)
{
	char * aux;
	t_mensaje * mensaje;
	printf("PID: %i.\n", pid);
	if (PCBS[pid].estado==EXEC)
	{
		pthread_mutex_lock(&mutexFinPid);
		EXITCODE=exitCode;
		PIDFIN=pid;
		pthread_mutex_lock(&mutexFinConsola);
		PIDFIN=-1;
		pthread_mutex_unlock(&mutexFinPid);
		pthread_mutex_unlock(&mutexDespuesFin);
	}
	else
	{
		if (PCBS[pid].estado!=EXIT)
		{
			mandarAExit(pid);
			setExitCode(pid,EXITCODE);
			cambiarEstado(pid,EXIT);
			aux=calloc(1,100);
			printf("exitCode: %i\n", exitCode);
			sprintf(aux,"Proceso Pid=%i finalizado, exitCode=%i",pid,exitCode);
			mensaje=malloc(sizeof(t_mensaje));
			mensaje->tamanio=strlen(aux);
			mensaje->mensaje=aux;
			enviarDinamico(MENSAJE,SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],mensaje);
			send(SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],&(pid),sizeof(int),0);
			informarLeaks(pid);
			escribirEnArchivoLog("envio mensaje", &KernelLog,nombreLog);
			pthread_mutex_lock(&mutexSolicitudMem);
			enviarDinamico(LIBERARMEMORIA,SOCKETMEMORIA,NULL);
			escribirEnArchivoLog("envio liberar memomria ", &KernelLog,nombreLog);
			send(SOCKETMEMORIA,&pid,sizeof(int),0);
			pthread_mutex_unlock(&mutexSolicitudMem);
			free(aux);
			free(mensaje);
		}
	}
}
void finalizarPidForzoso(int pid)
{
	char * aux;
	t_mensaje * mensaje;
	printf("PID: %i.\n", pid);
	if (PCBS[pid].estado==EXEC)
	{
		pthread_mutex_lock(&mutexFinPid);
		EXITCODE=-17;
		PIDFIN=pid;
		pthread_mutex_lock(&mutexFinConsola);
		PIDFIN=-1;
		pthread_mutex_unlock(&mutexFinPid);
	}
	else
	{
		if (PCBS[pid].estado!=EXIT)
		{
			setExitCode(pid,-17);
			cambiarEstado(pid,EXIT);
			mandarAExit(pid);
			escribirEnArchivoLog("envio mensaje", &KernelLog,nombreLog);
			pthread_mutex_lock(&mutexSolicitudMem);
			enviarDinamico(LIBERARMEMORIA,SOCKETMEMORIA,NULL);
			escribirEnArchivoLog("envio liberar memomria ", &KernelLog,nombreLog);
			send(SOCKETMEMORIA,&pid,sizeof(int),0);
			pthread_mutex_unlock(&mutexSolicitudMem);
		}
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
int estaBlocked(int pid){
	int i;
	for (i = 0; i < CANTIDADBLOCKS; i++)
		if (COLABLOCK[i]==pid)
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
	if ((*pcb)->indiceEtiquetas.etiquetas_size>0)
	{
		free((*pcb)->indiceEtiquetas.etiquetas);
	}
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
			PCBS[pcb->pid].indiceStack[i].variables=calloc(1,pcb->indiceStack[i].cantidadVariables*sizeof(t_variable));
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
void getPcb(t_pcb * pcb, int pid){
	int i,j;
	pthread_mutex_lock(&mutexPcbs);
	pcb->pid=PCBS[pid].pid;
	pcb->programCounter=PCBS[pid].programCounter;
	pcb->estado=PCBS[pid].estado;
	pcb->rafagasEjecutadas=PCBS[pid].rafagasEjecutadas;
	pcb->privilegiadasEjecutadas=PCBS[pid].privilegiadasEjecutadas;
	pcb->paginasHeap=PCBS[pid].paginasHeap;
	pcb->alocaciones=PCBS[pid].alocaciones;
	pcb->bytesAlocados=PCBS[pid].bytesAlocados;
	pcb->liberaciones=PCBS[pid].liberaciones;
	pcb->bytesLiberados=PCBS[pid].bytesLiberados;
	pcb->cantidadArchivos=PCBS[pid].cantidadArchivos;
	if (pcb->cantidadArchivos)
	{
		pcb->referenciaATabla=malloc(pcb->cantidadArchivos*sizeof(t_tablaArchivosDeProcesos));
	}
	for (i = 0; i < pcb->cantidadArchivos; i++)
	{	
		pcb->referenciaATabla[i].flags=PCBS[pid].referenciaATabla[i].flags;
		pcb->referenciaATabla[i].cursor=PCBS[pid].referenciaATabla[i].cursor;
		pcb->referenciaATabla[i].globalFd=PCBS[pid].referenciaATabla[i].globalFd;
		pcb->referenciaATabla[i].abierto=PCBS[pid].referenciaATabla[i].abierto;
	}
	pcb->paginasCodigo=PCBS[pid].paginasCodigo;
	pcb->posicionStack=PCBS[pid].posicionStack;
	pcb->cantidadInstrucciones=PCBS[pid].cantidadInstrucciones;
	if (pcb->cantidadInstrucciones)
	{
		pcb->indiceCodigo=malloc(pcb->cantidadInstrucciones*sizeof(t_intructions));
	}
	for (i = 0; i < pcb->cantidadInstrucciones; i++)
	{
		pcb->indiceCodigo[i].start=PCBS[pid].indiceCodigo[i].start;
		pcb->indiceCodigo[i].offset=PCBS[pid].indiceCodigo[i].offset;
	}
	pcb->indiceEtiquetas.etiquetas_size=PCBS[pid].indiceEtiquetas.etiquetas_size;
	if (pcb->indiceEtiquetas.etiquetas_size)
	{
		pcb->indiceEtiquetas.etiquetas=malloc(pcb->indiceEtiquetas.etiquetas_size);
		memcpy(pcb->indiceEtiquetas.etiquetas,PCBS[pid].indiceEtiquetas.etiquetas,pcb->indiceEtiquetas.etiquetas_size);
	}
	pcb->cantidadStack=PCBS[pid].cantidadStack;
	if (pcb->cantidadStack)
	{
		pcb->indiceStack=malloc(pcb->cantidadStack*sizeof(t_stack));
	}
	for (i = 0; i < pcb->cantidadStack; i++)
	{
		pcb->indiceStack[i].cantidadArgumentos=PCBS[pid].indiceStack[i].cantidadArgumentos;
		if (pcb->indiceStack[i].cantidadArgumentos)
		{
			pcb->indiceStack[i].argumentos=malloc(pcb->indiceStack[i].cantidadArgumentos*sizeof(t_argumento));
		}
		for (j = 0; j < pcb->indiceStack[i].cantidadArgumentos; j++)
		{
			pcb->indiceStack[i].argumentos[j].id=PCBS[pid].indiceStack[i].argumentos[j].id;
			pcb->indiceStack[i].argumentos[j].pagina=PCBS[pid].indiceStack[i].argumentos[j].pagina;
			pcb->indiceStack[i].argumentos[j].offset=PCBS[pid].indiceStack[i].argumentos[j].offset;
			pcb->indiceStack[i].argumentos[j].size=PCBS[pid].indiceStack[i].argumentos[j].size;
		}
		pcb->indiceStack[i].cantidadVariables=PCBS[pid].indiceStack[i].cantidadVariables;
		if (pcb->indiceStack[i].cantidadVariables)
		{
			pcb->indiceStack[i].variables=malloc(pcb->indiceStack[i].cantidadVariables*sizeof(t_variable));
		}
		for (j = 0; j < pcb->indiceStack[i].cantidadVariables; j++)
		{
			pcb->indiceStack[i].variables[j].id=PCBS[pid].indiceStack[i].variables[j].id;
			pcb->indiceStack[i].variables[j].pagina=PCBS[pid].indiceStack[i].variables[j].pagina;
			pcb->indiceStack[i].variables[j].offset=PCBS[pid].indiceStack[i].variables[j].offset;
			pcb->indiceStack[i].variables[j].size=PCBS[pid].indiceStack[i].variables[j].size;
		}
		pcb->indiceStack[i].posRetorno=PCBS[pid].indiceStack[i].posRetorno;
		pcb->indiceStack[i].varRetorno.pagina=PCBS[pid].indiceStack[i].varRetorno.pagina;
		pcb->indiceStack[i].varRetorno.offset=PCBS[pid].indiceStack[i].varRetorno.offset;
		pcb->indiceStack[i].varRetorno.size=PCBS[pid].indiceStack[i].varRetorno.size;
	}
	pcb->exitCode=PCBS[pid].exitCode;
	pthread_mutex_unlock(&mutexPcbs);
}
void consola(){
	int opcion,opcion2;
	int pid;
	int i,j,k;
	printf("\nPresione enter para iniciar la consola del Kernel.\n");
	getchar();
	while(CONSOLAHABILITADA){
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
				finalizarPid(pid,-6);
				printf("El proceso fue finalizado.\n");
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
void planificar(dataParaComunicarse ** dataDePlanificacion)
{
	// OBTENGO EL SOCKET Y LIBERO
	int socket=(*dataDePlanificacion)->socket;
	free(*dataDePlanificacion);
	flagLiberarData=1;
	escribirEnArchivoLog("en planificacion", &KernelLog,nombreLog);
	int pid,puntero,rv=1,auxInt,i,globalFd,flagQuantum=1,pagina,offset,primerAcceso=1,error=0,valor,estaPlanificacion=1,confirmacion;
	int * buffer;
	char * aux;
	t_path * path;
	t_heapMetaData * metadataAux;
	t_pcb * pcb;
	t_seleccionador * seleccionador;
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
	while(PLANIFICACIONHABILITADA && estaPlanificacion)
	{
		seleccionador=malloc(sizeof(t_seleccionador));
		rv=1;
		if (primerAcceso)
		{
			seleccionador->tipoPaquete=PCB;
			primerAcceso=0;
		}
		else
		{
			while(0>(rv=recv(socket, seleccionador, sizeof(t_seleccionador), 0)));
			if (rv==0)
			{
				printf("CPU desconectado\n");
				estaPlanificacion=0;
			}
		}
		if (rv!=0)
		{
				switch(seleccionador->tipoPaquete)
				{
					case ESPERONOVEDADES:
					if (!strcmp(ALGORITMO,"RR"))
					{	
						flagQuantum--;
					}
					escribirEnArchivoLog("en case espero novedades", &KernelLog,nombreLog);
					if (pid==PIDFIN) // SI FINALIZO EL PROCESO FORZADAMENTE MANDO FINALIZARPROCESO
					{
						enviarDinamico(FINALIZARPROCESO, socket,NULL);
						escribirEnArchivoLog("envio finalizar proceso", &KernelLog,nombreLog);
					}
					else if (error!=0)
					{
						enviarDinamico(FINALIZARPORERROR, socket,NULL);
						escribirEnArchivoLog("envio finalizar por error", &KernelLog,nombreLog);
					}
					else if (PCBS[pid].estado==EXIT && PCBS[pid].exitCode==0)
					{
						primerAcceso=1;
					}
					else if (estaBlocked(pid)) // SI ESTA BLOQUEADO MANDO PARAREJECUCION
					{
						enviarDinamico(PARAREJECUCION,socket, NULL);
						escribirEnArchivoLog("envio parar ejecucion", &KernelLog,nombreLog);
					}
					else if (flagQuantum==0) // SI TERMINO QUANTUM MANDO FINQUANTUM
					{
						enviarDinamico(FINQUANTUM, socket,NULL);
						escribirEnArchivoLog("envio fin de quantum", &KernelLog,nombreLog);
					}
					else // SI NO HAY NOVEDADES MANDO CONTINUAR
					{
						usleep(QUANTUM_SLEEP*1000);
						enviarDinamico(CONTINUAR, socket,NULL);
						escribirEnArchivoLog("envio continuar", &KernelLog,nombreLog);}		
					break;
					case PCB:	
					escribirEnArchivoLog("en case pcb", &KernelLog,nombreLog);
					pcb=malloc(sizeof(t_pcb));
					pid=getPidReady(socket);
					if (pid!=-1)
					{
						add(pid,&COLAEXEC,&CANTIDADEXECS,&mutexColaExec);
						cambiarEstado(pid,EXEC);
						getPcb(pcb,pid);
						usleep(QUANTUM_SLEEP*1000);
						enviarDinamico(PCB,socket,pcb);
						escribirEnArchivoLog("envio pcb", &KernelLog,nombreLog);
						if (!strcmp(ALGORITMO,"RR"))
						{	
							flagQuantum=QUANTUM;
						}
						liberarContenidoPcb(&pcb);
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
						if (existeCompartida(&(solicitudVariable->variable)))
						{
							// confirmacion=1;
							valor=valorCompartida(&(solicitudVariable->variable));
							// send(socket,&confirmacion,sizeof(int),0);
							send(socket,&(valor),sizeof(int),0);updatePCB(pcb);
						}
						else
						{
							// confirmacion=0;
							// send(socket,&confirmacion,sizeof(int),0);
							error=-12;
						}
						enviarDinamico(PCB,socket,pcb);
						free(solicitudVariable->variable);
						free(solicitudVariable);
					}
					else
					{
						estaPlanificacion=0;
					}
					liberarContenidoPcb(&pcb);
					break;
					case ASIGNARVARIABLECOMPARTIDA: // CPU QUIERE ASIGNAR UN VALOR A UNA VARIABLE COMPARTIDA
					escribirEnArchivoLog("en case asignar variable compartida", &KernelLog,nombreLog);
					asignarVariable=malloc(sizeof(t_asignarVariableCompartida));
					recibirDinamico(ASIGNARVARIABLECOMPARTIDA,socket,asignarVariable);
					pcb=malloc(sizeof(t_pcb));
					while(0>recv(socket, seleccionador, sizeof(t_seleccionador), 0));
					recibirDinamico(PCB,socket,pcb);
					pcb->privilegiadasEjecutadas++;
					escribirEnArchivoLog("recibo asignar variable compartida", &KernelLog,nombreLog);
					if (existeCompartida(&(asignarVariable->variable)))
					{
						// confirmacion=1;
						asignarCompartida(&(asignarVariable->variable),asignarVariable->valor);
						// send(socket,&confirmacion,sizeof(int),0);
						// send(socket,&(valor),sizeof(int),0);updatePCB(pcb);
					}
					else
					{
						// confirmacion=0;
						// send(socket,&confirmacion,sizeof(int),0);
						error=-12;
					}
					updatePCB(pcb);
					enviarDinamico(PCB,socket,pcb);
					liberarContenidoPcb(&pcb);
					free(asignarVariable->variable);
					free(asignarVariable);
					break;
					// SEMAFOROS
					case SOLICITUDSEMWAIT: // CPU ESTA HACIENDO WAIT (VER ESTRUCTURA)
					escribirEnArchivoLog("en case solicitud sem wait", &KernelLog,nombreLog);
					solicitudSemaforo=malloc(sizeof(t_solicitudSemaforo));
					recibirDinamico(SOLICITUDSEMWAIT,socket,solicitudSemaforo);
					escribirEnArchivoLog("recibo solicitud sem wait", &KernelLog,nombreLog);
					if (existeSemaforo(&(solicitudSemaforo->identificadorSemaforo)))
					{
						wait(&(solicitudSemaforo->identificadorSemaforo),pid);
					}
					else
					{
						error=-13; //SE INTENTA UTILIZAR UN SEMÁFORO INEXISTENTE
					}
					free(solicitudSemaforo->identificadorSemaforo);
					free(solicitudSemaforo);
					break;
					case SOLICITUDSEMSIGNAL: // CPU ESTA HACIENDO SIGNAL (VER ESTRUCTURA)
					escribirEnArchivoLog("en case solicitud sem signal", &KernelLog,nombreLog);
					solicitudSemaforo=malloc(sizeof(t_solicitudSemaforo));
					recibirDinamico(SOLICITUDSEMSIGNAL,socket,solicitudSemaforo);
					escribirEnArchivoLog("recibo solicitud sem signal", &KernelLog,nombreLog);
					if (existeSemaforo(&(solicitudSemaforo->identificadorSemaforo)))
					{
						warn(&(solicitudSemaforo->identificadorSemaforo));
					}
					else
					{
						error=-13; //SE INTENTA UTILIZAR UN SEMÁFORO INEXISTENTE
					}
					free(solicitudSemaforo->identificadorSemaforo);
					free(solicitudSemaforo);
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
					if (reservarMemoria->espacio>TAMPAGINA-2*sizeof(t_heapMetaData))
					{
						error=-8;
					}
					else
					{
						if(-1!=(pagina=alcanzaHeap(reservarMemoria->espacio,pid)))
						{
							printf("alcanza\n");
							puntero=alocar(reservarMemoria->espacio,pid,pagina);
						}
						else
						{	
							printf("no alcanza\n");
							pagina=nuevaPaginaHeap(pid);
							printf("nuevaPaginaHeap\n");
							if (pagina==-1)
							{
								error=-9;
							}
							else
							{
								printf("aloco en nuevaPaginaHeap\n");
								puntero=alocar(reservarMemoria->espacio,pid,pagina);
								pcb->paginasHeap++;
							}
						}
					}
					reservar=malloc(sizeof(t_heapMetaData));
					reservar->puntero=puntero;
					printf("ptr: %i\n", reservar->puntero);
					enviarDinamico(RESERVADOESPACIO,socket,reservar);
					escribirEnArchivoLog("envio resevar espacio", &KernelLog,nombreLog);
					updatePCB(pcb);
					enviarDinamico(PCB,socket,pcb);
					escribirEnArchivoLog("envio pcb", &KernelLog,nombreLog);
					free(reservarMemoria);
					free(reservar);
					liberarContenidoPcb(&pcb);
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
					pcb->bytesLiberados+=alocationSize(pid,pagina,offset);
					rv=liberarHeap(pid,pagina,offset);
					if (!rv)
					{
						error=-5;
					}
					updatePCB(pcb);
					enviarDinamico(PCB,socket,pcb);
					escribirEnArchivoLog("envio pcb", &KernelLog,nombreLog);
					free(liberarMemoria);
					liberarContenidoPcb(&pcb);
					break;
					// FILE SYSTEM
					case ABRIRARCHIVO: // CPU ABRE ARCHIVO
					escribirEnArchivoLog("en case abrir archivo", &KernelLog,nombreLog);
					abrirArchivo=malloc(sizeof(t_abrirArchivo));
					recibirDinamico(ABRIRARCHIVO,socket,abrirArchivo);
					strip(&(abrirArchivo->direccionArchivo));
					abrirArchivo->tamanio=string_length(abrirArchivo->direccionArchivo);
					escribirEnArchivoLog("recibo abrir archivo", &KernelLog,nombreLog);
					while(0>recv(socket, seleccionador, sizeof(t_seleccionador), 0));
					path=malloc(sizeof(t_path));
					path->path=calloc(1,abrirArchivo->tamanio+1);
					pcb=malloc(sizeof(t_pcb));
					recibirDinamico(PCB,socket,pcb);
					escribirEnArchivoLog("recibo pcb", &KernelLog,nombreLog);
					pcb->privilegiadasEjecutadas++;
					if ((i=estaAbierto(abrirArchivo->direccionArchivo))==-1)
					{
						printf("entro aca\n");
						strcpy(path->path,abrirArchivo->direccionArchivo);
						path->tamanio=string_length(path->path);
						enviarDinamico(VALIDARARCHIVO,SOCKETFS,path);
						escribirEnArchivoLog("envio validar archivo", &KernelLog,nombreLog);
						while(0>recv(SOCKETFS,&rv,sizeof(int),0));
						i=nuevaEntradaTablaArchivos(path->path);
						if (abrirArchivo->flags.creacion && rv==-1)
						{
							enviarDinamico(CREARARCHIVOFS,SOCKETFS,path);
							escribirEnArchivoLog("envio crear archivo fs", &KernelLog,nombreLog);
							recv(SOCKETFS,&auxInt,sizeof(int),0);
							if (auxInt==-1)
							{
								error=-20;
							}
						}
						else if (rv==-1 && !(abrirArchivo->flags.creacion))
						{
							error=-10; // NO EXISTE EL ARCHIVO Y NO SE TIENEN LOS PERMISOS PARA CREARLO
						}
					}
					else
					{
						pthread_mutex_lock(&mutexTablaArchivos);
						tablaArchivos[i].vecesAbierto++;
						pthread_mutex_unlock(&mutexTablaArchivos);
					}
					if (error!=-10)
					{
						pcb->cantidadArchivos++;
						if (pcb->cantidadArchivos==1)
						{
							pcb->referenciaATabla=malloc(sizeof(t_tablaArchivosDeProcesos));
						}
						else
							pcb->referenciaATabla=realloc(pcb->referenciaATabla,pcb->cantidadArchivos*sizeof(t_tablaArchivosDeProcesos));
						memcpy(&(pcb->referenciaATabla[pcb->cantidadArchivos-1].flags),&(abrirArchivo->flags),sizeof(t_banderas));
						pcb->referenciaATabla[pcb->cantidadArchivos-1].globalFd=tablaArchivos[i].fd;
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
					liberarContenidoPcb(&pcb);
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
					else if((i=puedeBorrarse(globalFd))!=-1)
					{
						path=malloc(sizeof(t_path));
						path->path=calloc(1,tablaArchivos[i].tamanioPath+1);
						path->tamanio=tablaArchivos[i].tamanioPath;
						strcpy(path->path,tablaArchivos[i].path);
						enviarDinamico(BORRARARCHIVOFS,SOCKETFS,path);
						free(path->path);
						free(path);
					}
					else
					{
						error=-11;  // SE INTENTA BORRAR UN ARCHIVO QUE OTRO PROCESO TIENE ABIERTO
					}
					updatePCB(pcb);
					enviarDinamico(PCB,socket,pcb);
					escribirEnArchivoLog("envio pcb", &KernelLog,nombreLog);
					free(borrarArchivo);
					liberarContenidoPcb(&pcb);
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
					else
					{
						closeFile(globalFd);
					}
					updatePCB(pcb);
					enviarDinamico(PCB,socket,pcb);
					escribirEnArchivoLog("envio pcb", &KernelLog,nombreLog);
					free(cerrarArchivo);
					liberarContenidoPcb(&pcb);
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
					liberarContenidoPcb(&pcb);
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
					else
					{
						pcb->privilegiadasEjecutadas++;
						if (pcb->referenciaATabla[(escribirArchivo->fdArchivo)-3].abierto==0)
						{
							error-14;
						}
						else if(pcb->referenciaATabla[(escribirArchivo->fdArchivo)-3].flags.escritura)
						{
							escribirArchivoFS=malloc(sizeof(t_escribirArchivoFS));
							escribirArchivoFS->size=escribirArchivo->tamanio;
							escribirArchivoFS->buffer=calloc(1,escribirArchivoFS->size+1);
							memcpy(escribirArchivoFS->buffer,escribirArchivo->informacion,escribirArchivoFS->size);
							escribirArchivoFS->offset=pcb->referenciaATabla[(escribirArchivo->fdArchivo)-3].cursor;
							globalFd=pcb->referenciaATabla[(escribirArchivo->fdArchivo)-3].globalFd;
							i=getPosicionTabla(globalFd);
							escribirArchivoFS->tamanioPath=tablaArchivos[i].tamanioPath;
							escribirArchivoFS->path=calloc(1,escribirArchivoFS->tamanioPath+1);
							strcpy(escribirArchivoFS->path,tablaArchivos[i].path);
							enviarDinamico(GUARDARDATOSFS,SOCKETFS,escribirArchivoFS);
							escribirEnArchivoLog("envio guardar datos fs", &KernelLog,nombreLog);
							while(0>recv(SOCKETFS,&rv,sizeof(int),0));
							if (!rv)
							{
								error=-20;
							}
							free(escribirArchivoFS->path);
							free(escribirArchivoFS->buffer);
							free(escribirArchivoFS);
						}
						else
						{
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
					liberarContenidoPcb(&pcb);
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
							i=getPosicionTabla(globalFd);
							leerArchivoFS->tamanioPath=tablaArchivos[i].tamanioPath;
							leerArchivoFS->path=calloc(1,leerArchivoFS->tamanioPath+1);
							printf("CANTTABLAARCHIVOS: %i.\n", CANTTABLAARCHIVOS);
							printf("i: %i.\n", i);
							strcpy(leerArchivoFS->path,tablaArchivos[i].path);
							enviarDinamico(OBTENERDATOSFS,SOCKETFS,leerArchivoFS);
							escribirEnArchivoLog("envio obtener datos fs", &KernelLog,nombreLog);
							while(0>recv(SOCKETFS,&rv,sizeof(int),0));
							if (!rv)
							{
								error=-2;
							}
							else
							{
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
						else
						{
							error=-3;
						}
					}
					updatePCB(pcb);
					enviarDinamico(PAQUETEFS,socket,paqueteFS);
					escribirEnArchivoLog("envio paquete fs", &KernelLog,nombreLog);
					free(paqueteFS->paquete);
					free(paqueteFS);
					free(buffer);
					enviarDinamico(PCB,socket,pcb);
					escribirEnArchivoLog("envio pcb", &KernelLog,nombreLog);
					free(leerArchivo);
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
					case PCBFINALIZADO: //FINALIZACION CORRECTA
					escribirEnArchivoLog("en case pcb finalizado ", &KernelLog,nombreLog);
					pcb=malloc(sizeof(t_pcb));
					recibirDinamico(PCBFINALIZADO,socket,pcb);
					escribirEnArchivoLog("recibo pcb finalizado", &KernelLog,nombreLog);
					pcb->rafagasEjecutadas++;
					updatePCB(pcb);
					setExitCode(pid,0);
					cambiarEstado(pid,EXIT);
					mandarAExit(pid);
					aux=calloc(1,100);
					sprintf(aux,"Finalizo correctamente el proceso PID=%i",pid);
					mensaje=malloc(sizeof(t_mensaje));
					mensaje->tamanio=strlen(aux);
					mensaje->mensaje=aux;
					enviarDinamico(MENSAJE,SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],mensaje);
					send(SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],&(pcb->pid),sizeof(int),0);
					informarLeaks(pcb->pid);
					escribirEnArchivoLog("envio mensaje", &KernelLog,nombreLog);
					pthread_mutex_lock(&mutexSolicitudMem);
					enviarDinamico(LIBERARMEMORIA,SOCKETMEMORIA,NULL);
					escribirEnArchivoLog("envio liberar memomria ", &KernelLog,nombreLog);
					send(SOCKETMEMORIA,&pid,sizeof(int),0);
					pthread_mutex_unlock(&mutexSolicitudMem);
					while(0>recv(socket,&rv,sizeof(int),0));
					if (rv)
					{
						estaPlanificacion=0;
					}
					primerAcceso=1;
					free(mensaje);
					free(aux);
					liberarContenidoPcb(&pcb);
					break;
					case PCBFINALIZADOPORCONSOLA:
					escribirEnArchivoLog("en case finalizado por consola", &KernelLog,nombreLog);
					pcb=malloc(sizeof(t_pcb));
					recibirDinamico(PCBFINALIZADOPORCONSOLA,socket,pcb);
					escribirEnArchivoLog("recibo pcb finalizado por consola", &KernelLog,nombreLog);
					pcb->rafagasEjecutadas++;
					updatePCB(pcb);
					setExitCode(pid,EXITCODE);
					cambiarEstado(pid,EXIT);
					mandarAExit(pid);
					aux=calloc(1,100);
					sprintf(aux,"Proceso Pid=%i finalizado, exitCode=%i",pid,EXITCODE);
					error=0;
					mensaje=malloc(sizeof(t_mensaje));
					mensaje->tamanio=strlen(aux);
					mensaje->mensaje=aux;
					if (EXITCODE!=-17)
					{
						enviarDinamico(MENSAJE,SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],mensaje);
						send(SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],&(pcb->pid),sizeof(int),0);
						informarLeaks(pcb->pid);
					}
					escribirEnArchivoLog("envio mensaje", &KernelLog,nombreLog);
					pthread_mutex_lock(&mutexSolicitudMem);
					enviarDinamico(LIBERARMEMORIA,SOCKETMEMORIA,NULL);
					escribirEnArchivoLog("envio liberar memomria ", &KernelLog,nombreLog);
					send(SOCKETMEMORIA,&pid,sizeof(int),0);
					pthread_mutex_unlock(&mutexSolicitudMem);
					while(0>recv(socket,&rv,sizeof(int),0));
					if (rv)
					{
						estaPlanificacion=0;
					}
					primerAcceso=1;
					pthread_mutex_unlock(&mutexFinConsola);
					pthread_mutex_lock(&mutexDespuesFin);
					free(mensaje);
					free(aux);
					liberarContenidoPcb(&pcb);
					break;
					case PCBERROR:
					escribirEnArchivoLog("en case pcb error", &KernelLog,nombreLog);
					pcb=malloc(sizeof(t_pcb));
					recibirDinamico(PCBERROR,socket,pcb);
					escribirEnArchivoLog("recibo pcb error", &KernelLog,nombreLog);
					pcb->rafagasEjecutadas++;
					updatePCB(pcb);
					setExitCode(pid,error);
					cambiarEstado(pid,EXIT);
					mandarAExit(pid);
					aux=calloc(1,100);
					sprintf(aux,"Proceso Pid=%i finalizado, exitCode=%i",pid,error);
					mensaje=malloc(sizeof(t_mensaje));
					mensaje->tamanio=strlen(aux);
					mensaje->mensaje=aux;
					enviarDinamico(MENSAJE,SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],mensaje);
					send(SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],&(pcb->pid),sizeof(int),0);
					informarLeaks(pcb->pid);
					error=0;
					escribirEnArchivoLog("envio mensaje", &KernelLog,nombreLog);
					pthread_mutex_lock(&mutexSolicitudMem);
					enviarDinamico(LIBERARMEMORIA,SOCKETMEMORIA,NULL);
					escribirEnArchivoLog("envio liberar memomria ", &KernelLog,nombreLog);
					send(SOCKETMEMORIA,&pid,sizeof(int),0);
					pthread_mutex_unlock(&mutexSolicitudMem);
					while(0>recv(socket,&rv,sizeof(int),0));
					if (rv)
					{
						estaPlanificacion=0;
					}
					primerAcceso=1;
					escribirEnArchivoLog("envio mensaje", &KernelLog,nombreLog);
					free(mensaje);
					free(aux);	
					liberarContenidoPcb(&pcb);		
					break;
					case PCBQUANTUM:
					escribirEnArchivoLog("en case pcb quantum", &KernelLog,nombreLog);
					pcb=malloc(sizeof(t_pcb));
					recibirDinamico(PCBQUANTUM,socket,pcb);
					escribirEnArchivoLog("recibo pcb quamtum", &KernelLog,nombreLog);
					pcb->rafagasEjecutadas++;
					updatePCB(pcb);
					cambiarEstado(pid,READY);
					oust(pid,&COLAEXEC,&CANTIDADEXECS,&mutexColaExec);
					add(pid,&COLAREADY,&CANTIDADREADYS,&mutexColaReady);
					sem_post(&semReadys);
					while(0>recv(socket,&rv,sizeof(int),0));
					if (rv)
					{
						estaPlanificacion=0;
					}
					primerAcceso=1;
					liberarContenidoPcb(&pcb);
					break;
					case PCBBLOQUEADO:
					escribirEnArchivoLog("en pcb bloqueado", &KernelLog,nombreLog);
					pcb=malloc(sizeof(t_pcb));
					recibirDinamico(PCBBLOQUEADO,socket,pcb);
					escribirEnArchivoLog("recibo pcb bloqueado", &KernelLog,nombreLog);
					pcb->rafagasEjecutadas++;
					updatePCB(pcb);
					cambiarEstado(pcb->pid,BLOCKED);
					while(0>recv(socket,&rv,sizeof(int),0));
					if (rv)
					{
						estaPlanificacion=0;
					}
					primerAcceso=1;
					liberarContenidoPcb(&pcb);
					break;
				}
		}
	free(seleccionador);
	}
}
void comunicarse(dataParaComunicarse ** dataDeConexion){
	escribirEnArchivoLog("en comunicarse", &KernelLog,nombreLog);
	// OBTENGO LOS DATOS PARA COMUNICACION Y LIBERO
	int socket=(*dataDeConexion)->socket;
	int interfaz=(*dataDeConexion)->interfaz;
	free(*dataDeConexion);
	flagLiberarData=1;
	// DECLARACION DE VARIABLES
	t_path * codigo;
	t_metadata_program * metadata;
	t_resultadoIniciarPrograma * resultado;
	t_pcb * pcb;
	t_arrayPids * arrayPids;
	t_solicitudMemoria * solicitudMemoria;
	t_seleccionador * seleccionador;
	int PidAFinalizar,pid,respuesta,rv,idConsola,i,estaComunicacion=1;
	// CICLO PRINCIPAL
	while(COMUNICACIONHABILITADA && estaComunicacion){
		// RECIBO EL SELECCIONADOR
		seleccionador=malloc(sizeof(t_seleccionador));
		while(0>(rv=recv(socket, seleccionador, sizeof(t_seleccionador), 0)));
		if (rv==0)
		{
			printf("Consola desconectada.\n");
			estaComunicacion=0;
			for (i = 0; i < ULTIMOPID; ++i)
			{
				if (SOCKETPID[i]==socket)
				{
					finalizarPidForzoso(i);
				}
			}
		}
		else
		{
			switch(seleccionador->tipoPaquete)
			{
				case PATH:	// RECIBIMOS EL CODIGO DE UN SCRIPT
				escribirEnArchivoLog("en case path", &KernelLog,nombreLog);
				codigo = malloc (sizeof(t_path));
				// ACLARACION: LA ESTRUCTURA CODIGO TIENE EN REALIDAD EL CODIGO COMPLETO Y NO EL PATH
				recibirDinamico(PATH, socket, codigo); 
				escribirEnArchivoLog("recibo path", &KernelLog,nombreLog);
				// RECIBO EL ID DEL PROCESO CONSOLA AL QUE PERTENECE EL SCRIPT
				while(0>recv(socket,&idConsola,sizeof(int),0)); 
				// GENERO EL PID
				pid = nextPid(); 
				// GUARDO SOCKET DE LA CONSOLA INDEXADO CON EL PID
				add(idConsola,&SOCKETSCONSOLA,&CANTIDADSOCKETSCONSOLA,&mutexSocketsConsola); 
				add(socket,&SOCKETPID,&CANTSOCKETSPID,&mutexSocketPid);
				// GENERO LA METADATA DEL SCRIPT
				metadata=metadata_desde_literal(codigo->path);
				// CREO EL PCB E INICIALIZO SUS VALORES
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
				pcb->paginasCodigo=calcularPaginas(TAMPAGINA,codigo->tamanio);
				pcb->programCounter=0;
				pcb->posicionStack=0;
				pcb->cantidadInstrucciones=metadata->instrucciones_size;
				pcb->indiceCodigo=malloc(metadata->instrucciones_size*sizeof(t_intructions));
				memcpy(pcb->indiceCodigo,metadata->instrucciones_serializado,metadata->instrucciones_size*sizeof(t_intructions));
				pcb->indiceEtiquetas.etiquetas_size=metadata->etiquetas_size;
				pcb->indiceEtiquetas.etiquetas=calloc(1,metadata->etiquetas_size);
				memcpy(pcb->indiceEtiquetas.etiquetas,metadata->etiquetas,metadata->etiquetas_size);
				pcb->cantidadStack=1;
				pcb->indiceStack=malloc(sizeof(t_stack));
				pcb->indiceStack[0].cantidadVariables=0;
				pcb->indiceStack[0].cantidadArgumentos=0;
				pcb->exitCode=1;
				// LE ASIGNO UN LUGAR EN LA TABLA DE HEAP
				pthread_mutex_lock(&mutexAlocar);
				CANTTABLAHEAP++;
				tablaHeap=realloc(tablaHeap,CANTTABLAHEAP*sizeof(t_heapGlobal));
				tablaHeap[CANTTABLAHEAP-1].pid=pcb->pid;
				tablaHeap[CANTTABLAHEAP-1].cantPaginas=0;
				tablaHeap[CANTTABLAHEAP-1].paginas=malloc(tablaHeap[CANTTABLAHEAP-1].cantPaginas*sizeof(t_heapProceso));
				pthread_mutex_unlock(&mutexAlocar);
				// LIBERO LA METADATA
				metadata_destruir(metadata); 
				// LO AGREGO A LA TABLA DE PCBS
				pthread_mutex_lock(&mutexPcbs);
				CANTIDADPCBS++;
				PCBS = realloc (PCBS,(CANTIDADPCBS) * sizeof(t_pcb));
				memcpy(&(PCBS[CANTIDADPCBS-1]),pcb,sizeof(t_pcb));
				pthread_mutex_unlock(&mutexPcbs);
				// ARMO LA SOLICITUD DE MEMORIA
				solicitudMemoria=malloc(sizeof(t_solicitudMemoria));
				solicitudMemoria->tamanioCodigo=codigo->tamanio;
				solicitudMemoria->codigo=calloc(1,solicitudMemoria->tamanioCodigo+1);
				strcpy(solicitudMemoria->codigo,codigo->path);
				solicitudMemoria->cantidadPaginasCodigo=pcb->paginasCodigo;
				solicitudMemoria->cantidadPaginasStack=STACK_SIZE;
				solicitudMemoria->pid=pid;
				solicitudMemoria->respuesta=0;
				pthread_mutex_lock(&mutexSolicitudMem);
				enviarDinamico(SOLICITUDMEMORIA,SOCKETMEMORIA,solicitudMemoria);
				escribirEnArchivoLog("envio solicitud memoria", &KernelLog,nombreLog);
				//LIBERO PUNTEROS
				free(codigo->path);
				free(codigo);
				free(solicitudMemoria->codigo);
				free(solicitudMemoria);
				free(pcb);
				// RECIBO LA RESPUESTA DE MEMORIA
				while(0>recv(SOCKETMEMORIA, &respuesta, sizeof(int), 0));
				pthread_mutex_unlock(&mutexSolicitudMem);
				// PREPARO LA RESPUESTA A CONSOLA
				resultado=malloc(sizeof(t_resultadoIniciarPrograma));
				resultado->pid=pid;
				// VERIFICO SI PUEDO PASAR A NEW
				if (respuesta==OK)
				{
					// AGREGO EL PID A LA COLA DE NEWS
					add(pid,&COLANEW,&CANTIDADNEWS,&mutexColaNew);
					resultado->resultado=1;
				}
				else if (respuesta==FAIL)
				{
				 	// AGREGO EL PID A LA COLA DE EXIT
					add(pid,&COLAEXIT,&CANTIDADEXITS,&mutexColaExit);
					setExitCode(pid,-1);
					cambiarEstado(pid,EXIT);
					resultado->resultado=0;
				}
				// VERIFICO SI HAY LUGAR EN LA COLA DE READYS
				rellenarColaReady(); 
				// ENVIO RESPUESTA A CONSOLA
				enviarDinamico(RESULTADOINICIARPROGRAMA,SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],resultado);
				escribirEnArchivoLog("envio resultado iniciar programa", &KernelLog,nombreLog);
				// LIBERO MEMORIA
				free(resultado);
				break;
				case FINALIZARPROGRAMA: // RECIBIMOS EL PID DE UN PROGRAMA ANSISOP A FINALIZAR
				escribirEnArchivoLog("en case finalizar programa", &KernelLog,nombreLog);
				recibirDinamico(FINALIZARPROGRAMA,socket,&PidAFinalizar);
				escribirEnArchivoLog("recibo finalizar programa", &KernelLog,nombreLog);
				finalizarPid(PidAFinalizar,-7);
				break;
				case ARRAYPIDS: // RECIBIMOS UN CONJUNTO DE PIDS A FINALIZAR
				escribirEnArchivoLog("en case arrays pids", &KernelLog,nombreLog); 
				arrayPids=malloc(sizeof(t_arrayPids));
				recibirDinamico(ARRAYPIDS,socket,&arrayPids);
				escribirEnArchivoLog("recibo arrays pids", &KernelLog,nombreLog);
				for (i = 0; i < arrayPids->cantidad; i++)
				{
					finalizarPid(arrayPids->pids[i],-6);
				}
				free(arrayPids);
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
			dataParaConexion=malloc(sizeof(dataParaComunicarse*));
			(*dataParaConexion) = malloc(sizeof(dataParaComunicarse));
			(*dataParaConexion)->interfaz=*interfaz; 
			(*dataParaConexion)->socket=socketNuevaConexion;
			if (*interfaz==CPU)
			{
				pthread_create(&hiloComunicador,NULL,(void *)planificar,dataParaConexion);
			}
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
				else
					pthread_create(&hiloComunicador,NULL,(void *)comunicarse,dataParaConexion);
			}
			// CONFIGURACION E INICIO DE HILO 
			if(!flag) 
			{
				pthread_detach(hiloComunicador);
				while(!flagLiberarData);
				flagLiberarData=0;
			}
			else 
			{
				flag=0;
				free(*dataParaConexion);
			}
			free(interfaz);
			free(dataParaConexion);
		}
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
pthread_mutex_init(&mutexFinConsola,NULL);
pthread_mutex_init(&mutexFinPid,NULL);
pthread_mutex_init(&mutexSocketPid,NULL);
pthread_mutex_init(&mutexDespuesFin,NULL);
pthread_mutex_init(&mutexSolicitudMem,NULL);
pthread_mutex_lock(&mutexFinConsola);
pthread_mutex_lock(&mutexDespuesFin);
sem_init(&semReadys,0,0);
// INICIO COLA READYS
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
	free(*buffer);
	free(buffer);
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