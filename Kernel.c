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
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <pthread.h>
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
	#define SOLICITUDPCB 0
	#define PCB 1
	#define ESCRIBIR 2
	#define PIDFINALIZARPROCESO 3
#define FS 4
#define TRUE 1
#define FALSE 0
#define OK 1
#define FAIL 0
#define BLOQUE 5
#define NEW 0
#define READY 1
#define EXEC 2
#define BLOCK 3
#define EXIT 4
// VARIABLES GLOBALES
char * ALGORITMO;
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

t_pcb * PCBS;
int CANTIDADPCBS=0;

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
	int i=0;
	for (i; i < GRADO_MULTIPROG; i++)
		if (COLAREADY[i]!=-1)
			i=1;
	return i;
}

void getPcbAndRemovePid(int pid, t_pcb * pcb){
	int i=0;
	//BUSCO EL PCB
	pcb=&PCBS[pid];
	// LO SACO DE LA COLA DE READYS
	for (i; i < GRADO_MULTIPROG; i++){
		if (i+1==GRADO_MULTIPROG) {
			pthread_mutex_lock(&mutexColaReady);
			COLAREADY[i]=-1;
			pthread_mutex_unlock(&mutexColaReady);
		}
		else {
			pthread_mutex_lock(&mutexColaReady);
			COLAREADY[i]=COLAREADY[i+1];
			pthread_mutex_unlock(&mutexColaReady);
		}
	}
}

