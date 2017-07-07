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
#include <commons/txt.h>
#include <commons/collections/list.h>
#include <parser/metadata_program.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>


#define BACKLOG 5
#define KERNEL 0
#define MEMORIA 1
#define CONSOLA 2
#define CPU 3
#define FS 4
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
#define PCBFINALIZADOPORCONSOLA 56

	#define ARRAYPIDS 51
	#define SOLICITUDMEMORIA 0
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

char *strip(const char *s) {
    char *p = malloc(strlen(s) + 1);
    if(p) {
        char *p2 = p;
        while(*s != '\0') {
            if(*s != '\t' && *s != '\n' && *s != ' ') {
                *p2++ = *s++;
            } else {
                ++s;
            }
        }
        *p2 = '\0';
    }
    return p;
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
	if (COLAREADY[0]!=-1)
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

void rellenarColaReady(){
	int i=0,cantVacios=0,j;
	while(1)
	{
		if (GRADO_MULTIPROG>CANTIDADREADYS)
		{
			for (; i < GRADO_MULTIPROG; i++)
				if (COLAREADY[i]==-1)
					cantVacios++;
			int primerReadyLibre;
			primerReadyLibre=GRADO_MULTIPROG-cantVacios;
			for (i=0; i<cantVacios; i++){
				if (CANTIDADNEWS>0)
				{
					pthread_mutex_lock(&mutexColaReady);
					CANTIDADREADYS++;
					COLAREADY=realloc(COLAREADY,CANTIDADREADYS);
					COLAREADY[primerReadyLibre+i]=COLANEW[0];
					pthread_mutex_unlock(&mutexColaReady);
					pthread_mutex_lock(&mutexColaNew);
					for (j = 0; j < CANTIDADNEWS; j++)
					{
						if (!(j+1==CANTIDADNEWS))
							COLANEW[j]=COLANEW[j+1];
					}
					CANTIDADNEWS--;
					COLANEW=realloc(COLANEW,CANTIDADNEWS*sizeof(int));
					pthread_mutex_unlock(&mutexColaNew);
				}
			}
		}
	}
}

void cambiarEstado(int pid, int estado){
	int i=0;
	while(PCBS[i].pid!=pid)
		i++;
	pthread_mutex_lock(&mutexPcbs);
	PCBS[i].estado=estado;
	pthread_mutex_unlock(&mutexPcbs);
}

int finalizarPid(int pid,int exitCode){
	int i=0, index=0;
	while(i<CANTIDADPCBS && PCBS[i].pid!=pid)
		i++;
	if (i==CANTIDADPCBS)
		return 0;
	printf("exitcode: %i\n", exitCode);
	printf("pid: %i\n", pid);
	printf("estado: %i\n", PCBS[i].estado);
	switch(PCBS[i].estado){
		case NEW:
			// BUSCO LA POSICION EN LA COLA DE NEW
			while(COLANEW[index]!=pid)
				index++;
			// LO SACO DE LA COLA DE NEW
			for (; index < CANTIDADNEWS; index++){
				pthread_mutex_lock(&mutexColaNew);
				if (index+1==CANTIDADNEWS)
					COLANEW[index]=-1;
				else
					COLANEW[index]=COLANEW[index+1];
				pthread_mutex_unlock(&mutexColaNew);
			}
			pthread_mutex_lock(&mutexColaNew);
			CANTIDADNEWS--;
			COLANEW=realloc(COLANEW,CANTIDADNEWS*sizeof(COLANEW[0]));
			pthread_mutex_unlock(&mutexColaNew);
		break;
		case READY:
			// LO SACO DE LA COLA DE READYS
					pthread_mutex_lock(&mutexColaReady);
			for (i=0; i < CANTIDADREADYS; i++){
				if (i+1==CANTIDADREADYS) {
					COLAREADY[i]=-1;
				}
				else {
					COLAREADY[i]=COLAREADY[i+1];
				}
			}
					pthread_mutex_unlock(&mutexColaReady);
		break;
		case EXEC:
			// BUSCO LA POSICION EN LA COLA DE EXEC
			while(COLAEXEC[index]!=pid)
				index++;
			// LO SACO DE LA COLA DE EXEC
			for (; index < CANTIDADEXECS; index++){
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
		break;
		case BLOCKED:
			// BUSCO LA POSICION EN LA COLA DE BLOCK
			while(COLABLOCK[index]!=pid)
				index++;
			// LO SACO DE LA COLA DE BLOCK
			for (; index < CANTIDADBLOCKS; index++){
				pthread_mutex_lock(&mutexColaBlock);
				if (index+1==CANTIDADBLOCKS)
					COLABLOCK[index]=-1;
				else
					COLABLOCK[index]=COLABLOCK[index+1];
				pthread_mutex_unlock(&mutexColaBlock);
			}
			pthread_mutex_lock(&mutexColaBlock);
			CANTIDADBLOCKS--;
			COLABLOCK=realloc(COLABLOCK,CANTIDADBLOCKS*sizeof(COLABLOCK[0]));
			pthread_mutex_unlock(&mutexColaBlock);
		break;
	}
	// ACTUALIZO EL PCB
	pthread_mutex_lock(&mutexPcbs);
	PCBS[i].exitCode=exitCode;
	PCBS[i].estado=EXIT;
	pthread_mutex_unlock(&mutexPcbs);
	// LO AGREGO A LA COLA DE EXIT
	pthread_mutex_lock(&mutexColaExit);
	CANTIDADEXITS++;
	COLAEXIT = realloc (COLAEXIT,(CANTIDADEXITS) * sizeof(COLAEXIT[0]));
	COLAEXIT[CANTIDADEXITS-1]=pid;
	pthread_mutex_unlock(&mutexColaExit);
	return 1;
}


int estaExec(int pid){
	int i=0;
	for (; i < CANTIDADEXECS; i++)
		if (COLAEXEC[i]==pid)
			return 1;
	return 0;
}

void updatePCB(t_pcb * pcb){
	pthread_mutex_lock(&mutexPcbs);
	PCBS[pcb->pid]=*pcb;
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
							if (PCBS[i].referenciaATabla[j].flags.creacion)
								printf("c");
							if (PCBS[i].referenciaATabla[j].flags.lectura)
								printf("r");
							if (PCBS[i].referenciaATabla[j].flags.escritura)
								printf("w");
							printf("'\tFD Global: %i\n",PCBS[i].referenciaATabla[j].globalFd);
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
				printf("En desarrollo.\n");
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

void quantum(int * flagQuantum){
	sleep(QUANTUM*QUANTUM_SLEEP);
	*flagQuantum=1;
}

void planificar(dataParaComunicarse * dataDePlanificacion){
	printf("en planificar\n");
	int pid,estaba,_pid,cantBytes,posPagAux,posPagNueva,puntero,bytesLibres;
	int rv,i,j,k,l,globalFd,intAux,flag,cantUsados,usadosRecorridos,primerFree,cantLibres,proximoMetadata;
	char * aux;
	t_path * path;
	void * paginaAux,*paginaNueva;
	t_heapMetaData * metadataAux;
	t_pcb * pcb=malloc(sizeof(t_pcb));
	int cpuLibre = 1;
	int flagQuantum,pagina,offset;
	t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
	seleccionador->tipoPaquete=PCB;
	pthread_t hiloQuantum;
	int primerAcceso=1,error=0;
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
	t_paqueteFS * paqueteFS;
	t_peticionBytes * peticion;
	t_almacenarBytes * bytes;
	t_solicitudAsignar * solicitudPaginaHeap;
	t_reservar * reservar;
	while(PLANIFICACIONHABILITADA)
	{
		
		if (primerAcceso){
					primerAcceso=0;}
		else
			while(0>recv(dataDePlanificacion->socket, seleccionador, sizeof(t_seleccionador), 0));
		switch(seleccionador->tipoPaquete)
		{
			case ESPERONOVEDADES:
				if (estaBlocked(pid)) // SI ESTA BLOQUEADO MANDO PARAREJECUCION
					enviarDinamico(PARAREJECUCION,dataDePlanificacion->socket, NULL);
				else if (flagQuantum) // SI TERMINO QUANTUM MANDO FINQUANTUM
					enviarDinamico(FINQUANTUM, dataDePlanificacion->socket,NULL);
				else if (error!=0){
					enviarDinamico(FINALIZARPORERROR, dataDePlanificacion->socket,NULL);
				}
				else if (PCBS[pid].estado==EXIT && (PCBS[pid].exitCode==-7 || PCBS[pid].exitCode==-6)) // SI FINALIZO EL PROCESO FORZADAMENTE MANDO FINALIZARPROCESO
				 	enviarDinamico(FINALIZARPROCESO, dataDePlanificacion->socket,NULL);
				else if (PCBS[pid].estado==EXIT && PCBS[pid].exitCode==0)
					{seleccionador->tipoPaquete=PCB;
					primerAcceso=1;}
				else // SI NO HAY NOVEDADES MANDO CONTINUAR
					enviarDinamico(CONTINUAR, dataDePlanificacion->socket,NULL);
				break;
			case PCB:	
						while(!hayProcesosReady() || PLANIFICACIONPAUSADA);
						pthread_mutex_lock(&mutexProcesosReady);
						// OBTENGO EL PRIMER PID DE LA COLA DE LISTOS
						pid = COLAREADY[0];
						// LO SACO DE DICHA COLA Y OBTENGO SU PCB
						// CAMBIO ESTADO A EJECUTANDO
						cambiarEstado(pid,EXEC);
						getPcbAndRemovePid(pid,pcb);
						pthread_mutex_unlock(&mutexProcesosReady);
						// LO PONGO EN LA COLA DE EJECUTANDO
						pthread_mutex_lock(&mutexColaExec);
						CANTIDADEXECS++;
						COLAEXEC = realloc (COLAEXEC,(CANTIDADEXECS) * sizeof(COLAEXEC[0]));
						COLAEXEC[CANTIDADEXECS-1]=pid;
						pthread_mutex_unlock(&mutexColaExec);
						// LE MANDO EL PCB AL CPU
						enviarDinamico(PCB,dataDePlanificacion->socket,pcb);
						cpuLibre=0;
						if (ALGORITMO == "RR")
						{
							flagQuantum=0;
							pthread_create(&hiloQuantum,NULL,(void *)quantum,&flagQuantum);
						}
			break;
			// VARIABLES COMPARTIDAS
			case SOLICITUDVALORVARIABLE: // CPU PIDE EL VALOR DE UNA VARIABLE COMPARTIDA
				solicitudVariable=malloc(sizeof(t_solicitudValorVariable));
				recibirDinamico(SOLICITUDVALORVARIABLE,dataDePlanificacion->socket,solicitudVariable);
				for (i = 0; i < CANTIDADVARS; i++)
					if (!strcmp(SHARED_VARS[i]+1,solicitudVariable->variable))
						send(dataDePlanificacion->socket,&(SHARED_VALUES[i]),sizeof(int),0);
				free(solicitudVariable);
			break;
			case ASIGNARVARIABLECOMPARTIDA: // CPU QUIERE ASIGNAR UN VALOR A UNA VARIABLE COMPARTIDA
				asignarVariable=malloc(sizeof(t_asignarVariableCompartida));
				recibirDinamico(ASIGNARVARIABLECOMPARTIDA,dataDePlanificacion->socket,asignarVariable);
				for (i = 0; i < CANTIDADVARS; i++)
				{
					if (!strcmp(SHARED_VARS[i]+1,asignarVariable->variable))
					{
						pthread_mutex_lock(&mutexCompartidas);
						SHARED_VALUES[i]=asignarVariable->valor;
						pthread_mutex_unlock(&mutexCompartidas);
					}
				}
				free(asignarVariable);
			break;
			// SEMAFOROS
			case SOLICITUDSEMWAIT: // CPU ESTA HACIENDO WAIT (VER ESTRUCTURA)
				solicitudSemaforo=malloc(sizeof(t_solicitudSemaforo));
				solicitudSemaforo->identificadorSemaforo=calloc(1,1);
				recibirDinamico(SOLICITUDSEMWAIT,dataDePlanificacion->socket,solicitudSemaforo);
				solicitudSemaforo->identificadorSemaforo=strip(solicitudSemaforo->identificadorSemaforo);
				for (i = 0; i < CANTIDADSEM; i++)
				{
					if (!strcmp(SEM_IDS[i],solicitudSemaforo->identificadorSemaforo))
					{
						if (atoi(SEM_INIT[i])>0)
						{
							pthread_mutex_lock(&mutexSemaforos);
							intAux=atoi(SEM_INIT[i]);
							intAux--;
							sprintf(aux, "%i", intAux);
							strcpy(SEM_INIT[i],aux);
							pthread_mutex_unlock(&mutexSemaforos);
						}
						else
						{
							pthread_mutex_lock(&mutexColaBlock);
							CANTIDADBLOCKS++;
							COLABLOCK = realloc (COLABLOCK,(CANTIDADBLOCKS) * sizeof(COLABLOCK[0]));
							COLABLOCK[CANTIDADBLOCKS-1]=pid;
							pthread_mutex_unlock(&mutexColaBlock);
							pthread_mutex_lock(&mutexSemaforos);
							CANTIDADBLOCKPORSEM[i]++;
							BLOQUEADOSPORSEM[i]=realloc(BLOQUEADOSPORSEM[i],CANTIDADBLOCKPORSEM[i]*sizeof(int));
							(BLOQUEADOSPORSEM[i])[CANTIDADBLOCKPORSEM[i]-1]=pid;
							pthread_mutex_unlock(&mutexSemaforos);
							j=0;
							while(COLAEXEC[j]!=_pid)
								j++;
							for (j; j < CANTIDADEXECS; j++)
							{
								pthread_mutex_lock(&mutexColaExec);
								if (j+1==CANTIDADEXECS)
									COLAEXEC[j]=-1;
								else
									COLAEXEC[j]=COLAEXEC[j+1];
								pthread_mutex_unlock(&mutexColaExec);
							}
							pthread_mutex_lock(&mutexColaExec);
							CANTIDADEXECS--;
							COLAEXEC=realloc(COLAEXEC,CANTIDADEXECS*sizeof(COLAEXEC[0]));
							pthread_mutex_unlock(&mutexColaExec);
							pthread_mutex_lock(&mutexPcbs);
							PCBS[_pid].estado=BLOCKED;
							pthread_mutex_unlock(&mutexPcbs);

						}
					}
				}
				free(solicitudSemaforo);
			break;
			case SOLICITUDSEMSIGNAL: // CPU ESTA HACIENDO SIGNAL (VER ESTRUCTURA)
				solicitudSemaforo=malloc(sizeof(t_solicitudSemaforo));
				solicitudSemaforo->identificadorSemaforo=calloc(1,1);
				recibirDinamico(SOLICITUDSEMSIGNAL,dataDePlanificacion->socket,solicitudSemaforo);
				solicitudSemaforo->identificadorSemaforo=strip(solicitudSemaforo->identificadorSemaforo);
				for (i = 0; i < CANTIDADSEM; i++)
				{
					if (!strcmp(SEM_IDS[i],solicitudSemaforo->identificadorSemaforo))
					{
						if(CANTIDADBLOCKPORSEM[i]!=0)
						{
							printf("hola\n");
							_pid=BLOQUEADOSPORSEM[i][0];
							printf("%i\n", _pid);
							pthread_mutex_lock(&mutexSemaforos);
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
							while(COLABLOCK[j]!=_pid)
								j++;
							pthread_mutex_unlock(&mutexSemaforos);
							for (; j < CANTIDADBLOCKS; j++)
							{
								pthread_mutex_lock(&mutexColaBlock);
								if (j+1==CANTIDADBLOCKS)
									COLABLOCK[j]=-1;
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
							pthread_mutex_lock(&mutexSemaforos);
							intAux=atoi(SEM_INIT[i]);
							intAux++;
							sprintf(aux, "%i", intAux);
							strcpy(SEM_INIT[i],aux);
							pthread_mutex_unlock(&mutexSemaforos);
						}
					}
				}
				free(solicitudSemaforo);
			break;
			// HEAP
			case RESERVARESPACIO: // CPU RESERVA LUGAR EN EL HEAP
				reservarMemoria=malloc(sizeof(t_reservarEspacioMemoria));
				recibirDinamico(RESERVARESPACIO,dataDePlanificacion->socket,reservarMemoria);
				recibirDinamico(PCB,dataDePlanificacion->socket,pcb);
				pcb->alocaciones++;
				pcb->bytesAlocados+=reservarMemoria->espacio;
				pcb->privilegiadasEjecutadas++;
				pthread_mutex_lock(&mutexAlocar);
				if (reservarMemoria->espacio>TAMPAGINA-2*sizeof(t_heapMetaData));
				{
					error=-8;
				}
				for (i = 0; i < CANTTABLAHEAP; i++)
				{
					if (tablaHeap[i].pid==pid)
					{
						break;
					}
				}
				for (j = 0; j < tablaHeap[i].cantPaginas; j++)
				{
					if (tablaHeap[i].paginas[j].espacioLibre<=(reservarMemoria->espacio)-sizeof(t_heapMetaData))
					{
						break;
					}
				}
				if (j!=tablaHeap[i].cantPaginas)
				{	flag=0;
					for (k = 0; flag; k++)
					{	
						if (k==tablaHeap[i].paginas[j].cantidadMetadatas)
						{
							paginaAux=calloc(1,TAMPAGINA);
							paginaNueva=calloc(1,TAMPAGINA);
							metadataAux=malloc(sizeof(t_heapMetaData)*tablaHeap[i].paginas[j].cantidadMetadatas);
							memcpy(metadataAux,tablaHeap[i].paginas[j].contenido,sizeof(t_heapMetaData)*tablaHeap[i].paginas[j].cantidadMetadatas);
							peticion=malloc(sizeof(t_peticionBytes));
							peticion->pid=pid;
							peticion->pagina=tablaHeap[i].paginas[j].pagina;
							peticion->offset=0;
							peticion->size=TAMPAGINA;
							enviarDinamico(SOLICITUDBYTES,SOCKETMEMORIA,peticion);
							while(0>recv(SOCKETMEMORIA,&rv,sizeof(int),0)){
									perror("asd:");
							}
							if (rv==1)
							{
								while(0>recv(SOCKETMEMORIA,paginaAux,TAMPAGINA,0)){
									perror("asd:");
								}
							}
							cantUsados=0;
							cantLibres=0;
							bytesLibres=0;
							for (k = 0; k < tablaHeap[i].paginas[j].cantidadMetadatas; k++)
							{
								if(tablaHeap[i].paginas[j].contenido[k].isFree==false)
									cantUsados++;
								if(tablaHeap[i].paginas[j].contenido[k].isFree==true)
								{
									cantLibres++;
									bytesLibres+=tablaHeap[i].paginas[j].contenido[k].size;
								}
							}
							usadosRecorridos=0;
							k=0;
							proximoMetadata=0;
							while(usadosRecorridos<cantUsados){
								if (metadataAux[k].isFree==false)
								{
									memcpy(&(tablaHeap[i].paginas[j].contenido[proximoMetadata]),&(metadataAux[k]),sizeof(t_heapMetaData));
									posPagAux=sizeof(t_heapMetaData);
									for (l = 0; l < k; l++)
									{
										posPagAux+=sizeof(t_heapMetaData)+metadataAux[l].size;
									}
									posPagNueva=sizeof(t_heapMetaData);
									for (l = 0; l < proximoMetadata; l++)
									{
										posPagNueva+=sizeof(t_heapMetaData)+tablaHeap[i].paginas[j].contenido[proximoMetadata].size;
									}
									memcpy(paginaNueva+posPagNueva,paginaAux+posPagAux,metadataAux[k].size);
									proximoMetadata++;
									usadosRecorridos++;

								}
								k++;
							}
							tablaHeap[i].paginas[j].contenido[cantUsados].isFree=true;
							tablaHeap[i].paginas[j].contenido[cantUsados].size=bytesLibres+(cantLibres-1)*sizeof(t_heapMetaData);
							tablaHeap[i].paginas[j].contenido=realloc(tablaHeap[i].paginas[j].contenido,(cantUsados+1)*sizeof(t_heapMetaData));
							bytes=malloc(sizeof(t_almacenarBytes));
							bytes->pid=pid;
							bytes->pagina=tablaHeap[i].paginas[j].pagina;
							bytes->offset=0;
							bytes->size=TAMPAGINA;
							bytes->valor=calloc(1,TAMPAGINA);
							memcpy(bytes->valor,paginaNueva,TAMPAGINA);
							enviarDinamico(ALMACENARBYTES,SOCKETMEMORIA,bytes);
							k=0;

						}
						if (k==tablaHeap[i].paginas[j].cantidadMetadatas-1)
						{
							if ((reservarMemoria->espacio<=(tablaHeap[i].paginas[j].contenido[k].size)-sizeof(t_heapMetaData) )&& tablaHeap[i].paginas[j].contenido[k].isFree)
							{
								tablaHeap[i].paginas[j].contenido[k].isFree=false;
								intAux=tablaHeap[i].paginas[j].contenido[k].size;
								tablaHeap[i].paginas[j].contenido[k].size=reservarMemoria->espacio;
								tablaHeap[i].paginas[j].cantidadMetadatas++;
								tablaHeap[i].paginas[j].contenido=realloc(tablaHeap[i].paginas[j].contenido,tablaHeap[i].paginas[j].cantidadMetadatas*sizeof(t_heapMetaData));
								tablaHeap[i].paginas[j].contenido[k+1].isFree=true;
								tablaHeap[i].paginas[j].contenido[k+1].size=intAux-reservarMemoria->espacio-sizeof(t_heapMetaData);
								tablaHeap[i].paginas[j].espacioLibre-=sizeof(t_heapMetaData)-reservarMemoria->espacio;
								flag=1;
							}
						}
						else{
							if ((reservarMemoria->espacio<=(tablaHeap[i].paginas[j].contenido[k].size)) && tablaHeap[i].paginas[j].contenido[k].isFree)
							{
								if (reservarMemoria->espacio==(tablaHeap[i].paginas[j].contenido[k].size))
								{
									tablaHeap[i].paginas[j].contenido[k].isFree=false;
									tablaHeap[i].paginas[j].espacioLibre-=reservarMemoria->espacio;
									flag=1;
								}
								else{
									tablaHeap[i].paginas[j].contenido[k].isFree=false;
									intAux=tablaHeap[i].paginas[j].contenido[k].size;
									tablaHeap[i].paginas[j].contenido[k].size=reservarMemoria->espacio;
									tablaHeap[i].paginas[j].cantidadMetadatas++;
									tablaHeap[i].paginas[j].contenido=realloc(tablaHeap[i].paginas[j].contenido,tablaHeap[i].paginas[j].cantidadMetadatas*sizeof(t_heapMetaData));
									tablaHeap[i].paginas[j].contenido[k+1].isFree=true;
									tablaHeap[i].paginas[j].contenido[k+1].size=intAux-reservarMemoria->espacio-sizeof(t_heapMetaData);
									tablaHeap[i].paginas[j].espacioLibre-=sizeof(t_heapMetaData)-reservarMemoria->espacio;
									flag=1;
								}
							}
						}
					}
					puntero=(tablaHeap[i].paginas[j].pagina-pcb->paginasCodigo)*TAMPAGINA;
					for (l = 0; l < k; l++)
					{
						puntero+=sizeof(t_heapMetaData)+tablaHeap[i].paginas[j].contenido[l].size;
					}
					puntero+=sizeof(t_heapMetaData);
				}
				else
				{
					solicitudPaginaHeap=malloc(sizeof(t_solicitudAsignar));
					solicitudPaginaHeap->pid=pid;
					solicitudPaginaHeap->paginasAAsignar=1;
					enviarDinamico(ASIGNARPAGINAS,SOCKETMEMORIA,solicitudPaginaHeap);
					while(0>recv(SOCKETMEMORIA,&rv,sizeof(int),0)){
							perror("asd:");
					}
					if (rv==-1)
					{
						error=-9;
					}
					else{
						pcb->paginasHeap++;
						tablaHeap[i].cantPaginas++;
						tablaHeap[i].paginas=realloc(tablaHeap[i].paginas,tablaHeap[i].cantPaginas);
						tablaHeap[i].paginas[tablaHeap[i].cantPaginas-1].pagina=pcb->paginasCodigo+STACK_SIZE+pcb->paginasHeap;
						tablaHeap[i].paginas[tablaHeap[i].cantPaginas-1].espacioLibre=TAMPAGINA-10-reservarMemoria->espacio;
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
				pthread_mutex_unlock(&mutexAlocar);
				reservar=malloc(sizeof(t_heapMetaData));
				reservar->puntero=puntero;
				enviarDinamico(RESERVADOESPACIO,dataDePlanificacion->socket,reservar);
				updatePCB(pcb);
				enviarDinamico(PCB,dataDePlanificacion->socket,pcb);
				free(reservarMemoria);
				free(reservar);
			break;
			case LIBERARESPACIOMEMORIA: // CPU LIBERA MEMORIA DEL HEAP
				liberarMemoria=malloc(sizeof(t_liberarMemoria));
				recibirDinamico(LIBERARESPACIOMEMORIA,dataDePlanificacion->socket,liberarMemoria);
				recibirDinamico(PCB,dataDePlanificacion->socket,pcb);
				pcb->privilegiadasEjecutadas++;
				pcb->liberaciones++;
				pagina=liberarMemoria->direccionMemoria/TAMPAGINA+pcb->paginasCodigo;
				offset=liberarMemoria->direccionMemoria%TAMPAGINA;
				pthread_mutex_lock(&mutexAlocar);
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
										tablaHeap[i].paginas[j].contenido[k].isFree=true;
									}
									else{
										offset-=tablaHeap[i].paginas[j].contenido[k].size;
									}
									k++;
								}
							}
						}
					}
				}
				pthread_mutex_unlock(&mutexAlocar);
				updatePCB(pcb);
				enviarDinamico(PCB,dataDePlanificacion->socket,pcb);
				free(liberarMemoria);
			break;
			// FILE SYSTEM
			case ABRIRARCHIVO: // CPU ABRE ARCHIVO
				abrirArchivo=malloc(sizeof(t_abrirArchivo));
				recibirDinamico(ABRIRARCHIVO,dataDePlanificacion->socket,abrirArchivo);
				recibirDinamico(PCB,dataDePlanificacion->socket,pcb);
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
				if (!estaba)
				{
					if (abrirArchivo->flags.creacion)
					{
						pthread_mutex_lock(&mutexTablaArchivos);
						globalFd=proximoFd;
						proximoFd++;
						CANTTABLAARCHIVOS++;
						tablaArchivos=realloc(tablaArchivos,CANTTABLAARCHIVOS*sizeof(t_tablaGlobalArchivos));
						memcpy(tablaArchivos[CANTTABLAARCHIVOS-1].path,abrirArchivo->direccionArchivo,abrirArchivo->tamanio);
						tablaArchivos[CANTTABLAARCHIVOS-1].vecesAbierto=1;
						pthread_mutex_unlock(&mutexTablaArchivos);
						path=malloc(sizeof(t_path));
						path->path=calloc(1,abrirArchivo->tamanio);
						path->tamanio=abrirArchivo->tamanio;
						memcpy(path->path,abrirArchivo->direccionArchivo,abrirArchivo->tamanio);
						enviarDinamico(CREARARCHIVOFS,SOCKETFS,path);
						free(path);
					}
					else{
						flag=1;
						error=-20;
					}
				}
				if(estaba){
					pthread_mutex_lock(&mutexTablaArchivos);
					tablaArchivos[globalFd].vecesAbierto++;
					pthread_mutex_unlock(&mutexTablaArchivos);
				}if (!flag)
				{
					pcb->cantidadArchivos++;
					pcb->referenciaATabla=realloc(pcb->referenciaATabla,pcb->cantidadArchivos);
					memcpy(&(pcb->referenciaATabla[pcb->cantidadArchivos-1].flags),&(abrirArchivo->flags),sizeof(t_banderas));
					pcb->referenciaATabla[pcb->cantidadArchivos-1].globalFd=globalFd;
					pcb->referenciaATabla[pcb->cantidadArchivos-1].cursor=0;
					pcb->referenciaATabla[pcb->cantidadArchivos-1].abierto=1;
					updatePCB(pcb);
				}
				enviarDinamico(PCB,dataDePlanificacion->socket,pcb);
				fd=malloc(sizeof(t_fdParaLeer));
				fd->fd=pcb->cantidadArchivos-1+3;
				enviarDinamico(ABRIOARCHIVO,dataDePlanificacion->socket,fd);
				free(fd);
				free(abrirArchivo);
			break;
			case BORRARARCHIVO: // CPU BORRA ARCHIVO
				borrarArchivo=malloc(sizeof(t_borrarArchivo));
				recibirDinamico(BORRARARCHIVO,dataDePlanificacion->socket,borrarArchivo);
				recibirDinamico(PCB,dataDePlanificacion->socket,pcb);
				pcb->privilegiadasEjecutadas++;
				globalFd=pcb->referenciaATabla[(borrarArchivo->fdABorrar)-3].globalFd;
				for (i = 0; i < CANTTABLAARCHIVOS; i++)
				{
					if (globalFd==tablaArchivos[i].fd)
					{
						break;
					}
				}
				if (tablaArchivos[i].vecesAbierto!=1)
				{
					error=-20;
				}
				else{
					path=malloc(sizeof(t_path));
					path->path=calloc(1,strlen(tablaArchivos[i].path));
					path->tamanio=abrirArchivo->tamanio;
					memcpy(path->path,tablaArchivos[i].path,path->tamanio);
					enviarDinamico(BORRARARCHIVOFS,SOCKETFS,path);
					free(path);
					pthread_mutex_lock(&mutexTablaArchivos);
					for (; i < CANTTABLAARCHIVOS; i++)
					{
						if (!(i+1==CANTTABLAARCHIVOS))
						{
							memcpy(&tablaArchivos[i],&tablaArchivos[i+1],sizeof(t_tablaGlobalArchivos));
						}
					}
					CANTTABLAARCHIVOS--;
					tablaArchivos=realloc(tablaArchivos,CANTTABLAARCHIVOS*sizeof(t_tablaGlobalArchivos));
					pthread_mutex_unlock(&mutexTablaArchivos);
				}
				pcb->referenciaATabla[(borrarArchivo->fdABorrar)-3].abierto=0;
				updatePCB(pcb);
				enviarDinamico(PCB,dataDePlanificacion->socket,pcb);
				free(borrarArchivo);
			break;
			case CERRARARCHIVO: // CPU CIERRA ARCHIVO
				cerrarArchivo=malloc(sizeof(t_cerrarArchivo));
				recibirDinamico(CERRARARCHIVO,dataDePlanificacion->socket,cerrarArchivo);
				recibirDinamico(PCB,dataDePlanificacion->socket,pcb);
				pcb->privilegiadasEjecutadas++;
				globalFd=pcb->referenciaATabla[(cerrarArchivo->descriptorArchivo)-3].globalFd;
				for (i = 0; i < CANTTABLAARCHIVOS; i++)
				{
					if (globalFd==tablaArchivos[i].fd)
					{
						break;
					}
				}
				pthread_mutex_lock(&mutexTablaArchivos);
				tablaArchivos[i].vecesAbierto--;
				for (; i < CANTTABLAARCHIVOS; i++)
					{
						if (!(i+1==CANTTABLAARCHIVOS))
						{
							memcpy(&tablaArchivos[i],&tablaArchivos[i+1],sizeof(t_tablaGlobalArchivos));
						}
					}
				CANTTABLAARCHIVOS--;
				tablaArchivos=realloc(tablaArchivos,CANTTABLAARCHIVOS*sizeof(t_tablaGlobalArchivos));
				pthread_mutex_unlock(&mutexTablaArchivos);
				updatePCB(pcb);
				enviarDinamico(PCB,dataDePlanificacion->socket,pcb);
				free(cerrarArchivo);
			break;
			case MOVERCURSOR: // CPU MUEVE EL CURSOR DE UN ARCHIVO
				moverCursor=malloc(sizeof(t_moverCursor));
				recibirDinamico(MOVERCURSOR,dataDePlanificacion->socket,moverCursor);
				recibirDinamico(PCB,dataDePlanificacion->socket,pcb);
				pcb->privilegiadasEjecutadas++;
				pcb->referenciaATabla[(moverCursor->descriptorArchivo)-3].cursor=moverCursor->posicion;
				updatePCB(pcb);
				enviarDinamico(PCB,dataDePlanificacion->socket,pcb);
				free(moverCursor);
			break;
			case ESCRIBIRARCHIVO: // CPU ESCRIBE EN UN ARCHIVO
				escribirArchivo=malloc(sizeof(t_escribirArchivo));
				recibirDinamico(ESCRIBIRARCHIVO,dataDePlanificacion->socket,escribirArchivo);
				recibirDinamico(PCB,dataDePlanificacion->socket,pcb);
				pcb->privilegiadasEjecutadas++;
				if (escribirArchivo->fdArchivo==1)
				{
					mensaje=malloc(sizeof(t_mensaje));
					mensaje->mensaje=calloc(1,escribirArchivo->tamanio);
					mensaje->tamanio=escribirArchivo->tamanio;
					mensaje->mensaje=escribirArchivo->informacion;
					enviarDinamico(MENSAJE,SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],mensaje);
					free(mensaje);
				}
				else{
					if (pcb->referenciaATabla[(escribirArchivo->fdArchivo)-3].flags.escritura)
					{
						escribirArchivoFS=malloc(sizeof(t_escribirArchivoFS));
						escribirArchivoFS->size=escribirArchivo->tamanio;
						escribirArchivoFS->buffer=calloc(1,escribirArchivoFS->size);
						memcpy(escribirArchivoFS->buffer,escribirArchivo->informacion,escribirArchivoFS->size);
						escribirArchivoFS->offset=pcb->referenciaATabla[(escribirArchivo->fdArchivo)-3].cursor;
						globalFd=pcb->referenciaATabla[(escribirArchivo->fdArchivo)-3].globalFd;
						for (i = 0; i < CANTTABLAARCHIVOS; i++)
						{
							if (globalFd==tablaArchivos[i].fd)
							{
								break;
							}
						}
						escribirArchivoFS->tamanioPath=strlen(tablaArchivos[i].path);
						escribirArchivoFS->path=calloc(1,escribirArchivoFS->tamanioPath);
						memcpy(escribirArchivoFS->path,tablaArchivos[i].path,escribirArchivoFS->tamanioPath);
						enviarDinamico(GUARDARDATOSFS,SOCKETFS,escribirArchivoFS);
						while(0>recv(SOCKETFS,&rv,sizeof(int),0));
						if (!rv)
						{
							error=-2;
						}
						free(escribirArchivoFS);
					}
					else{
						error=-4;
					}
				}
				updatePCB(pcb);
				enviarDinamico(PCB,dataDePlanificacion->socket,pcb);
				free(escribirArchivo);
			break;
			case LEERARCHIVO: // CPU OBTIENE CONTENIDO DEL ARCHIVO
				leerArchivo=malloc(sizeof(t_leerArchivo));
				recibirDinamico(LEERARCHIVO,dataDePlanificacion->socket,leerArchivo);
				recibirDinamico(PCB,dataDePlanificacion->socket,pcb);
				pcb->privilegiadasEjecutadas++;
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
					leerArchivoFS->path=calloc(1,leerArchivoFS->tamanioPath);
					memcpy(leerArchivoFS->path,tablaArchivos[i].path,leerArchivoFS->tamanioPath);
					enviarDinamico(OBTENERDATOSFS,SOCKETFS,leerArchivoFS);
					while(0>recv(SOCKETFS,&rv,sizeof(int),0));
					if (!rv)
					{
						error=-3;
					}
					else{
						paqueteFS=malloc(sizeof(t_paqueteFS));
						recibirDinamico(PAQUETEFS,SOCKETFS,paqueteFS);
						enviarDinamico(PAQUETEFS,dataDePlanificacion->socket,paqueteFS);
						free(paqueteFS);
					}
					free(leerArchivoFS);
				}
				else{
					error=-4;
				}
				updatePCB(pcb);
				enviarDinamico(PCB,dataDePlanificacion->socket,pcb);
				free(leerArchivo);
			break;
			case PCBFINALIZADO: //FINALIZACION CORRECTA
						// RECIBO EL PCB
						recibirDinamico(PCBFINALIZADO,dataDePlanificacion->socket,pcb);
						while(0>recv(dataDePlanificacion->socket,&rv,sizeof(int),0));
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
						mensaje->mensaje=calloc(1,mensaje->tamanio);
						mensaje->mensaje=aux;
						enviarDinamico(MENSAJE,SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],mensaje);
						// free(mensaje->mensaje);
						free(mensaje);
						free(aux);
			break;
			case PCBFINALIZADOPORCONSOLA:
				// RECIBO EL PCB
						recibirDinamico(PCBFINALIZADOPORCONSOLA,dataDePlanificacion->socket,pcb);
						pcb->rafagasEjecutadas++;
						updatePCB(pcb);
						while(0>recv(dataDePlanificacion->socket,&rv,sizeof(int),0));
						if (rv)
						{
							PLANIFICACIONHABILITADA=0;
						}
						primerAcceso=1;
						seleccionador->tipoPaquete=PCB;
			break;
			case PCBERROR:
				// RECIBO EL PCB
			printf("gola\n");
						recibirDinamico(PCBERROR,dataDePlanificacion->socket,pcb);
						printf("despue recibi\n");
						pcb->rafagasEjecutadas++;
						updatePCB(pcb);
						printf("veamo\n");
						finalizarPid(pcb->pid,error);
						printf("ya que estamo\n");
						error=0;
						printf("llegue astak\n");
						while(0>recv(dataDePlanificacion->socket,&rv,sizeof(int),0));
						if (rv)
						{
							PLANIFICACIONHABILITADA=0;
						}
						primerAcceso=1;
						seleccionador->tipoPaquete=PCB;
			break;
			case PAGINAINVALIDA:
						error=-5;
						aux=calloc(1,100);
						sprintf(aux,"Finalizo por peticion de una pagina invalida el proceso PID=%i",pid);
						mensaje=malloc(sizeof(t_mensaje));
						mensaje->tamanio=strlen(aux);
						mensaje->mensaje=calloc(1,mensaje->tamanio);
						mensaje->mensaje=aux;
						enviarDinamico(MENSAJE,SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],mensaje);
						// free(mensaje->mensaje);
						free(mensaje);
						free(aux);
			break;
			case STACKOVERFLOW:
						error=-5;
						aux=calloc(1,100);
						printf("error detecta3\n");
						sprintf(aux,"Finalizo por stack overflow el proceso PID=%i",pid);
						mensaje=malloc(sizeof(t_mensaje));
						mensaje->tamanio=strlen(aux);
						mensaje->mensaje=calloc(1,mensaje->tamanio);
						mensaje->mensaje=aux;
						enviarDinamico(MENSAJE,SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],mensaje);
						// free(mensaje->mensaje);
						free(mensaje);
						free(aux);
			break;
			case PCBQUANTUM:
				recibirDinamico(PCBQUANTUM,dataDePlanificacion->socket,pcb);
				cambiarEstado(pcb->pid,READY);
				pthread_mutex_lock(&mutexColaReady);
				CANTIDADREADYS++;
				COLAREADY=realloc(COLAREADY,CANTIDADREADYS*sizeof(int));
				COLAREADY[CANTIDADREADYS-1]=_pid;
				pthread_mutex_unlock(&mutexColaReady);
				pcb->rafagasEjecutadas++;
				updatePCB(pcb);
				primerAcceso=1;
				seleccionador->tipoPaquete=PCB;
				while(0>recv(dataDePlanificacion->socket,&rv,sizeof(int),0));
				if (rv)
				{
					PLANIFICACIONHABILITADA=0;
				}
			break;
			case PCBBLOQUEADO:
				recibirDinamico(PCBQUANTUM,dataDePlanificacion->socket,pcb);
				cambiarEstado(pcb->pid,BLOCKED);
				pcb->rafagasEjecutadas++;
				updatePCB(pcb);
				primerAcceso=1;
				seleccionador->tipoPaquete=PCB;
				while(0>recv(dataDePlanificacion->socket,&rv,sizeof(int),0));
				if (rv)
				{
					PLANIFICACIONHABILITADA=0;
				}
			break;
		}
	}
	// LIBERO MEMORIA
	free(dataDePlanificacion);
	free(pcb);
}

