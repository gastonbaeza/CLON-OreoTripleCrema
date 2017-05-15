#ifndef _estructuras_h
#define _estructuras_h
// #include "desSerializador.h"
#define MENSAJE 0
#define PATH 1
#define TAMPAGINA 2
#define PIDFinalizacion 3
#define PIDINFO 4
#define BLOQUE  5
#define FLAGS 6
#define NOTIFICACION 7
#define EXCEPCION 8
#define OKMEMORIA 9
#define SYSCALL 10
#define FD 11
#define PAGPROGSTACK 12

#define INFOPROG 14
#define PCB 15
#define LEER 16
#define ESCRIBIR 17
#define LIMPIAR printf("\033[H\033[J")
#define KERNEL 0
#define MEMORIA 1
#define SOLICITUDMEMORIA 0
#define CODIGO 1
#define SOLICITUDINFOPROG 2
#define ESCRIBIRMEMORIA 3
#define LIBERARMEMORIA 4
#define CONSOLA 2
#define CPU 3
#define FS 4
#define READY 0
#define RUNNING 1
#define BLOCKED 2
#define LIBRE 1
#define OCUPADO 0

typedef struct {
				int codigo;
				char * descripcion;
				} t_exitCode;

typedef struct {
				int ocupado; // 1=ocupado ; 0=libre
				int bytes;
				} hashMemoria;
typedef struct { 
				int frame;
				int pid;
				int numeroPagina;
				}t_estructuraADM;
typedef struct 
				{char * codigo;
				 unsigned int chain;		
					
				}t_chain;
typedef struct { 
				int numeroMarco;
				void * numeroPagina[3];
				}t_marco;

typedef struct {
				char * path;
				int vecesAbierto;
				} t_tablaGlobalArchivos;

typedef struct {
				int descriptor;
				int flag;
				int posicionTablaglobal;
				} t_tablaArchivosDeProcesos;

typedef struct {
				char ** unPrograma; // el programa en si
				int  dimension; // cantidad de lineas
				} t_programa;

typedef struct { 
				char * elPrograma; 
				int tamanio; // bytes del programa
				} t_programaSalida;

typedef struct { int socket;
				
				} dataParaComunicarse;


typedef struct {
				int pid;
				int estado;
				int programCounter;
				int referenciaATabla;
				int posicionStack;
				t_exitCode exitCode;
				} pcb;

typedef struct {
				char * archivo;// este archivo en realidad es un path
				char flag;
				}t_solicitudFS;

typedef struct { 
				int unaInterfaz;
				int tipoPaquete;
				} t_seleccionador;

typedef struct {
				t_seleccionador seleccionador;
				int tamanio;
				} t_header;

typedef struct {
				char * excepcion;
				}t_excepcion;

typedef struct {
				t_programaSalida  codigo;
				int cantidadPaginasCodigo;
				int cantidadPaginasStack;
				}t_solicitudMemoria;


//funcion que retorna ok/exception al kernel ante el pedido de memoria



#endif