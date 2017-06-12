#include "estructuras.h"
#include "funciones.h"
#include <parser/parser.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/txt.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <parser/metadata_program.h>
#include <stddef.h>



#define SOLICITUDLINEA 2
#define FINALIZARPROCESO 16
#define PCB 17
#define CPU 1
#define SOLICITUDBYTES 31
#define ABRIRARCHiVO 32
#define ABRIOARCHIVO 33
#define ESCRIBIRARCHIVO 34
#define LEERARCHIVO 35
#define ALMACENARBYTES 36
#define SOLICITUDVALORVARIABLE 37
#define ASIGNARVARIABLECOMPARTIDA 38
#define SOLICITUDSEM 39
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
#define LINEA 19
#define BLOQUE 5
#define SIZE 4

int continuarEjecucion=1;
int PID;
int i;
t_pcb * pcb;
char * IP_KERNEL;
char * PUERTO_KERNEL;
char * IP_MEMORIA;
char * PUERTO_MEMORIA;
pthread_mutex_t mutexPcb;
int PROXIMAPAG=0;
int PROXIMOOFFSET=0;
int TAMPAGINA;



int socketKernel;
int socketMemoria;



void nuevoNivelStack(){
	pthread_mutex_lock(&mutexPcb);
	pcb->cantidadStack++;
	pcb->indiceStack=realloc(pcb->indiceStack,pcb->cantidadStack*sizeof(t_stack));
	pcb->posicionStack++;
	pcb->indiceStack[pcb->posicionStack].cantidadArgumentos=0;
	pcb->indiceStack[pcb->posicionStack].cantidadVariables=0;
	pcb->indiceStack[pcb->posicionStack].varRetorno.pagina=-1;
	pcb->indiceStack[pcb->posicionStack].varRetorno.offset=-1;
	pcb->indiceStack[pcb->posicionStack].varRetorno.size=-1;
	pcb->indiceStack[pcb->posicionStack].posRetorno=pcb->programCounter;
	pthread_mutex_unlock(&mutexPcb);
}

void finalizarNivelStack(){
	pthread_mutex_lock(&mutexPcb);
	pcb->programCounter=pcb->indiceStack[pcb->posicionStack].posRetorno;
	pcb->cantidadStack--;
	pcb->indiceStack=realloc(pcb->indiceStack,pcb->cantidadStack*sizeof(t_stack));
	pcb->posicionStack--;
	pthread_mutex_unlock(&mutexPcb);
}


void liberarContenidoPcb(){
	free(pcb->indiceCodigo);
	free(pcb->indiceStack->argumentos);
	free(pcb->indiceStack->variables);
	free(pcb->indiceStack);
}

