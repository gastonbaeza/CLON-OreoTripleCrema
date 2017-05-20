#ifndef _estructuras_h
#define _estructuras_h



typedef struct {
				int codigo;
				char * descripcion;
				} t_exitCode;

typedef struct {
				int ocupado; // 1=ocupado ; 0=libre
				int bytes;
				} hashMemoria;

typedef struct 
				{char * codigo;
				 void * chain;		
					
				}t_chain;
typedef struct { 
				int marco;
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
				 int interfaz;				
				} dataParaComunicarse;


typedef struct {
				int pid;
				int estado;
				int programCounter;
				int referenciaATabla;
				int posicionStack;
				int indiceCodigo;
				int indiceEtiquetas;
				int exitCode;
				}t_pcb;

typedef struct { 
				void * primerPagina;
				t_pcb proceso;
				}t_estructuraADM;
typedef struct {
				char * archivo;// este archivo en realidad es un path
				char flag;
				}t_solicitudFS;

typedef struct { 
				int unaInterfaz;
				int tipoPaquete;
				} t_seleccionador;
typedef struct 
				{
				int pid;
				}t_solicitudInfoProg;

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
				int pid;
				int respuesta; 

				}t_solicitudMemoria;

typedef struct {
				char * linea;
				int tamanioLinea;
				}t_linea;
typedef struct 
				{
				int pid;
				int programCounter;
				int indiceStack;	

				}t_actualizacion;
typedef struct 
				{
					int socket;
					t_marco * asignadorSecuencial;
					t_marco * marcos;
	
				}t_shittyStructure;

typedef struct {
				int pid;
				int resultado; // 0 NO HAY MEMORIA - 1 TODO PIOLA
				} t_resultadoIniciarPrograma;

//funcion que retorna ok/exception al kernel ante el pedido de memoria



#endif