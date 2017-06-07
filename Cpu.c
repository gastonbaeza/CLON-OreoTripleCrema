#include "funciones.h"
#include "estructuras.h"
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
#include <stddef.h>



#define SOLICITUDLINEA 2
#define FINALIZARPROCESO 16
#define PCB 17
#define CPU 1

#define LINEA 19

int continuarEjecucion=1;
int PID;
int i;
t_pcb * pcb;
char * IP_KERNEL;
char * PUERTO_KERNEL;
char * IP_MEMORIA;
char * PUERTO_MEMORIA;



int socketKernel;
int socketMemoria;

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
		t_puntero (*AnSISOP_definirVariable)(t_nombre_variable identificador_variable){
			if (identificador_variable>=48 $$ identificador_variable<=57) // es un parametro
			{
				
			}
			else // es una variable
			{

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
		t_puntero (*AnSISOP_obtenerPosicionVariable)(t_nombre_variable identificador_variable);

		/*
		 * DEREFERENCIAR
		 *
		 * Obtiene el valor resultante de leer a partir de direccion_variable, sin importar cual fuera el contexto actual
		 *
		 * @sintax	TEXT_DEREFERENCE_OP (*)
		 * @param	direccion_variable	Lugar donde buscar
		 * @return	Valor que se encuentra en esa posicion
		 */
		t_valor_variable (*AnSISOP_dereferenciar)(t_puntero direccion_variable);

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
		void (*AnSISOP_asignar)(t_puntero direccion_variable, t_valor_variable valor);

		/*
		 * OBTENER VALOR de una variable COMPARTIDA
		 *
		 * Pide al kernel el valor (copia, no puntero) de la variable compartida por parametro.
		 *
		 * @sintax	TEXT_VAR_START_GLOBAL (!)
		 * @param	variable	Nombre de la variable compartida a buscar
		 * @return	El valor de la variable compartida
		 */
		t_valor_variable (*AnSISOP_obtenerValorCompartida)(t_nombre_compartida variable){

			t_solicitudValorVariable * solicitudValorVariable;
			solicitudValorVariable=malloc(sizeof(t_solicitudValorVariable));
			solicitudValorVariable->variable=variable;
			
			enviarDinamico(SOLICITUDVALORVARIABLE,socketKernel,solicitudValorVariable);
			recibirDinamico(,socketKernel,valor_variable)
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
		t_valor_variable (*AnSISOP_asignarValorCompartida)(t_nombre_compartida variable, t_valor_variable valor){
			t_asignarVariableCompartida * asignarVariableCompartida;
			asignarVariableCompartida=malloc(sizeof(t_asignarVariableCompartida));
			asignarVariableCompartida->variableCompartida=variable;
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
		void (*AnSISOP_irAlLabel)(t_nombre_etiqueta t_nombre_etiqueta);

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
		void (*AnSISOP_llamarSinRetorno)(t_nombre_etiqueta etiqueta);

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
		void (*AnSISOP_llamarConRetorno)(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);


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
		void (*AnSISOP_finalizar)(void);

		/*
		 * RETORNAR
		 *
		 * Cambia el Contexto de Ejecución Actual para volver al Contexto anterior al que se está ejecutando, recuperando el Cursor de Contexto Actual, el Program Counter y la direccion donde retornar, asignando el valor de retorno en esta, previamente apilados en el Stack.
		 *
		 * @sintax	TEXT_RETURN (return)
		 * @param	retorno	Valor a ingresar en la posicion corespondiente
		 * @return	void
		 */
		void (*AnSISOP_retornar)(t_valor_variable retorno);

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
		void (*AnSISOP_wait)(t_nombre_semaforo identificador_semaforo){
		t_solicitudSemaforo * solicitudSemaforo;
		solicitudSemaforo=malloc(sizeof(t_solicitudSemaforo));
		solicitudSemaforo->identificadorSemaforo=identificador_semaforo;
		solicitudSemaforo->estadoSemaforo=-1;
		envioDinamico(SOLICITUDSEM,socketKernel,solicitudSemaforo);
		recibirDinamico(SEMAFORO,socketKernel,paquete);
		t_semaforo * semaforo=(t_semaforo *) paquete;
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
		void (*AnSISOP_signal)(t_nombre_semaforo identificador_semaforo){

		t_solicitudSemaforo * solicitudSemaforo;
		solicitudSemaforo=malloc(sizeof(t_solicitudSemaforo));
		solicitudSemaforo->identificadorSemaforo=identificador_semaforo;
		solicitudSemaforo->estadoSemaforo=-1;
		envioDinamico(SOLICITUDSEM,socketKernel,solicitudSemaforo);
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
		t_puntero (*AnSISOP_reservar)(t_valor_variable espacio);
		{
			t_reservarEspacioMemoria * reservarEspacioMemoria;
			reservarEspacioMemoria=malloc(sizeof(t_reservarEspacioMemoria));
			reservarEspacioMemoria->espacio=espacio;
			envioDinamico(RESERVARESPACIO,socketMemoria,reservarEspacioMemoria);
			recibirDinamico(RESERVAESPACIO,socketMemoria,paquete);
			t_reservar * reservar=(t_reservar *) paquete;

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
		void (*AnSISOP_liberar)(t_puntero puntero){
			t_liberarMemoria * liberarMemoria;
			liberarMemoria=malloc(sizeof(t_liberarMemoria));
			liberarMemoria->direccionMemoria=puntero;
			envioDinamico(LIBERARESPACIOMEMORIA,socketMemoria,liberarMemoria);
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
		t_descriptor_archivo (*AnSISOP_abrir)(t_direccion_archivo direccion, t_banderas flags);
			t_abrirArchivo * abrirArchivo;
			abrirArchivo=malloc(sizeof(t_abrirArchivo));
			abrirArchivo->direccionArchivo=direccion;
			abrirArchivo->flags=flags;
			enviarDinamico(ABRIRARCHiVO,socketKernel,abrirArchivo);
			recibirDinamico(ABRIOARCHIVO,socketKernel,paquete);
			t_fdParaLeer * fdParaLeer= (t_fdParaLeer *) paquete;

			return fdParaLeer->fd;
		/*
		 * BORRAR ARCHIVO
		 *
		 * Informa al Kernel que el proceso requiere que se borre un archivo.
		 *
		 * @syntax 	TEXT_DELETE_FILE (borrar)
		 * @param	descriptor_archivo		Descriptor de archivo del archivo a borrar
		 * @return	void
		 */
		void (*AnSISOP_borrar)(t_descriptor_archivo descriptor_archivo){
			t_borrarArchivo * borrarArchivo;
			borrarArchivo=malloc(sizeof(t_borrarArchivo));
			borrarArchivo->fdABorrar=descriptor_archivo;
			envioDinamico(BORRARARCHIVO,socketKernel,borrarArchivo);
			
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

		void (*AnSISOP_cerrar)(t_descriptor_archivo descriptor_archivo);
		t_abrirArchivo * abrirArchivo;
			abrirArchivo=malloc(sizeof(t_abrirArchivo));
			abrirArchivo->direccionArchivo=direccion;
			abrirArchivo->flags=flags;
			enviarDinamico(ABRIRARCHiVO,socketKernel,abrirArchivo);
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
		void (*AnSISOP_moverCursor)(t_descriptor_archivo descriptor_archivo, t_valor_variable posicion){
			t_moverCursorArchivo * moverCursorArchivo;
			moverCursorArchivo=malloc(sizeof(t_moverCursorArchivo));
			abrirArchivo->direccionArchivo=direccion;
			abrirArchivo->flags=flags;
			enviarDinamico(ABRIRARCHiVO,socketKernel,abrirArchivo);}
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
		void (*AnSISOP_escribir)(t_descriptor_archivo descriptor_archivo, void* informacion, t_valor_variable tamanio){
			t_escribirArchivo * escribirArchivo;
			escribirArchivo=malloc(sizeof(t_escribirArchivo));
			escribirArchivo->fdArchivo=descriptor;
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
		void (*AnSISOP_leer)(t_descriptor_archivo descriptor_archivo, t_puntero informacion, t_valor_variable tamanio){

			t_abrirArchivo * abrirArchivo;
			abrirArchivo=malloc(sizeof(t_abrirArchivo));
			abrirArchivo->direccionArchivo=direccion;
			abrirArchivo->flags=flags;
			enviarDinamico(ABRIRARCHiVO,socketKernel,abrirArchivo);
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

void * unBuffer;
handshakeCliente(socketKernel, CPU, unBuffer);
void * paquete;
int recibir;
t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));

t_peticionLinea * peticionLinea;
while(1) {
	while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0));
	
	if(seleccionador->unaInterfaz==CPU){
	switch (seleccionador->tipoPaquete){
		case PCB: // [Identificador del Programa] // informacion del proceso
							
							recibirDinamico(PCB,socketKernel,pcb);
							
 							
 							peticionLinea=malloc(sizeof(t_peticionLinea));
							peticionLinea->start=pcb->indiceCodigo[0].start;
							peticionLinea->offset=pcb->indiceCodigo[0].offset;		
							envioDinamico(SOLICITUDLINEA,socketMemoria,(void *) peticionLinea);
							free(peticionLinea);
							pcb->programCounter=pcb->indiceCodigo[0].start;
							i=0;

 								
 								
 				break;

 		case FINALIZARPROCESO: 
 								continuarEjecucion=0; 
 								free(pcb->indiceCodigo);					
 		break;
}}}
free(seleccionador);
}	



void conectarMemoria(void){
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
	void * paquete;
	int recibir;
	t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
	t_linea * linea;
	char* lineaCodigo;
	t_programaSalida * programa;
	while(1){
		while(0>recv(socketMemoria, seleccionador, sizeof(t_seleccionador),0));
		
		if(continuarEjecucion==1){
	if(seleccionador->unaInterfaz==CPU){
 	switch(seleccionador->tipoPaquete){
 		case LINEA: 
 					linea=malloc(sizeof(t_linea));
 					recibirDinamico(LINEA,socketMemoria, linea);
 					lineaCodigo=malloc(linea->tamanio);
 					lineaCodigo=linea->linea;
 					iniciarEjecucion(lineaCodigo);
 					free(linea);
 		break;



 		
}}}

				
 	}
 	free(seleccionador);
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


	config_destroy(CFG);

	pthread_t conectarKernel, conectarMemoria;
	pthread_create(&conectarMemoria, NULL, (void *) conectarMemoria,&socketMemoria);
	pthread_create(&conectarKernel, NULL, (void *) conectarKernel,&socketKernel);
	pthread_join(conectarKernel,NULL);
	pthread_join(conectarMemoria,NULL);






void iniciarEjecucion(char * linea){

		
		printf("\t Evaluando -> %s", linea);
		
		analizadorLinea(linea, &functions, &kernel_functions);

		//tengo que guardar en que linea estoy en el program counter para que cuando tuermine un quantum guardar ese contexto para que despues pueda seguir desde ahi
		
		
		i++;
		
		t_peticionLinea * peticionLinea=malloc(sizeof(t_peticionLinea));
		peticionLinea->start=pcb->indiceCodigo[i].start;
		peticionLinea->offset=pcb->indiceCodigo[i].offset;		
		envioDinamico(SOLICITUDLINEA,socketMemoria,(void *) peticionLinea);
		pcb->programCounter=pcb->indiceCodigo[i].start;
		free(peticionLinea);
	
	
		
}}