void posicionarPC(int pos){
	for (i = 0; i < pcb->cantidadInstrucciones; i++)
			{
				if (pos<=pcb->indiceCodigo[i].start)
				{
					pthread_mutex_lock(&mutexPcb);
					pcb->programCounter=i;
					pthread_mutex_unlock(&mutexPcb);
				}
			}
}


		/*
		 * DEFINIR VARIABLE
		 *
		 * Reserva en el Contexto de Ejecución Actual el espacio necesario para una variable llamada identificador_variable y la registra tanto en el Stack como en el Diccionario de Variables. Retornando la posición del valor de esta nueva variable del stack
		 * El valor de la variable queda indefinido: no deberá inicializarlo con ningún valor default.
		 * Esta función se invoca una vez por variable, a pesar que este varias veces en una línea.
		 * Ej: Evaluar "variables a, b, c" llamará tres veces a esta función con los parámetros "a", "b" y "c"
		 *
		 * @sintax	TEXT_VARIABLE (variables)
		 * 			-- nota: Al menos un identificador; separado por comas
		 * @param	identificador_variable	Nombre de variable a definir
		 * @return	Puntero a la variable recien asignada
		 */
		t_puntero cpu_definirVariable(t_nombre_variable identificador_variable){
			if (identificador_variable>=48 && identificador_variable<=57) // es un parametro
			{
				pthread_mutex_lock(&mutexPcb);
				pcb->indiceStack[pcb->posicionStack].cantidadArgumentos++;
				pcb->indiceStack[pcb->posicionStack].argumentos=realloc(pcb->indiceStack->argumentos,pcb->indiceStack[pcb->posicionStack].cantidadArgumentos*sizeof(t_argumento));
				pcb->indiceStack[pcb->posicionStack].argumentos[pcb->indiceStack[pcb->posicionStack].cantidadArgumentos-1].id=identificador_variable;
				pcb->indiceStack[pcb->posicionStack].argumentos[pcb->indiceStack[pcb->posicionStack].cantidadArgumentos-1].pagina=PROXIMAPAG;
				pcb->indiceStack[pcb->posicionStack].argumentos[pcb->indiceStack[pcb->posicionStack].cantidadArgumentos-1].offset=PROXIMOOFFSET;
				pcb->indiceStack[pcb->posicionStack].argumentos[pcb->indiceStack[pcb->posicionStack].cantidadArgumentos-1].size=SIZE;
				if (PROXIMOOFFSET+SIZE==TAMPAGINA)
				{
					PROXIMAPAG++;
					PROXIMOOFFSET=0;
				}
				else
				{
					PROXIMOOFFSET+=SIZE;
				}
				pthread_mutex_unlock(&mutexPcb);
				return (pcb->indiceStack[pcb->posicionStack].argumentos[pcb->indiceStack[pcb->posicionStack].cantidadArgumentos-1].pagina)*TAMPAGINA+(pcb->indiceStack[pcb->posicionStack].argumentos[pcb->indiceStack[pcb->posicionStack].cantidadArgumentos-1].offset);
			}
			else // es una variable
			{
				pthread_mutex_lock(&mutexPcb);
				pcb->indiceStack[pcb->posicionStack].cantidadVariables++;
				pcb->indiceStack[pcb->posicionStack].variables=realloc(pcb->indiceStack[pcb->posicionStack].variables,pcb->indiceStack[pcb->posicionStack].cantidadVariables*sizeof(t_variable));
				pcb->indiceStack[pcb->posicionStack].variables[pcb->indiceStack[pcb->posicionStack].cantidadVariables-1].id=identificador_variable;
				pcb->indiceStack[pcb->posicionStack].variables[pcb->indiceStack[pcb->posicionStack].cantidadVariables-1].pagina=PROXIMAPAG;
				pcb->indiceStack[pcb->posicionStack].variables[pcb->indiceStack[pcb->posicionStack].cantidadVariables-1].offset=PROXIMOOFFSET;
				pcb->indiceStack[pcb->posicionStack].variables[pcb->indiceStack[pcb->posicionStack].cantidadVariables-1].size=SIZE;
				if (PROXIMOOFFSET+SIZE==TAMPAGINA)
				{
					PROXIMAPAG++;
					PROXIMOOFFSET=0;
				}
				else
				{
					PROXIMOOFFSET+=SIZE;
				}
				pthread_mutex_unlock(&mutexPcb);
				return (pcb->indiceStack[pcb->posicionStack].variables[pcb->indiceStack[pcb->posicionStack].cantidadVariables-1].pagina)*TAMPAGINA+(pcb->indiceStack[pcb->posicionStack].variables[pcb->indiceStack[pcb->posicionStack].cantidadVariables-1].offset);
			}
		}

		/*
		 * OBTENER POSICION de una VARIABLE
		 *
		 * Devuelve el desplazamiento respecto al inicio del segmento Stack en que se encuentra el valor de la variable identificador_variable del contexto actual.
		 * En caso de error, retorna -1.
		 *
		 * @sintax	TEXT_REFERENCE_OP (&)
		 * @param	identificador_variable 		Nombre de la variable a buscar (De ser un parametro, se invocara sin el '$')
		 * @return	Donde se encuentre la variable buscada
		 */
		t_puntero cpu_obtenerPosicionVariable (t_nombre_variable identificador_variable){
			int i;
			if (identificador_variable>=48 && identificador_variable<=57) // es un parametro
			{
				return (pcb->indiceStack[pcb->posicionStack].argumentos[identificador_variable-48].pagina)*TAMPAGINA+(pcb->indiceStack[pcb->posicionStack].argumentos[identificador_variable-48].offset);
			}
			else // es una variable
			{
				for (i = 0; i < pcb->indiceStack[pcb->posicionStack].cantidadVariables; i++)
				{
					if (identificador_variable==pcb->indiceStack[pcb->posicionStack].variables[i].id)
						break;
				}
				return (pcb->indiceStack->variables[i].pagina)*TAMPAGINA+(pcb->indiceStack->variables[i].offset);
			}
		}

		/*
		 * DEREFERENCIAR
		 *
		 * Obtiene el valor resultante de leer a partir de direccion_variable, sin importar cual fuera el contexto actual
		 *
		 * @sintax	TEXT_DEREFERENCE_OP (*)
		 * @param	direccion_variable	Lugar donde buscar
		 * @return	Valor que se encuentra en esa posicion
		 */
		t_valor_variable cpu_dereferenciar(t_puntero direccion_variable){
			int offset,pagina,valor;
			offset=direccion_variable%TAMPAGINA;
			pagina=pcb->paginasCodigo+direccion_variable/TAMPAGINA;
			t_peticionBytes * peticionValor=malloc(sizeof(t_peticionBytes));
			peticionValor->pid=PID;
			peticionValor->pagina=pagina;
			peticionValor->offset=offset;
			peticionValor->size=SIZE;
			enviarDinamico(SOLICITUDBYTES,socketMemoria,peticionValor);
			free(peticionValor);
			recv(socketMemoria,&valor,sizeof(int),0);
			return valor;
		}

		/*
		 * ASIGNAR
		 *
		 * Inserta una copia del valor en la variable ubicada en direccion_variable.
		 *
		 * @sintax	TEXT_ASSIGNATION (=)
		 * @param	direccion_variable	lugar donde insertar el valor
		 * @param	valor	Valor a insertar
		 * @return	void
		 */
		void cpu_asignar(t_puntero direccion_variable, t_valor_variable valor){
			int offset,pagina;
			offset=direccion_variable%TAMPAGINA;
			pagina=pcb->paginasCodigo+direccion_variable/TAMPAGINA;
			t_almacenarBytes * bytes=malloc(sizeof(t_almacenarBytes));
			bytes->pid=PID;
			bytes->pagina=pagina;
			bytes->offset=offset;
			bytes->size=SIZE;
			bytes->valor=valor;
			enviarDinamico(ALMACENARBYTES,socketMemoria,bytes);
			free(bytes);
		}

		/*
		 * OBTENER VALOR de una variable COMPARTIDA
		 *
		 * Pide al kernel el valor (copia, no puntero) de la variable compartida por parametro.
		 *
		 * @sintax	TEXT_VAR_START_GLOBAL (!)
		 * @param	variable	Nombre de la variable compartida a buscar
		 * @return	El valor de la variable compartida
		 */
		t_valor_variable cpu_obtenerValorCompartida(t_nombre_compartida variable){
			int valor;
			t_solicitudValorVariable * solicitudValorVariable;
			solicitudValorVariable=malloc(sizeof(t_solicitudValorVariable));
			solicitudValorVariable->tamanioNombre=strlen(variable);
			solicitudValorVariable->variable=variable;
			
			enviarDinamico(SOLICITUDVALORVARIABLE,socketKernel,solicitudValorVariable);
			recv(socketKernel,&valor,sizeof(int),0);
			return valor;
		}

		/*
		 * ASIGNAR VALOR a variable COMPARTIDA
		 *
		 * Pide al kernel asignar el valor a la variable compartida.
		 * Devuelve el valor asignado.
		 *
		 * @sintax	TEXT_VAR_START_GLOBAL (!) IDENTIFICADOR TEXT_ASSIGNATION (=) EXPRESION
		 * @param	variable	Nombre (sin el '!') de la variable a pedir
		 * @param	valor	Valor que se le quire asignar
		 * @return	Valor que se asigno
		 */
		t_valor_variable cpu_asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){
			t_asignarVariableCompartida * asignarVariableCompartida;
			asignarVariableCompartida=malloc(sizeof(t_asignarVariableCompartida));
			asignarVariableCompartida->tamanioNombre=strlen(variable);
			asignarVariableCompartida->variable=variable;
			asignarVariableCompartida->valor=valor;
			enviarDinamico(ASIGNARVARIABLECOMPARTIDA,socketKernel,asignarVariableCompartida);
			return valor;
		}


		/*
		 * IR a la ETIQUETA
		 *
		 * Cambia la linea de ejecucion a la correspondiente de la etiqueta buscada.
		 *
		 * @sintax	TEXT_GOTO (goto)
		 * @param	t_nombre_etiqueta	Nombre de la etiqueta
		 * @return	void
		 */
		void cpu_irAlLabel(t_nombre_etiqueta t_nombre_etiqueta){
			int i,pos;
			pos=metadata_buscar_etiqueta(t_nombre_etiqueta, pcb->indiceEtiquetas.etiquetas, pcb->indiceEtiquetas.etiquetas_size);
			posicionarPC(pos);
		}

		/*
		 * LLAMAR SIN RETORNO
		 *
		 * Preserva el contexto de ejecución actual para poder retornar luego al mismo.
		 * Modifica las estructuras correspondientes para mostrar un nuevo contexto vacío.
		 *
		 * Los parámetros serán definidos luego de esta instrucción de la misma manera que una variable local, con identificadores numéricos empezando por el 0.
		 *
		 * @sintax	Sin sintaxis particular, se invoca cuando no coresponde a ninguna de las otras reglas sintacticas
		 * @param	etiqueta	Nombre de la funcion
		 * @return	void
		 */
		void cpu_llamarSinRetorno(t_nombre_etiqueta etiqueta){
			int i,pos;
			nuevoNivelStack();
			pos=metadata_buscar_etiqueta(etiqueta, pcb->indiceEtiquetas.etiquetas, pcb->indiceEtiquetas.etiquetas_size);
			posicionarPC(pos);
		}

		/*
		 * LLAMAR CON RETORNO
		 *
		 * Preserva el contexto de ejecución actual para poder retornar luego al mismo, junto con la posicion de la variable entregada por donde_retornar.
		 * Modifica las estructuras correspondientes para mostrar un nuevo contexto vacío.
		 *
		 * Los parámetros serán definidos luego de esta instrucción de la misma manera que una variable local, con identificadores numéricos empezando por el 0.
		 *
		 * @sintax	TEXT_CALL (<-)
		 * @param	etiqueta	Nombre de la funcion
		 * @param	donde_retornar	Posicion donde insertar el valor de retorno
		 * @return	void
		 */
		void cpu_llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
			nuevoNivelStack();
			int pos;
			pthread_mutex_lock(&mutexPcb);
			pcb->indiceStack[pcb->posicionStack].varRetorno.pagina=calcularPaginas(TAMPAGINA,donde_retornar);
			pcb->indiceStack[pcb->posicionStack].varRetorno.offset=donde_retornar%TAMPAGINA;
			pcb->indiceStack[pcb->posicionStack].varRetorno.size=SIZE;
			pthread_mutex_unlock(&mutexPcb);
			pos=metadata_buscar_etiqueta(etiqueta, pcb->indiceEtiquetas.etiquetas, pcb->indiceEtiquetas.etiquetas_size);
			posicionarPC(pos);
			
		}


		/*
		 * FINALIZAR
		 *
		 * Cambia el Contexto de Ejecución Actual para volver al Contexto anterior al que se está ejecutando, recuperando el Cursor de Contexto Actual y el Program Counter previamente apilados en el Stack.
		 * En caso de estar finalizando el Contexto principal (el ubicado al inicio del Stack), deberá finalizar la ejecución del programa.
		 *
		 * @sintax	TEXT_END (end)
		 * @param	void
		 * @return	void
		 */
		void cpu_finalizar(void){
			if (pcb->posicionStack==0)
			{	
				enviarDinamico(PCBFINALIZADO,socketKernel,pcb);
				liberarContenidoPcb();
			}
			else{
				finalizarNivelStack();
			}
		}

		/*
		 * RETORNAR
		 *
		 * Cambia el Contexto de Ejecución Actual para volver al Contexto anterior al que se está ejecutando, recuperando el Cursor de Contexto Actual, el Program Counter y la direccion donde retornar, asignando el valor de retorno en esta, previamente apilados en el Stack.
		 *
		 * @sintax	TEXT_RETURN (return)
		 * @param	retorno	Valor a ingresar en la posicion corespondiente
		 * @return	void
		 */
		void cpu_retornar(t_valor_variable retorno){
			t_almacenarBytes * bytes=malloc(sizeof(t_almacenarBytes));
			bytes->pid=PID;
			bytes->pagina=pcb->paginasCodigo+pcb->indiceStack[pcb->posicionStack].varRetorno.pagina;
			bytes->offset=pcb->indiceStack[pcb->posicionStack].varRetorno.offset;
			bytes->size=pcb->indiceStack[pcb->posicionStack].varRetorno.size;
			bytes->valor=retorno;
			enviarDinamico(ALMACENARBYTES,socketMemoria,bytes);
			free(bytes);
			finalizarNivelStack();
		}

		/*
		 * WAIT
		 *
		 * Informa al kernel que ejecute la función wait para el semáforo con el nombre identificador_semaforo.
		 * El kernel deberá decidir si bloquearlo o no.
		 *
		 * @sintax	TEXT_WAIT (wait)
		 * @param	identificador_semaforo	Semaforo a aplicar WAIT
		 * @return	void
		 */
		void cpu_wait(t_nombre_semaforo identificador_semaforo){
		t_solicitudSemaforo * solicitudSemaforo;
		void * paquete;
		solicitudSemaforo=malloc(sizeof(t_solicitudSemaforo));
		solicitudSemaforo->tamanioIdentificador=strlen(identificador_semaforo);
		solicitudSemaforo->identificadorSemaforo=malloc(solicitudSemaforo->tamanioIdentificador);
		solicitudSemaforo->identificadorSemaforo=identificador_semaforo;
		solicitudSemaforo->estadoSemaforo=-1;
		enviarDinamico(SOLICITUDSEM,socketKernel,solicitudSemaforo);
		t_semaforo * semaforo=malloc(sizeof(t_semaforo));
		recibirDinamico(SEMAFORO,socketKernel,semaforo);
		if(semaforo->estadoSemaforo==0){
			continuarEjecucion=0;
		}else
			continuarEjecucion=1;
		}

		/*
		 * SIGNAL
		 *
		 * Informa al kernel que ejecute la función signal para el semáforo con el nombre identificador_semaforo.
		 * El kernel deberá decidir si desbloquear otros procesos o no.
		 *
		 * @sintax	TEXT_SIGNAL (signal)
		 * @param	identificador_semaforo	Semaforo a aplicar SIGNAL
		 * @return	void
		 */
		void cpu_signal(t_nombre_semaforo identificador_semaforo){

		t_solicitudSemaforo * solicitudSemaforo;
		solicitudSemaforo=malloc(sizeof(t_solicitudSemaforo));
		solicitudSemaforo->identificadorSemaforo=identificador_semaforo;
		solicitudSemaforo->estadoSemaforo=-1;
		enviarDinamico(SOLICITUDSEM,socketKernel,solicitudSemaforo);
		}
	
		/*
		 * RESERVAR MEMORIA
		 *
		 * Informa al kernel que reserve en el Heap una cantidad de memoria
		 * acorde al espacio recibido como parametro.
		 *
		 * @sintax	TEXT_MALLOC (alocar)
		 * @param	valor_variable Cantidad de espacio
		 * @return	puntero a donde esta reservada la memoria
		 */
		t_puntero cpu_reservar(t_valor_variable espacio)
		{
			t_reservarEspacioMemoria * reservarEspacioMemoria;
			reservarEspacioMemoria=malloc(sizeof(t_reservarEspacioMemoria));
			reservarEspacioMemoria->espacio=espacio;
			enviarDinamico(RESERVARESPACIO,socketKernel,reservarEspacioMemoria);
			t_reservar * reservar=malloc(sizeof(t_reservar));
			recibirDinamico(RESERVAESPACIO,socketKernel,reservar);
			return reservar->puntero;
			

			//tengo que ver como es la respuesta de memoria
			//si puede, la reserva y me da la direccion de memoria
			//si no, nose si me tiene que dar un mensaje de excepcion
		}

		/*
		 * LIBERAR MEMORIA
		 *
		 * Informa al kernel que libera la memoria previamente reservada con RESERVAR.
		 * Solo se podra liberar memoria previamente asignada con RESERVAR.
		 *
		 * @sintax	TEXT_FREE (liberar)
		 * @param	puntero Inicio de espacio de memoria a liberar (previamente retornado por RESERVAR)
		 * @return	void
		 */
		void cpu_liberar(t_puntero puntero){
			t_liberarMemoria * liberarMemoria;
			liberarMemoria=malloc(sizeof(t_liberarMemoria));
			liberarMemoria->direccionMemoria=puntero;
			enviarDinamico(LIBERARESPACIOMEMORIA,socketKernel,liberarMemoria);
		}

		/*
		 * ABRIR ARCHIVO
		 *
		 * Informa al Kernel que el proceso requiere que se abra un archivo.
		 *
		 * @syntax 	TEXT_OPEN_FILE (abrir)
		 * @param	direccion		Ruta al archivo a abrir
		 * @param	banderas		String que contiene los permisos con los que se abre el archivo
		 * @return	El valor del descriptor de archivo abierto por el sistema
		 */
		t_descriptor_archivo cpu_abrir(t_direccion_archivo direccion, t_banderas flags){
					t_abrirArchivo * abrirArchivo;
					abrirArchivo=malloc(sizeof(t_abrirArchivo));
					abrirArchivo->direccionArchivo=direccion;
					abrirArchivo->flags=flags;
					enviarDinamico(ABRIRARCHiVO,socketKernel,abrirArchivo);
					t_fdParaLeer * fdParaLeer= malloc(sizeof(t_fdParaLeer));
					recibirDinamico(ABRIOARCHIVO,socketKernel,fdParaLeer);
					
		
					return fdParaLeer->fd;}
		/*
		 * BORRAR ARCHIVO
		 *
		 * Informa al Kernel que el proceso requiere que se borre un archivo.
		 *
		 * @syntax 	TEXT_DELETE_FILE (borrar)
		 * @param	descriptor_archivo		Descriptor de archivo del archivo a borrar
		 * @return	void
		 */
		void cpu_borrar(t_descriptor_archivo descriptor_archivo){
			t_borrarArchivo * borrarArchivo;
			borrarArchivo=malloc(sizeof(t_borrarArchivo));
			borrarArchivo->fdABorrar=descriptor_archivo;
			enviarDinamico(BORRARARCHIVO,socketKernel,borrarArchivo);
			
		}

		/*
		 * CERRAR ARCHIVO
		 *
		 * Informa al Kernel que el proceso requiere que se cierre un archivo.
		 *
		 * @syntax 	TEXT_CLOSE_FILE (cerrar)
		 * @param	descriptor_archivo		Descriptor de archivo del archivo abierto
		 * @return	void
		 */

		void cpu_cerrar(t_descriptor_archivo descriptor_archivo){
					t_cerrarArchivo * cerrarArchivo;
					cerrarArchivo=malloc(sizeof(t_cerrarArchivo));
					cerrarArchivo->descriptorArchivo=descriptor_archivo;
					
					enviarDinamico(CERRARARCHIVO,socketKernel,cerrarArchivo);}
		/*
		 * MOVER CURSOR DE ARCHIVO
		 *
		 * Informa al Kernel que el proceso requiere que se mueva el cursor a la posicion indicada.
		 *
		 * @syntax 	TEXT_SEEK_FILE (buscar)
		 * @param	descriptor_archivo		Descriptor de archivo del archivo abierto
		 * @param	posicion			Posicion a donde mover el cursor
		 * @return	void
		 */
		void cpu_moverCursor(t_descriptor_archivo descriptor_archivo, t_valor_variable posicion){
			t_moverCursor * moverCursor;
			moverCursor=malloc(sizeof(t_moverCursor));
			moverCursor->descriptorArchivo=descriptor_archivo;
			moverCursor->posicion=posicion;
			
			enviarDinamico(ABRIRARCHiVO,socketKernel,moverCursor);
		}
		/*
		 * ESCRIBIR ARCHIVO
		 *
		 * Informa al Kernel que el proceso requiere que se escriba un archivo previamente abierto.
		 * El mismo escribira "tamanio" de bytes de "informacion" luego del cursor
		 * No es necesario mover el cursor luego de esta operación
		 *
		 * @syntax 	TEXT_WRITE_FILE (escribir)
		 * @param	descriptor_archivo		Descriptor de archivo del archivo abierto
		 * @param	informacion			Informacion a ser escrita
		 * @param	tamanio				Tamanio de la informacion a enviar
		 * @return	void
		 */
		void cpu_escribir(t_descriptor_archivo descriptor_archivo, void* informacion, t_valor_variable tamanio){
			t_escribirArchivo * escribirArchivo;
			escribirArchivo=malloc(sizeof(t_escribirArchivo));
			escribirArchivo->fdArchivo=descriptor_archivo;
			escribirArchivo->informacion=malloc(tamanio);
			escribirArchivo->informacion=informacion;
			escribirArchivo->tamanio=tamanio;
			enviarDinamico(ESCRIBIRARCHIVO,socketKernel,escribirArchivo);
		}

		/*
		 * LEER ARCHIVO
		 *
		 * Informa al Kernel que el proceso requiere que se lea un archivo previamente abierto.
		 * El mismo leera "tamanio" de bytes luego del cursor.
		 * No es necesario mover el cursor luego de esta operación
		 *
		 * @syntax 	TEXT_READ_FILE (leer)
		 * @param	descriptor_archivo		Descriptor de archivo del archivo abierto
		 * @param	informacion			Puntero que indica donde se guarda la informacion leida
		 * @param	tamanio				Tamanio de la informacion a leer
		 * @return	void
		 */
		void cpu_leer(t_descriptor_archivo descriptor_archivo, t_puntero informacion, t_valor_variable tamanio){

			t_leerArchivo * leerArchivo;
			leerArchivo=malloc(sizeof(t_leerArchivo));
			leerArchivo->descriptor=descriptor_archivo;
			leerArchivo->punteroInformacion=informacion;
			leerArchivo->tamanio=tamanio;
			enviarDinamico(LEERARCHIVO,socketKernel,leerArchivo);
		}