void rellenarColaReady(){
	int i=0,cantVacios=0;
	for (; i < GRADO_MULTIPROG; i++)
		if (COLAREADY[i]==-1)
			cantVacios++;
	int primerReadyLibre;
	primerReadyLibre=GRADO_MULTIPROG-cantVacios;
	for (i=0; i<cantVacios; i++){
		pthread_mutex_lock(&mutexColaReady);
		COLAREADY[primerReadyLibre+i]=COLANEW[0];
		pthread_mutex_unlock(&mutexColaReady);
		pthread_mutex_lock(&mutexColaNew);
		CANTIDADNEWS--;
		COLANEW+=sizeof(int);
		COLANEW=realloc(COLANEW,CANTIDADNEWS*sizeof(COLANEW[0]));
		pthread_mutex_unlock(&mutexColaNew);
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

int finalizarPid(int pid){
	int i=0, index=0;
	while(i<CANTIDADPCBS && PCBS[i].pid!=pid)
		i++;
	if (i==CANTIDADPCBS)
		return 0;
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
			for (i=0; i < GRADO_MULTIPROG; i++){
				if (i+1==GRADO_MULTIPROG) {
					pthread_mutex_lock(&mutexColaReady);
					COLAREADY[i]=-1;
					pthread_mutex_unlock(&mutexColaReady);
				}
				else {
					pthread_mutex_lock(&mutexColaReady);
					COLAREADY[i]=COLAREADY[i+1];
					pthread_mutex_unlock(&mutexColaReady);
				}
			}
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
		case BLOCK:
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
	PCBS[i].exitCode=-6;
	PCBS[i].estado=EXIT;
	pthread_mutex_unlock(&mutexPcbs);
	// LO AGREGO A LA COLA DE EXIT
	pthread_mutex_lock(&mutexColaExit);
	if (CANTIDADEXITS % BLOQUE == 0)
		COLAEXIT = realloc (COLAEXIT,(CANTIDADEXITS+BLOQUE) * sizeof(COLAEXIT[0]));
	COLAEXIT[CANTIDADEXITS]=pid;
	CANTIDADEXITS++;
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

void consola(){
	int consolaHabilitada=1;
	int opcion,opcion2;
	int pid;
	int i;
	printf("\nPresione enter para iniciar la consola.\n");
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
						while(COLAREADY[i]!=-1)
							i++;
						if (i==0)
							printf("Cola ready vacía.\n");
						else{
							printf("Cola ready:\n");
							i=0;
							while(COLAREADY[i]!=-1){
								printf("\t%i\n",COLAREADY[i]);
								i++;
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
								printf("\t%i\n",COLAEXEC[i]);
						}
						if (CANTIDADEXITS==0)
							printf("Cola exit vacía.\n");
						else{
							printf("Cola exit:\n");
							for (i = 0; i < CANTIDADEXITS; i++)
								printf("\t%i\n",COLAEXEC[i]);
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
						while(COLAREADY[i]!=-1)
							i++;
						if (i==0)
							printf("Cola ready vacía.\n");
						else{
							printf("Cola ready:\n");
							i=0;
							while(COLAREADY[i]!=-1){
								printf("\t%i\n",COLAREADY[i]);
								i++;
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
								printf("\t%i\n",COLAEXEC[i]);
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
								printf("\t%i\n",COLAEXEC[i]);
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
					printf("Proceso %i:\n", pid);
					printf("\tEstado: %i\n", PCBS[i].estado);
					printf("\tPC: %i\n", PCBS[i].programCounter);
					printf("\tReferencia a tabla de archivos: %i\n", PCBS[i].referenciaATabla);
					printf("\tPosicion stack: %i\n", PCBS[i].posicionStack);
					printf("\tIndice Codigo: %i\n", PCBS[i].indiceCodigo);
					printf("\tIndice Etiquetas: %i\n", PCBS[i].indiceEtiquetas);
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
				if (finalizarPid(pid))
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
		}
	}
}

void planificar(dataParaComunicarse * dataDePlanificacion){
	int pid;
	t_pcb * pcb;
	pcb=malloc(sizeof(t_pcb));
	int cpuLibre = 1;
	t_header * header;
	header=malloc(sizeof(t_header));
	while(PLANIFICACIONHABILITADA){
		while(PLANIFICACIONPAUSADA){} // SI SE PAUSA LA PLANIFICACION QUEDO LOOPEANDO ACA
		// VERIFICO QUE SIGA EN LA COLA DE EXEC
		if (!estaExec(pid))
			cpuLibre=1;
		rellenarColaReady();
		if (cpuLibre)
		{
			if (hayProcesosReady()){
					// OBTENGO EL PRIMER PID DE LA COLA DE LISTOS
					pid = COLAREADY[0];
					// LO SACO DE DICHA COLA Y OBTENGO SU PCB
					getPcbAndRemovePid(pid,pcb);
					// LO PONGO EN LA COLA DE EJECUTANDO
					pthread_mutex_lock(&mutexColaExec);
					if (CANTIDADEXECS % BLOQUE == 0)
						COLAEXEC = realloc (COLAEXEC,(CANTIDADEXECS+BLOQUE) * sizeof(COLAEXEC[0]));
					COLAEXEC[CANTIDADEXECS]=pid;
					CANTIDADEXECS++;
					pthread_mutex_unlock(&mutexColaExec);
					// CAMBIO ESTADO A EJECUTANDO
					cambiarEstado(pid,EXEC);
					// LE MANDO EL PCB AL CPU
					enviarDinamico(KERNEL,PCB,dataDePlanificacion->socket,pcb,sizeof(t_pcb));
					cpuLibre=0;
			}
		}
		else{
			recv(dataDePlanificacion->socket, header, sizeof(t_header), 0);
			switch(header->seleccionador.tipoPaquete){
				case PIDFINALIZARPROCESO: //FINALIZACION CORRECTA
					// RECIBO EL PCB
					recibirDinamico(dataDePlanificacion->socket,pcb,sizeof(header->tamanio));
					pcb->exitCode=0;
					// LO AGREGO A LA COLA EXIT
					pthread_mutex_lock(&mutexColaExit);
					if (CANTIDADEXITS % BLOQUE == 0)
						COLAEXIT = realloc (COLAEXIT,(CANTIDADEXITS+BLOQUE) * sizeof(COLAEXIT[0]));
					COLAEXIT[CANTIDADEXITS]=pid;
					CANTIDADEXITS++;
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
				break;
				case ESCRIBIR:
				break;
			}
		cpuLibre=1;
		}
	}
	// LIBERO MEMORIA
	free(pcb);
	free(dataDePlanificacion);
	free(header);
}

void comunicarse(dataParaComunicarse * dataDeConexion){
	// RETURN VALUES
	int pid;
	int rv;
	// NUMERO DE BYTES
	int nbytes;
	// BUFFER RECEPTOR
	void * paquete;
	// JOBS
	int * jobs;
	int cantidadJobs=0;
	// FLAG DE ESTE HILO
	int estaComunicacion=1;
	// HEADERS
	t_header * header;
	header = malloc(sizeof(t_header));
	// HILO PLANIFICADOR
	pthread_t hiloPlanificacion;
	// DATA PARA PLANIFICACION EN CASO DE NECESITARSE
    dataParaComunicarse * dataDePlanificacion;
	// CICLO PRINCIPAL
	while(COMUNICACIONHABILITADA && estaComunicacion){
		// RECIBO EL HEADER (SI ES CPU, DIRECTAMENTE LE MANDO PCBS)
		if (dataDeConexion->interfaz==CPU)
		{
			header->seleccionador.unaInterfaz=CPU;
			header->seleccionador.tipoPaquete=SOLICITUDPCB;
		}
		else
			nbytes = recv(dataDeConexion->socket, header, sizeof(t_header), 0);
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
						pid = ULTIMOPID;
						pthread_mutex_lock(&mutexPid);
						ULTIMOPID++;
						pthread_mutex_unlock(&mutexPid);
						// GUARDO SOCKET DE LA CONSOLA INDEXADO CON EL PID
						pthread_mutex_lock(&mutexSocketsConsola);
						if (CANTIDADSOCKETSCONSOLA%BLOQUE==0)
							SOCKETSCONSOLA = realloc (SOCKETSCONSOLA,(CANTIDADSOCKETSCONSOLA+BLOQUE) * sizeof(SOCKETSCONSOLA[0]));
						SOCKETSCONSOLA[CANTIDADSOCKETSCONSOLA]=dataDeConexion->socket;
						CANTIDADSOCKETSCONSOLA++;
						pthread_mutex_unlock(&mutexSocketsConsola);
						// RECUPERO EL PROGRAMA DEL PATH
						t_programaSalida * programa;
						programa= obtenerPrograma(path);
						// CALCULO LA CANTIDAD DE PAGINAS
						int cantPaginasCodigo = calcularPaginas(TAMPAGINA,programa->tamanio);
						int cantPaginasStack = calcularPaginas(TAMPAGINA,STACK_SIZE);
						// CREO EL PCB
						t_pcb * pcb;
						pcb=malloc(sizeof(t_pcb));
						pcb->pid = pid;
						pcb->estado = NEW;
						pcb->programCounter=0;
						pcb->posicionStack=0;
						pcb->indiceCodigo=0;
						pcb->indiceEtiquetas=0;
						pcb->exitCode=1;
						// LO AGREGO A LA TABLA
						pthread_mutex_lock(&mutexPcbs);
						if (CANTIDADPCBS%BLOQUE==0)
							PCBS = realloc (PCBS,(CANTIDADPCBS+BLOQUE) * sizeof(PCBS[0]));
						PCBS[CANTIDADPCBS]=*pcb;
						CANTIDADPCBS++;
						pthread_mutex_unlock(&mutexPcbs);
						// SOLICITUD DE MEMORIA
						t_solicitudMemoria * solicitudMemoria;
						solicitudMemoria=malloc(sizeof(t_solicitudMemoria));
						solicitudMemoria->codigo=*programa;
						solicitudMemoria->cantidadPaginasCodigo=cantPaginasCodigo;
						solicitudMemoria->cantidadPaginasStack=cantPaginasStack;
						solicitudMemoria->pid=pid;
						enviarDinamico(KERNEL,SOLICITUDMEMORIA,SOCKETMEMORIA,solicitudMemoria,sizeof(t_solicitudMemoria));
						// LO AGREGO A LA LISTA DE JOBS
						if (cantidadJobs % BLOQUE == 0)
							jobs = realloc (jobs, (cantidadJobs+BLOQUE) * sizeof(jobs[0]));
						jobs[cantidadJobs]=pid;
						cantidadJobs++;
						//LIBERO MEMORIA
						free(path);
						free(solicitudMemoria);
						free(pcb);
					break;
					case FINALIZARPROGRAMA: // RECIBIMOS EL PID DE UN PROGRAMA ANSISOP A FINALIZAR
						
					break;
					case DESCONECTARCONSOLA: // SE DESCONECTA ESTA CONSOLA
					break;
		        }
		    break;
        	case CPU:
        		switch(header->seleccionador.tipoPaquete){
        			case SOLICITUDPCB:
						// INICIO DE HILO PLANIFICADOR
        				dataDePlanificacion=malloc(sizeof(dataParaComunicarse));
        				dataDePlanificacion=dataDeConexion;
						pthread_create(&hiloPlanificacion,NULL,(void *)planificar,dataDeConexion);
						estaComunicacion=0;
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
						// VERIFICO SI PUEDO PASAR A NEW
						if (respuestaSolicitud->respuesta==OK){
							// AGREGO EL PID A LA COLA DE NEWS
							pthread_mutex_lock(&mutexColaNew);
							if (CANTIDADNEWS % BLOQUE == 0)
								COLANEW = realloc (COLANEW,(CANTIDADNEWS+BLOQUE) * sizeof(COLANEW[0]));
							COLANEW[CANTIDADNEWS]=pid;
							CANTIDADNEWS++;
							pthread_mutex_unlock(&mutexColaNew);
							resultado->resultado=1;
						}
						else if (respuestaSolicitud->respuesta==FAIL){
						 	resultado->resultado=0;
						 	// AGREGO EL PID A LA COLA DE EXIT
							pthread_mutex_lock(&mutexColaExit);
							if (CANTIDADEXITS % BLOQUE == 0)
								COLAEXIT = realloc (COLAEXIT,(CANTIDADEXITS+BLOQUE) * sizeof(COLAEXIT[0]));
							COLAEXIT[CANTIDADEXITS]=pid;
							CANTIDADEXITS++;
							pthread_mutex_unlock(&mutexColaExit);
							pthread_mutex_lock(&mutexPcbs);
							PCBS[pid].exitCode=-1;
							pthread_mutex_unlock(&mutexPcbs);
							cambiarEstado(pid,EXIT);
							resultado->resultado=1;
						}
						// ELIMINO EL PID DE MI COLA DE JOBS
						int i=0;
						while(jobs[i]!=pid)
							i++;
						for (i; i < cantidadJobs; i++){
							if (i+1==cantidadJobs)
								jobs[i]=-1;
							else
								jobs[i]=jobs[i+1];
						}
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

void aceptar(dataParaComunicarse * dataParaAceptar){
	// VARIABLES PARA LAS CONEXIONES ENTRANTES
	int * interfaz;
	interfaz = malloc(sizeof(int));
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
			// ME INFORMO SOBRE LA INTERFAZ QUE SE CONECTÓ
			handshakeServer(socketNuevaConexion,KERNEL,interfaz);
			// CONFIGURACION E INICIO DE HILO COMUNICADOR
			dataParaConexion = malloc(sizeof(dataParaComunicarse));
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
QUANTUM= config_get_int_value(CFG ,"QUANTUM");
QUANTUM_SLEEP= config_get_int_value(CFG ,"QUANTUM_SLEEP");
ALGORITMO= config_get_string_value(CFG ,"ALGORITMO");
GRADO_MULTIPROG= config_get_int_value(CFG ,"GRADO_MULTIPROG");
char* SEM_IDS= config_get_string_value(CFG ,"SEM_IDS");
char* SEM_INIT= config_get_string_value(CFG ,"SEM_INIT");
char* SHARED_VARS= config_get_string_value(CFG ,"SHARED_VARS");
STACK_SIZE= config_get_int_value(CFG ,"STACK_SIZE");
system("clear");
printf("Configuración:\nPUERTO_PROG = %s,\nPUERTO_CPU = %s,\nIP_MEMORIA = %s,\nPUERTO_MEMORIA = %s,\nIP_FS = %s,\nPUERTO_FS = %s,\nQUANTUM = %i,\nQUANTUM_SLEEP = %i,\nALGORITMO = %s,\nGRADO_MULTIPROG = %i,\nSEM_IDS = %s,\nSEM_INIT = %s,\nSHARED_VARS = %s,\nSTACK_SIZE = %i.\n"
		,PUERTO_PROG,PUERTO_CPU,IP_MEMORIA,PUERTO_MEMORIA,IP_FS,PUERTO_FS,QUANTUM,QUANTUM_SLEEP,ALGORITMO,GRADO_MULTIPROG,SEM_IDS,SEM_INIT,SHARED_VARS,STACK_SIZE);
printf("\nPresione enter para continuar.\n");
getchar();
system("clear");
/* LEER CONFIGURACION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
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