void comunicarse(dataParaComunicarse * dataDeConexion){
	t_solicitudMemoria * respuestaSolicitud;
	t_path * path;
	int PidAFinalizar;
	t_metadata_program * metadata;
	t_resultadoIniciarPrograma * resultado;
	int socket=dataDeConexion->socket;
	int interfaz=dataDeConexion->interfaz;
	free(dataDeConexion);
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
    dataParaComunicarse * dataDePlanificacion;
	// CICLO PRINCIPAL
	while(COMUNICACIONHABILITADA && estaComunicacion){
		// RECIBO EL SELECCIONADOR (SI ES CPU, DIRECTAMENTE LE MANDO PCBS)
		seleccionador=malloc(sizeof(t_seleccionador));
		if (interfaz==CPU)
			seleccionador->tipoPaquete=SOLICITUDPCB;
		else
			while(0>recv(socket, seleccionador, sizeof(t_seleccionador), 0));
		switch(seleccionador->tipoPaquete){
				case PATH:	// RECIBIMOS EL PATH DE UN PROGRAMA ANSISOP A EJECUTAR Y SU PID
					// RECIBO EL PATH
					path = malloc (sizeof(t_path));
					recibirDinamico(PATH, socket, path);
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
					printf("path->path: %s.\n", path->path);
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
					printf("metadata->etiquetas_size: %i.\n", metadata->etiquetas_size);
					printf("metadata->etiquetas: %s.\n",metadata->etiquetas);
					printf("metadata->cantidad_de_etiquetas: %i.\n",metadata->cantidad_de_etiquetas);
					
					pthread_mutex_lock(&mutexAlocar);
					CANTTABLAHEAP++;
					tablaHeap[CANTTABLAHEAP-1].pid=pcb->pid;
					tablaHeap[CANTTABLAHEAP-1].cantPaginas=0;
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
					PCBS[CANTIDADPCBS-1]=*pcb;
					pthread_mutex_unlock(&mutexPcbs);
					// SOLICITUD DE MEMORIA
					solicitudMemoria=calloc(1,sizeof(t_solicitudMemoria));
					solicitudMemoria->tamanioCodigo=path->tamanio;
					solicitudMemoria->codigo=path->path;
					solicitudMemoria->cantidadPaginasCodigo=cantPaginasCodigo;
					solicitudMemoria->cantidadPaginasStack=STACK_SIZE;
					solicitudMemoria->pid=pid;
					enviarDinamico(SOLICITUDMEMORIA,SOCKETMEMORIA,solicitudMemoria);
					//LIBERO MEMORIA
					free(path);
					free(solicitudMemoria);
					free(pcb);
					// RECIBO LA RESPUESTA DE MEMORIA
					while(0>recv(SOCKETMEMORIA, seleccionador, sizeof(t_seleccionador), 0));
					respuestaSolicitud=malloc(sizeof(t_solicitudMemoria));
					recibirDinamico(SOLICITUDMEMORIA,SOCKETMEMORIA,respuestaSolicitud);
					// PREPARO LA RESPUESTA A CONSOLA
					resultado=malloc(sizeof(t_resultadoIniciarPrograma));
					pid = respuestaSolicitud->pid;
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
						COLAEXIT = realloc (COLAEXIT,(CANTIDADEXITS) * sizeof(COLAEXIT[0]));
						COLAEXIT[CANTIDADEXITS-1]=pid;
						pthread_mutex_unlock(&mutexColaExit);
						pthread_mutex_lock(&mutexPcbs);
						PCBS[pid].exitCode=-1;
						pthread_mutex_unlock(&mutexPcbs);
						cambiarEstado(pid,EXIT);
						resultado->resultado=1;
					}
					// ENVIO RESPUESTA A CONSOLA
					enviarDinamico(RESULTADOINICIARPROGRAMA,SOCKETSCONSOLAMENSAJE[SOCKETSCONSOLA[pid]],resultado);
					// LIBERO MEMORIA
					free(respuestaSolicitud);
					free(resultado);
				break;
				case FINALIZARPROGRAMA: // RECIBIMOS EL PID DE UN PROGRAMA ANSISOP A FINALIZAR
					recibirDinamico(FINALIZARPROGRAMA,socket,&PidAFinalizar);
					finalizarPid(PidAFinalizar,-7);
				break;
				case ARRAYPIDS: 
					arrayPids=malloc(sizeof(t_arrayPids));
					recibirDinamico(ARRAYPIDS,socket,arrayPids);
					for (i = 0; i < arrayPids->cantidad; i++)
					{
						finalizarPid(arrayPids->pids[i],-6);
					}
				break;
				case SOLICITUDPCB:
					// INICIO DE HILO PLANIFICADOR
					dataDePlanificacion=malloc(sizeof(dataParaComunicarse));
        			dataDePlanificacion->socket=socket;
        			pthread_create(&hiloPlanificacion,NULL,(void *)planificar,dataDePlanificacion);
					estaComunicacion=0;
        		break;
        		case SOLICITUDMEMORIA:
        			
        		break;
		    }
		free(seleccionador);
	}
}