AnSISOP_funciones functions = {
		.AnSISOP_obtenerValorCompartida=cpu_obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida=cpu_asignarValorCompartida,
		.AnSISOP_definirVariable=cpu_definirVariable,
		.AnSISOP_obtenerPosicionVariable=cpu_obtenerPosicionVariable,
		.AnSISOP_finalizar=cpu_finalizar,
		.AnSISOP_dereferenciar=cpu_dereferenciar,
		.AnSISOP_asignar=cpu_asignar,
		.AnSISOP_irAlLabel=cpu_irAlLabel,
		.AnSISOP_llamarConRetorno=cpu_llamarConRetorno,
		.AnSISOP_llamarSinRetorno=cpu_llamarSinRetorno,
		.AnSISOP_retornar=cpu_retornar,
};
AnSISOP_kernel kernel_functions = {
		.AnSISOP_wait=cpu_wait,
		.AnSISOP_signal=cpu_signal,
		.AnSISOP_reservar=cpu_reservar,
		.AnSISOP_liberar=cpu_liberar,
		.AnSISOP_abrir=cpu_abrir,
		.AnSISOP_borrar=cpu_borrar,
		.AnSISOP_cerrar=cpu_cerrar,
		.AnSISOP_moverCursor=cpu_moverCursor,
		.AnSISOP_escribir=cpu_escribir,
		.AnSISOP_leer=cpu_leer,
};

