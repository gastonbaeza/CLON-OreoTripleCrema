#ifndef _estructuras_h
#define _estructuras_h




typedef struct {
				int ocupado; // 1=ocupado ; 0=libre
				int bytes;
				}__attribute__((packed))
				  hashMemoria;

typedef struct { 
				int marco;
				void * numeroPagina[3];
				}__attribute__((packed))
				 t_marco;

typedef struct {
				char * path;
				int vecesAbierto;
				int tamanioPath;
				}__attribute__((packed))
				  t_tablaGlobalArchivos;

typedef struct {
				int descriptor;
				int flag;
				int posicionTablaGlobal;
				}__attribute__((packed))
				  t_tablaArchivosDeProcesos;

typedef struct {
				char ** unPrograma; // el programa en si
				int  dimension; // cantidad de lineas
				}__attribute__((packed))
				  t_programa;

typedef struct { 
				char * elPrograma; 
				int tamanio; // bytes del programa
				}__attribute__((packed))
				t_programaSalida;

typedef struct { 
				int tamanio; // bytes del programa
				char * path; 
				}__attribute__((packed))
				t_path;

typedef struct { 
				int tamanio; // bytes del programa
				char * mensaje; 
				}__attribute__((packed))
				t_mensaje;

typedef struct { 
				int socket;
				int interfaz;				
				}__attribute__((packed))
				  dataParaComunicarse;


typedef struct {
				int pid;
				int programCounter;
				int estado;
				int referenciaATabla;
				int paginasCodigo; // estaba en el TP pero no lo habiamos puesto
				int posicionStack;
				int indiceCodigo;
				int indiceEtiquetas;
				int exitCode;
				}__attribute__((packed))
				 t_pcb;

typedef struct { 
				int frame;
				int pid;
				int hashPagina;
				int estado;
				}t_estructuraADM;

typedef struct { 
				int unaInterfaz;
				int tipoPaquete;

				}__attribute__((packed))
				  t_seleccionador;
typedef struct {
				
				int tamanioCodigo;
				char * codigo;
				int cantidadPaginasCodigo;
				int cantidadPaginasStack;
				int pid;
				int respuesta; 

				}__attribute__((packed))
				 t_solicitudMemoria;

typedef struct {
				char * linea;
				int tamanioLinea;
				}__attribute__((packed))
				 t_linea;
typedef struct 
				{
				int pid;
				int programCounter;
				int indiceStack;	
				}__attribute__((packed))
				 t_actualizacion;
typedef struct {
				int pid;
				int resultado; // 0 NO HAY MEMORIA - 1 TODO PIOLA
				}__attribute__((packed))
				  t_resultadoIniciarPrograma;

//funcion que retorna ok/exception al kernel ante el pedido de memoria



#endif