void aceptar(dataParaComunicarse * dataParaAceptar){
	// VARIABLES PARA LAS CONEXIONES ENTRANTES
	int * interfaz;
	int flag=0;
	pthread_t hiloComunicador;
	dataParaComunicarse * dataParaConexion;
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
			interfaz = malloc(sizeof(int));
			// ME INFORMO SOBRE LA INTERFAZ QUE SE CONECTÓ
			handshakeServer(socketNuevaConexion,KERNEL,interfaz);
			if (*interfaz==CPU && primerIngresoCpu)
			{
				pthread_t hiloRellenarColaReady;
				pthread_create(&hiloRellenarColaReady,NULL,(void *)rellenarColaReady,NULL);
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
			}
			// CONFIGURACION E INICIO DE HILO 
			if(!flag) {
						dataParaConexion = malloc(sizeof(dataParaComunicarse));
						dataParaConexion->interfaz=*interfaz; 
						dataParaConexion->socket=socketNuevaConexion;
						pthread_create(&hiloComunicador,NULL,(void *)comunicarse,dataParaConexion);
						}
			else flag=0;
						free(interfaz);
		}
	}
	// LIBERO MEMORIA
	free(dataParaAceptar);
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
int * buffer=malloc(sizeof(int));
if ((rv = connect(socketMemoria,memoria->ai_addr,memoria->ai_addrlen)) == -1) 
	perror("No se pudo conectar con memoria.\n");