void iniciarEjecucion(char * linea){

		
		printf("\t Evaluando -> %s", linea);

		pthread_mutex_lock(&mutexPcb);
		pcb->programCounter++;
		pthread_mutex_unlock(&mutexPcb);
		
		analizadorLinea(linea, &functions, &kernel_functions);
		free(linea);

		//tengo que guardar en que linea estoy en el program counter para que cuando tuermine un quantum guardar ese contexto para que despues pueda seguir desde ahi
			
		
}

void conectarKernel(void){

int bytesRecibidos;
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(IP_KERNEL,PUERTO_KERNEL,&hints,&serverInfo);


	
	
	
	socketKernel = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);


	connect(socketKernel, serverInfo->ai_addr, serverInfo->ai_addrlen);

	
	freeaddrinfo(serverInfo);
	int * buffer=malloc(sizeof(int));
	int a=1;
	memcpy(buffer,&a,sizeof(int));
	void * unBuffer;
	handshakeCliente(socketKernel, CPU, unBuffer);
	int recibir;
	t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
	t_linea * linea;

t_peticionBytes * peticionLinea;
while(1) {
	if (continuarEjecucion)
	{
		enviarDinamico(ESPERONOVEDADES,socketKernel,NULL);
		while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0));
	}
	else{
		seleccionador->tipoPaquete=PARAREJECUCION;
	}
	
	if(seleccionador->unaInterfaz==CPU){
	switch (seleccionador->tipoPaquete){
		case PCB: 
							recibirDinamico(PCB,socketKernel,pcb); 							
 							peticionLinea=malloc(sizeof(t_peticionBytes));
 							peticionLinea->pid=PID;
 							peticionLinea->pagina=calcularPaginas(TAMPAGINA,pcb->indiceCodigo[0].start);
							peticionLinea->offset=pcb->indiceCodigo[0].start;
							peticionLinea->size=pcb->indiceCodigo[0].offset;		
							enviarDinamico(SOLICITUDBYTES,socketMemoria,(void *) peticionLinea);
							free(peticionLinea);
							linea=malloc(sizeof(t_linea));
		 					recibirDinamico(LINEA,socketMemoria, linea);
							linea->linea=realloc(linea->linea,(linea->tamanio+1)*sizeof(char));
		 					iniciarEjecucion(linea->linea);
		 					free(linea);
							break;
		case CONTINUAR:
							
							peticionLinea=malloc(sizeof(t_peticionBytes));
							peticionLinea->pid=PID;
					 		peticionLinea->pagina=calcularPaginas(TAMPAGINA,pcb->indiceCodigo[pcb->programCounter].start);
							peticionLinea->offset=pcb->indiceCodigo[pcb->programCounter].start;
							peticionLinea->size=pcb->indiceCodigo[pcb->programCounter].offset;			
							enviarDinamico(SOLICITUDBYTES,socketMemoria,(void *) peticionLinea);
							free(peticionLinea);
							linea=malloc(sizeof(t_linea));
		 					recibirDinamico(LINEA,socketMemoria, linea);
							linea->linea=realloc(linea->linea,(linea->tamanio+1)*sizeof(char));
		 					iniciarEjecucion(linea->linea);
		 					free(linea);
							break;

 		case FINALIZARPROCESO: case FINQUANTUM: case PARAREJECUCION:
 								enviarDinamico(PCBFINALIZADO,socketKernel,pcb);
 								liberarContenidoPcb();		
 		break;
}}}
free(seleccionador);
free(buffer);
}	





 