else if (rv == 0){
	printf("Se conectó con memoria correctamente.\n");
	handshakeCliente(socketMemoria,KERNEL,buffer);
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
if ((rv = connect(socketFS,fs->ai_addr,fs->ai_addrlen)) == -1) 
	perror("No se pudo conectar con filesystem.\n");
else if (rv == 0){
	printf("Se conectó con filesystem correctamente.\n");
	handshakeCliente(socketFS,KERNEL,buffer);
	free(buffer);
}
SOCKETFS=socketFS;
freeaddrinfo(fs);
// CONFIGURACION DEL SERVIDOR
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
dataParaComunicarse *dataParaAceptar;
dataParaAceptar=malloc(sizeof(dataParaComunicarse));
dataParaAceptar->socket = socketEscuchador;
// INICIO DE HILO ACEPTADOR
pthread_t hiloAceptador;
pthread_create(&hiloAceptador,NULL,(void *)aceptar,dataParaAceptar);
// INICIO DE HILO DE CONSOLA
pthread_t hiloConsola;
pthread_create(&hiloConsola,NULL,(void *)consola,NULL);
// ESPERO FIN DE HILO CONSOLA
pthread_join(hiloConsola,NULL);
//LIBERO MEMORIA
free(COLAREADY);
free(COLANEW);
free(COLAEXEC);
free(COLABLOCK);
free(COLAEXIT);
free(SOCKETSCONSOLA);
return 0;
}