//tiene que estar siempre en contacto con la memoria
//cuando se conecta con el kernel, queda a la espera de recibir el pcb de un proceso
//tiene que aumentar el program counter del PCB
//utilizo el indice de codigo para solicitar a memoria la proxima linea a ejecutar
//cuando la ejecuto se debe acutualizar los valores del programa en la memoria
//actualizar el program counter del PCB
//cuando termina todo el codigo de ejecutar avisarle al kernel para que libere ese espacio que ocupaba










int main(){


	t_config * CFG;
	CFG = config_create("CPUCFG.txt");
	IP_KERNEL=(char*) config_get_string_value(CFG ,"IP_KERNEL");
	IP_MEMORIA= (char*)config_get_string_value(CFG, "IP_MEMORIA");
	PUERTO_KERNEL= (char*)config_get_string_value(CFG ,"PUERTO_KERNEL");
	PUERTO_MEMORIA= (char*)config_get_string_value(CFG,"PUERTO_MEMORIA");
	printf("Configuración:\nIP_KERNEL = %s,\nIP_MEMORIA=%s,\nPUERTO_KERNEL = %s.\n,PUERTO_MEMORIA=%s. \n",IP_KERNEL,IP_MEMORIA,PUERTO_KERNEL,PUERTO_MEMORIA);
	
	printf("Presione enter para continuar.\n");
	getchar();
	pcb=malloc(sizeof(t_pcb));
	/*
	*
	*/
	int bytesRecibidos;
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(IP_MEMORIA,PUERTO_MEMORIA,&hints,&serverInfo);


	socketMemoria = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);


	connect(socketMemoria, serverInfo->ai_addr, serverInfo->ai_addrlen);

	freeaddrinfo(serverInfo);
	void * unBuffer;

	
	handshakeCliente(socketMemoria, CPU, unBuffer);
	recv(socketMemoria, &TAMPAGINA, sizeof(int), 0);



	config_destroy(CFG);
	pthread_mutex_init(&mutexPcb,NULL);

	pthread_t conectarKernel, conectarMemoria;
	pthread_create(&conectarKernel, NULL, (void *) conectarKernel,&socketKernel);
	pthread_join(conectarKernel,NULL);
}


