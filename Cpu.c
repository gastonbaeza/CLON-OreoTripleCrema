#include "estructuras.h"
#include "funciones.h"
#include <parser/parser.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/string.h>
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
#include <signal.h> 
#include <linux/limits.h>


#define SOLICITUDLINEA 2
#define PCB 17
#define CPU 3
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
#define MOVERCURSOR 52
#define FINALIZARPROCESO 53
#define PCBBLOQUEADO 54
#define PCBQUANTUM 55
#define PCBFINALIZADOPORCONSOLA 56
#define SOLICITUDSEMWAIT 57
#define FINALIZARPORERROR 59
#define PCBERROR 60
#define PAGINAINVALIDA 61
#define STACKOVERFLOW 62
#define PAQUETEFS 67
#define RESERVADOESPACIO 69
#define LINEA 19
#define BLOQUE 5
#define SIZE 4

int continuarEjecucion=1;
int PID;
int i;
int primerAcceso;
t_pcb * pcb;
char * IP_KERNEL;
char * PUERTO_KERNEL;
char * IP_MEMORIA;
char * PUERTO_MEMORIA;
pthread_mutex_t mutexPcb;
int PROXIMAPAG=0;
int PROXIMOOFFSET=0;
int TAMPAGINA;
int flagSignal=0;
FILE *CPULog;
char * horaActual;
char* nombreLog;
int len;



int socketKernel;
int socketMemoria;

void stripLog(char** string){
	int i ;
	printf("%s\n", *string);
	for(i=0;i<string_length(*string); i++){
		if((*string)[i]==' ' || (*string)[i]=='/' )
			(*string)[i]='-';
	}
}
void cortar(){
	fclose(CPULog);
	exit(0);
}
void escribirEnArchivoLog(char * contenidoAEscribir, FILE ** archivoDeLog,char * direccionDelArchivo){
	
	fseek(*archivoDeLog,0,SEEK_END);
	len=ftell(*archivoDeLog);
	fwrite(contenidoAEscribir,strlen(contenidoAEscribir),1,*archivoDeLog);
	fwrite("\n",1,1,*archivoDeLog);
	}

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
	free(pcb->indiceStack[pcb->posicionStack].variables);
	free(pcb->indiceStack[pcb->posicionStack].argumentos);
	pcb->cantidadStack--;
	pcb->indiceStack=realloc(pcb->indiceStack,pcb->cantidadStack*sizeof(t_stack));
	pcb->posicionStack--;
	pthread_mutex_unlock(&mutexPcb);
}


void liberarContenidoPcb(){
	int i;
	free(pcb->indiceCodigo);
	if (pcb->cantidadArchivos)
	{
		free(pcb->referenciaATabla);
	}
	for (i = 0; i < pcb->cantidadStack; i++)
	{	
		if (pcb->indiceStack[i].cantidadArgumentos)
		{
			free(pcb->indiceStack[i].argumentos);
		}
		if (pcb->indiceStack[i].cantidadVariables)
		{
			free(pcb->indiceStack[i].variables);
		}
	}
	free(pcb->indiceEtiquetas.etiquetas);
	free(pcb->indiceStack);
}

void limpiar_string(char** string){
	int i ;
	for(i=0;i<string_length(*string); i++){
		if((*string)[i]=='\n' || (*string)[i]=='\r' || (*string)[i]==' ' )
			(*string)[i]='\0';
	}
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
			printf("en cpu_definirVariable\n");
			escribirEnArchivoLog("en definir variable", &CPULog,nombreLog);
			if (identificador_variable>=48 && identificador_variable<=57) // es un parametro
			{
				pthread_mutex_lock(&mutexPcb);
				pcb->indiceStack[pcb->posicionStack].cantidadArgumentos++;
				if (pcb->indiceStack[pcb->posicionStack].cantidadArgumentos==1)
				{
					pcb->indiceStack[pcb->posicionStack].argumentos=malloc(pcb->indiceStack[pcb->posicionStack].cantidadArgumentos*sizeof(t_argumento));
				}
				else{

					pcb->indiceStack[pcb->posicionStack].argumentos=realloc(pcb->indiceStack[pcb->posicionStack].argumentos,pcb->indiceStack[pcb->posicionStack].cantidadArgumentos*sizeof(t_argumento));
				}
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
				if (pcb->indiceStack[pcb->posicionStack].cantidadVariables==1)
				{
					pcb->indiceStack[pcb->posicionStack].variables=malloc(pcb->indiceStack[pcb->posicionStack].cantidadVariables*sizeof(t_variable));
				}
				else{
					pcb->indiceStack[pcb->posicionStack].variables=realloc(pcb->indiceStack[pcb->posicionStack].variables,pcb->indiceStack[pcb->posicionStack].cantidadVariables*sizeof(t_variable));
				}
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
			printf("en cpu_obtenerPosicionVariable\n");
			escribirEnArchivoLog("en obtener posicion variable", &CPULog,nombreLog);
			int i;
			printf("identificador: %c.\n", identificador_variable);
			if (identificador_variable>=48 && identificador_variable<=57) // es un parametro
			{
				return (pcb->indiceStack[pcb->posicionStack].argumentos[identificador_variable-48].pagina)*TAMPAGINA+(pcb->indiceStack[pcb->posicionStack].argumentos[identificador_variable-48].offset);
			}
			else // es una variable
			{
				printf("pcb->posicionStack: %i.\n", pcb->posicionStack);
				printf("es variable\n");
				printf("cantidad Variables: %i.\n",pcb->indiceStack[pcb->posicionStack].cantidadVariables);
				printf("primerVariable: %c.\n", pcb->indiceStack[pcb->posicionStack].variables[0].id);
				for (i = 0; i < pcb->indiceStack[pcb->posicionStack].cantidadVariables; i++)
				{	printf("%c==%c?\n",identificador_variable, pcb->indiceStack[pcb->posicionStack].variables[i].id);
					if (identificador_variable==pcb->indiceStack[pcb->posicionStack].variables[i].id)
						{break;
							printf("entre!\n");}
				}
				printf("direccion: %i.\n", (pcb->indiceStack[pcb->posicionStack].variables[i].pagina)*TAMPAGINA+(pcb->indiceStack[pcb->posicionStack].variables[i].offset));
				return (pcb->indiceStack[pcb->posicionStack].variables[i].pagina)*TAMPAGINA+(pcb->indiceStack[pcb->posicionStack].variables[i].offset);
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
			printf("en cpu_dereferenciar\n");
			escribirEnArchivoLog("en dereferenciar", &CPULog,nombreLog);
			printf("Direccion: %i.\n", direccion_variable);
			int offset,pagina,valor,foo,resultado;
			offset=direccion_variable%TAMPAGINA;
			pagina=pcb->paginasCodigo+direccion_variable/TAMPAGINA;
			t_peticionBytes * peticion=malloc(sizeof(t_peticionBytes));
			peticion->pid=PID;
			peticion->pagina=pagina;
			peticion->offset=offset;
			peticion->size=SIZE;
			printf("%i\n", peticion->pid);
			printf("%i\n", peticion->pagina);
			printf("%i\n", peticion->offset);
			printf("%i\n", peticion->size);
			enviarDinamico(SOLICITUDBYTES,socketMemoria,peticion);
			escribirEnArchivoLog("envio solicitud bytes", &CPULog,nombreLog);
			while(0>recv(socketMemoria,&foo,sizeof(int),0)){
				perror("asd:");
			}
			if(foo==1){
				free(peticion);
				while(0>recv(socketMemoria,&resultado,sizeof(int),0));
				printf("valor dereferenciado: %i.\n", resultado);
				return resultado;
			}
			else{
				enviarDinamico(STACKOVERFLOW,socketKernel,1);
				escribirEnArchivoLog("envio stack overflow", &CPULog,nombreLog);
				return -1;
			}
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
			printf("en cpu_asignar\n");
			escribirEnArchivoLog("en asignar", &CPULog,nombreLog);
			printf("direccion a asignar: %i.\n",direccion_variable );
			printf("valor a asignar: %i.\n", valor);
			int offset,pagina,rv;
			offset=direccion_variable%TAMPAGINA;
			pagina=pcb->paginasCodigo+direccion_variable/TAMPAGINA;
			t_almacenarBytes * bytes=malloc(sizeof(t_almacenarBytes));
			bytes->valor=calloc(1,sizeof(int));
			bytes->pid=PID;
			bytes->pagina=pagina;
			bytes->offset=offset;
			memcpy(bytes->valor,&valor,sizeof(int));
			bytes->size=SIZE;
			printf("este es el valor pagina %i\n", bytes->pagina);
			printf("este es el valor offset %i\n", bytes->offset);
			printf("este es el valor size %i\n", bytes->size);
			printf("este es el buffer : %i\n",*(int*)(bytes->valor) );
			enviarDinamico(ALMACENARBYTES,socketMemoria,bytes);
			escribirEnArchivoLog("envio almacenar bytes", &CPULog,nombreLog);
			while(0>recv(socketMemoria,&rv,sizeof(int),0)){
				perror("asd:");
			}
			if (rv==-1)
			{
				printf("me devolvio error\n");
				enviarDinamico(STACKOVERFLOW,socketKernel,1);
				escribirEnArchivoLog("envio stack overflow", &CPULog,nombreLog);
			}
			free(bytes->valor);
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
			t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
			printf("en cpu_obtenerValorCompartida\n");
			escribirEnArchivoLog("en obtener valor compartida", &CPULog,nombreLog);
			int valor;
			t_solicitudValorVariable * solicitudValorVariable;
			solicitudValorVariable=malloc(sizeof(t_solicitudValorVariable));
			solicitudValorVariable->tamanioNombre=strlen(variable);
			solicitudValorVariable->variable=variable;
			
			enviarDinamico(SOLICITUDVALORVARIABLE,socketKernel,solicitudValorVariable);
			enviarDinamico(PCB,socketKernel,pcb);
			liberarContenidoPcb();
			escribirEnArchivoLog("envio solicitud valor variable", &CPULog,nombreLog);
			recv(socketKernel,&valor,sizeof(int),0);
			printf("valor compartida: %i\n", valor);
			while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0)){printf("asddsa\n");}
			recibirDinamico(PCB,socketKernel,pcb);
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
			t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
			printf("en cpu_asignarValorCompartida\n");
			escribirEnArchivoLog("en asignar valor compartida", &CPULog,nombreLog);
			printf("valor: %i.\n", valor);
			t_asignarVariableCompartida * asignarVariableCompartida;
			asignarVariableCompartida=malloc(sizeof(t_asignarVariableCompartida));
			asignarVariableCompartida->tamanioNombre=strlen(variable);
			asignarVariableCompartida->variable=variable;
			asignarVariableCompartida->valor=valor;
			enviarDinamico(ASIGNARVARIABLECOMPARTIDA,socketKernel,asignarVariableCompartida);
			escribirEnArchivoLog("envio asignar variable compartida", &CPULog,nombreLog);
			enviarDinamico(PCB,socketKernel,pcb);
			liberarContenidoPcb();
			while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0)){printf("asddsa\n");}
			recibirDinamico(PCB,socketKernel,pcb);
			printf("qwe\n");
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
		void cpu_irAlLabel(t_nombre_etiqueta nombre_etiqueta){
			printf("en cpu_irAlLabel\n");
			escribirEnArchivoLog("en ir a label", &CPULog,nombreLog);
			int i,pos;
			printf("%s\n",nombre_etiqueta );
			limpiar_string(&nombre_etiqueta);
			pos=metadata_buscar_etiqueta(nombre_etiqueta, pcb->indiceEtiquetas.etiquetas, pcb->indiceEtiquetas.etiquetas_size);
			printf("pos:%i\n", pos);
			pcb->programCounter=pos;
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
			printf("en cpu_llamarSinRetorno\n");
			escribirEnArchivoLog("en llamada sin retorno", &CPULog,nombreLog);
			int i,pos;
			nuevoNivelStack();
			limpiar_string(&etiqueta);
			pos=metadata_buscar_etiqueta(etiqueta, pcb->indiceEtiquetas.etiquetas, pcb->indiceEtiquetas.etiquetas_size);
			pcb->programCounter=pos;
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
			printf("en cpu_llamarConRetorno\n");
			escribirEnArchivoLog("en llamada con retorno", &CPULog,nombreLog);
			nuevoNivelStack();
			int pos;
			pthread_mutex_lock(&mutexPcb);
			pcb->indiceStack[pcb->posicionStack].varRetorno.pagina=calcularPaginas(TAMPAGINA,donde_retornar);
			pcb->indiceStack[pcb->posicionStack].varRetorno.offset=donde_retornar%TAMPAGINA;
			pcb->indiceStack[pcb->posicionStack].varRetorno.size=SIZE;
			pthread_mutex_unlock(&mutexPcb);
			limpiar_string(&etiqueta);
			pos=metadata_buscar_etiqueta(etiqueta, pcb->indiceEtiquetas.etiquetas, pcb->indiceEtiquetas.etiquetas_size);
			printf("pcb->indiceEtiquetas.etiquetas: %s.\n", pcb->indiceEtiquetas.etiquetas);
			printf("pcb->indiceEtiquetas.etiquetas_size: %i.\n", pcb->indiceEtiquetas.etiquetas_size);
			printf("pos: %i\n", pos);
			pcb->programCounter=pos;
			
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
			printf("en cpu_finalizar\n");
			escribirEnArchivoLog("en finalizar", &CPULog,nombreLog);
			if (pcb->posicionStack==0)
			{	
				enviarDinamico(PCBFINALIZADO,socketKernel,pcb);
				escribirEnArchivoLog("envio pcb finalizado", &CPULog,nombreLog);
				send(socketKernel,&flagSignal,sizeof(int),0);
 				if (flagSignal)
 				{
 					exit(0);
 				}
				liberarContenidoPcb();
				free(pcb);
				primerAcceso=1;
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
			printf("en cpu_retornar\n");
			escribirEnArchivoLog("en retornar", &CPULog,nombreLog);
			t_almacenarBytes * bytes=malloc(sizeof(t_almacenarBytes));
			bytes->valor=calloc(1,sizeof(int));
			int rv;
			memcpy(bytes->valor,&retorno,sizeof(int));
			bytes->pid=PID;
			bytes->pagina=pcb->paginasCodigo+pcb->indiceStack[pcb->posicionStack].varRetorno.pagina;
			bytes->offset=pcb->indiceStack[pcb->posicionStack].varRetorno.offset;
			bytes->size=pcb->indiceStack[pcb->posicionStack].varRetorno.size;
			printf("este es el buffer : %i\n",*(int*)(bytes->valor) );
			enviarDinamico(ALMACENARBYTES,socketMemoria,bytes);
			escribirEnArchivoLog("envio almacenar bytes", &CPULog,nombreLog);
			while(0>recv(socketMemoria,&rv,sizeof(int),0)){
				perror("asd:");
			}
			if (rv==-1)
			{
				enviarDinamico(STACKOVERFLOW,socketKernel,1);
				escribirEnArchivoLog("envio stack overflow", &CPULog,nombreLog);
			}
			free(bytes->valor);
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
			printf("en cpu_wait\n");
			escribirEnArchivoLog("en wait", &CPULog,nombreLog);
		t_solicitudSemaforo * solicitudSemaforo;
		void * paquete;
		solicitudSemaforo=malloc(sizeof(t_solicitudSemaforo));
		solicitudSemaforo->tamanioIdentificador=strlen(identificador_semaforo);
		solicitudSemaforo->identificadorSemaforo=malloc(solicitudSemaforo->tamanioIdentificador);
		solicitudSemaforo->identificadorSemaforo=identificador_semaforo;
		solicitudSemaforo->estadoSemaforo=-1;
		enviarDinamico(SOLICITUDSEMWAIT,socketKernel,solicitudSemaforo);
		escribirEnArchivoLog("envio sem wait", &CPULog,nombreLog);
		free(solicitudSemaforo);
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
			printf("en cpu_signal\n");
			escribirEnArchivoLog("en signal", &CPULog,nombreLog);
		t_solicitudSemaforo * solicitudSemaforo;
		solicitudSemaforo=malloc(sizeof(t_solicitudSemaforo));
		solicitudSemaforo->tamanioIdentificador=strlen(identificador_semaforo);
		solicitudSemaforo->identificadorSemaforo=identificador_semaforo;
		solicitudSemaforo->estadoSemaforo=-1;
		enviarDinamico(SOLICITUDSEMSIGNAL,socketKernel,solicitudSemaforo);
		escribirEnArchivoLog("envio solicitud sem signal", &CPULog,nombreLog);
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
			t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
			printf("en cpu_reservar\n");
			escribirEnArchivoLog("en reservar", &CPULog,nombreLog);
			t_reservarEspacioMemoria * reservarEspacioMemoria;
			reservarEspacioMemoria=malloc(sizeof(t_reservarEspacioMemoria));
			reservarEspacioMemoria->espacio=espacio;
			printf("alocar%i\n",reservarEspacioMemoria->espacio );
			enviarDinamico(RESERVARESPACIO,socketKernel,reservarEspacioMemoria);
			escribirEnArchivoLog("envio reservar espacio", &CPULog,nombreLog);
			enviarDinamico(PCB,socketKernel,pcb);
			liberarContenidoPcb();
			escribirEnArchivoLog("envio pcb", &CPULog,nombreLog);
			t_reservar * reservar=malloc(sizeof(t_reservar));
			printf("cantidad Variables: %i.\n",pcb->indiceStack[pcb->posicionStack].cantidadVariables);
			while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0)){printf("asddsa\n");}
			recibirDinamico(RESERVADOESPACIO,socketKernel,reservar);
			printf("puntero: %i.\n", reservar->puntero);
			escribirEnArchivoLog("recibo reservado espacio", &CPULog,nombreLog);
			while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0)){printf("asddsa\n");}
			recibirDinamico(PCB,socketKernel,pcb);
			escribirEnArchivoLog("recibo pcb", &CPULog,nombreLog);
			printf("cantidad Variables: %i.\n",pcb->indiceStack[pcb->posicionStack].cantidadVariables);
			printf("puntero: %i.\n", reservar->puntero);
				
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
			
			t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
			printf("en cpu_liberar\n");
			escribirEnArchivoLog("en liberar", &CPULog,nombreLog);
			t_liberarMemoria * liberarMemoria;
			liberarMemoria=malloc(sizeof(t_liberarMemoria));
			liberarMemoria->direccionMemoria=puntero;
			enviarDinamico(LIBERARESPACIOMEMORIA,socketKernel,liberarMemoria);
			escribirEnArchivoLog("envio liberar espacio memoria", &CPULog,nombreLog);
			enviarDinamico(PCB,socketKernel,pcb);
			liberarContenidoPcb();
			escribirEnArchivoLog("envio pcb", &CPULog,nombreLog);
			while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0)){printf("asddsa\n");}
			recibirDinamico(PCB,socketKernel,pcb);
			escribirEnArchivoLog("recibo pcb", &CPULog,nombreLog);
			
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
			t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
			printf("en cpu_abrir\n");
			escribirEnArchivoLog("en abrir archivo", &CPULog,nombreLog);
			printf("flags: C=%i,L=%i,E=%i.\n", flags.creacion,flags.lectura,flags.escritura);
			printf("direccion: %s.\n", direccion);
					limpiar_string(&direccion);
					t_abrirArchivo * abrirArchivo;
					abrirArchivo=malloc(sizeof(t_abrirArchivo));
					abrirArchivo->direccionArchivo=malloc(strlen(direccion)+1);
					strcpy(abrirArchivo->direccionArchivo,direccion);
					abrirArchivo->tamanio=strlen(abrirArchivo->direccionArchivo);
					memcpy(&(abrirArchivo->flags),&flags,sizeof(t_banderas));
					enviarDinamico(ABRIRARCHIVO,socketKernel,abrirArchivo);
					escribirEnArchivoLog("envio abrir archivo", &CPULog,nombreLog);
					enviarDinamico(PCB,socketKernel,pcb);
					liberarContenidoPcb();
					escribirEnArchivoLog("envio pcb", &CPULog,nombreLog);
					free(abrirArchivo->direccionArchivo);
					free(abrirArchivo);
					while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0)){printf("asddsa\n");}
					recibirDinamico(PCB,socketKernel,pcb);
					escribirEnArchivoLog("recibo pcb", &CPULog,nombreLog);
					t_fdParaLeer * fdParaLeer= malloc(sizeof(t_fdParaLeer));
					while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0)){printf("asddsa\n");}
					recibirDinamico(ABRIOARCHIVO,socketKernel,fdParaLeer);
					escribirEnArchivoLog("recibo abrio archivo", &CPULog,nombreLog);
					printf("fd: %i\n",fdParaLeer->fd );
		
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
			t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
			printf("en cpu_borrar\n");
			escribirEnArchivoLog("en borrar archivo", &CPULog,nombreLog);
			t_borrarArchivo * borrarArchivo;
			borrarArchivo=malloc(sizeof(t_borrarArchivo));
			borrarArchivo->fdABorrar=descriptor_archivo;
			enviarDinamico(BORRARARCHIVO,socketKernel,borrarArchivo);
			escribirEnArchivoLog("envio borrar archivo", &CPULog,nombreLog);
			enviarDinamico(PCB,socketKernel,pcb);
			liberarContenidoPcb();
			escribirEnArchivoLog("envio pcb", &CPULog,nombreLog);
			while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0)){printf("asddsa\n");}
			recibirDinamico(PCB,socketKernel,pcb);
			escribirEnArchivoLog("recibo pcb", &CPULog,nombreLog);
			
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
			printf("en cpu_cerrar\n");
			escribirEnArchivoLog("en cerrar archivo", &CPULog,nombreLog);
					t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
			t_cerrarArchivo * cerrarArchivo;
					cerrarArchivo=malloc(sizeof(t_cerrarArchivo));
					cerrarArchivo->descriptorArchivo=descriptor_archivo;
					enviarDinamico(CERRARARCHIVO,socketKernel,cerrarArchivo);
					escribirEnArchivoLog("envio cerrar archivo", &CPULog,nombreLog);
					enviarDinamico(PCB,socketKernel,pcb);
					liberarContenidoPcb();
					escribirEnArchivoLog("envio pcb", &CPULog,nombreLog);
					while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0)){printf("asddsa\n");}
					recibirDinamico(PCB,socketKernel,pcb);
					escribirEnArchivoLog("recibo pcb", &CPULog,nombreLog);
				}
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
			t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
			printf("en moverCursor\n");
			escribirEnArchivoLog("en mover cursor", &CPULog,nombreLog);
			t_moverCursor * moverCursor;
			moverCursor=malloc(sizeof(t_moverCursor));
			moverCursor->descriptorArchivo=descriptor_archivo;
			moverCursor->posicion=posicion;
			enviarDinamico(MOVERCURSOR,socketKernel,moverCursor);
			escribirEnArchivoLog("envio mover cursor", &CPULog,nombreLog);
			enviarDinamico(PCB,socketKernel,pcb);
			liberarContenidoPcb();
			escribirEnArchivoLog("envio pcb", &CPULog,nombreLog);
			while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0)){printf("asddsa\n");}
			recibirDinamico(PCB,socketKernel,pcb);
			escribirEnArchivoLog("recibo pcb", &CPULog,nombreLog);
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
			t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
			printf("en cpu_escribir\n");
			escribirEnArchivoLog("en escribir archivo", &CPULog,nombreLog);
			printf("fd: %i\n", descriptor_archivo);
			t_escribirArchivo * escribirArchivo;
			escribirArchivo=malloc(sizeof(t_escribirArchivo));
			escribirArchivo->fdArchivo=descriptor_archivo;
			escribirArchivo->informacion=malloc(tamanio);
			memcpy(escribirArchivo->informacion,informacion,tamanio);
			printf("escribirArchivo->informacion: %s.\n",(char*) escribirArchivo->informacion);
			escribirArchivo->tamanio=tamanio;
			enviarDinamico(ESCRIBIRARCHIVO,socketKernel,escribirArchivo);
			escribirEnArchivoLog("envio escribir archivo", &CPULog,nombreLog);
			enviarDinamico(PCB,socketKernel,pcb);
			liberarContenidoPcb();
			escribirEnArchivoLog("envio pcb", &CPULog,nombreLog);
			while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0)){printf("asddsa\n");}
			recibirDinamico(PCB,socketKernel,pcb);
			escribirEnArchivoLog("recibo pcb", &CPULog,nombreLog);
			free(seleccionador);
			free(escribirArchivo->informacion);
			free(escribirArchivo);
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
			t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
			printf("en cpu_leer\n");
			escribirEnArchivoLog("en leer archivo", &CPULog,nombreLog);
			t_leerArchivo * leerArchivo;
			t_paqueteFS * paqueteFS;
			leerArchivo=malloc(sizeof(t_leerArchivo));
			printf("FD: %i\n", descriptor_archivo);
			leerArchivo->descriptor=descriptor_archivo;
			leerArchivo->tamanio=tamanio;
			enviarDinamico(LEERARCHIVO,socketKernel,leerArchivo);
			escribirEnArchivoLog("envio leer archivo", &CPULog,nombreLog);
			printf("paso l primero\n");
			enviarDinamico(PCB,socketKernel,pcb);
			liberarContenidoPcb();
			escribirEnArchivoLog("envio pcb", &CPULog,nombreLog);
			paqueteFS=malloc(sizeof(t_paqueteFS));
			while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0)){printf("asddsa\n");}
			recibirDinamico(PAQUETEFS,socketKernel,paqueteFS);
			escribirEnArchivoLog("recibo paquete fs", &CPULog,nombreLog);
			while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0)){printf("asddsa\n");}
			recibirDinamico(PCB,socketKernel,pcb);
			escribirEnArchivoLog("recibo pcb", &CPULog,nombreLog);
			t_almacenarBytes * bytes;
			bytes=malloc(sizeof(t_almacenarBytes));
			bytes->valor=calloc(1,tamanio);
			bytes->pid=PID;
			bytes->pagina=pcb->paginasCodigo+informacion/TAMPAGINA;
			bytes->offset=informacion%TAMPAGINA;
			bytes->size=tamanio;
			memcpy(bytes->valor,paqueteFS->paquete,tamanio);
			enviarDinamico(ALMACENARBYTES,socketMemoria,bytes);
			int rv;
			while(0>recv(socketMemoria,&rv,sizeof(int),0));
			if (rv==-1)
			{
				enviarDinamico(STACKOVERFLOW,socketKernel,1);
			}
			escribirEnArchivoLog("envio almacenar bytes", &CPULog,nombreLog);
			free(paqueteFS->paquete);
			free(paqueteFS);
			free(bytes->valor);
			free(bytes);
			free(leerArchivo);
			free(seleccionador);

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

void iniciarEjecucion(char *  linea){

		
		// getchar();
		// getchar();
		pthread_mutex_lock(&mutexPcb);
		pcb->programCounter++;
		pthread_mutex_unlock(&mutexPcb);
		
		printf("\t Evaluando -> %s", linea);
		analizadorLinea(linea, &functions, &kernel_functions);
		printf("ya esta?\n");
		

		//tengo que guardar en que linea estoy en el program counter para que cuando tuermine un quantum guardar ese contexto para que despues pueda seguir desde ahi
			
		
}

void conectarKernel(void){

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(IP_KERNEL,PUERTO_KERNEL,&hints,&serverInfo);
	socketKernel = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	void * buffer;
	int a=1;

	connect(socketKernel, serverInfo->ai_addr, serverInfo->ai_addrlen);

	
	freeaddrinfo(serverInfo);
	void * unBuffer=malloc(sizeof(int));
	handshakeCliente(socketKernel, CPU, &unBuffer);
	free(unBuffer);
	int recibir;
	t_seleccionador * seleccionador;
	char * linea;
	primerAcceso=1;
	int j,k;
	int rv;

t_peticionBytes * peticion;
while(1) {
	seleccionador=malloc(sizeof(t_seleccionador));
		if (primerAcceso){
					primerAcceso=0;}
		else
			enviarDinamico(ESPERONOVEDADES,socketKernel,NULL);
		escribirEnArchivoLog("envio espero novedades", &CPULog,nombreLog);
		while(0>recv(socketKernel,seleccionador, sizeof(t_seleccionador),0)){printf("asddsa\n");}
	printf("tipoPaquete: %i\n",seleccionador->tipoPaquete);
	
	switch (seleccionador->tipoPaquete){
		case PCB: 
							escribirEnArchivoLog("en case pcb", &CPULog,nombreLog);
							pcb=malloc(sizeof(t_pcb));
							recibirDinamico(PCB,socketKernel,pcb);
							if (pcb->indiceStack[pcb->posicionStack].cantidadVariables==0 && pcb->indiceStack[pcb->posicionStack].cantidadArgumentos==0)
							{
								PROXIMAPAG=0;
								PROXIMOOFFSET=0;
							}
							else if(pcb->indiceStack[pcb->posicionStack].cantidadArgumentos==0)
							{
								PROXIMAPAG=pcb->indiceStack[pcb->posicionStack].variables[(pcb->indiceStack[pcb->posicionStack].cantidadVariables)-1].pagina;
								PROXIMOOFFSET=(pcb->indiceStack[pcb->posicionStack].variables[(pcb->indiceStack[pcb->posicionStack].cantidadVariables)-1].offset)+4;
							}
							else if(pcb->indiceStack[pcb->posicionStack].cantidadVariables==0){
								PROXIMAPAG=pcb->indiceStack[pcb->posicionStack].argumentos[(pcb->indiceStack[pcb->posicionStack].cantidadArgumentos)-1].pagina;
								PROXIMOOFFSET=(pcb->indiceStack[pcb->posicionStack].argumentos[(pcb->indiceStack[pcb->posicionStack].cantidadArgumentos)-1].offset)+4;
							}
							else if(pcb->indiceStack[pcb->posicionStack].variables[(pcb->indiceStack[pcb->posicionStack].cantidadVariables)-1].pagina > pcb->indiceStack[pcb->posicionStack].argumentos[(pcb->indiceStack[pcb->posicionStack].cantidadArgumentos)-1].pagina)
 							{
 								PROXIMAPAG=pcb->indiceStack[pcb->posicionStack].variables[(pcb->indiceStack[pcb->posicionStack].cantidadVariables)-1].pagina;
 								PROXIMOOFFSET=(pcb->indiceStack[pcb->posicionStack].variables[(pcb->indiceStack[pcb->posicionStack].cantidadVariables)-1].offset)+4;
 							}
 							else if (pcb->indiceStack[pcb->posicionStack].variables[(pcb->indiceStack[pcb->posicionStack].cantidadVariables)-1].pagina < pcb->indiceStack[pcb->posicionStack].argumentos[(pcb->indiceStack[pcb->posicionStack].cantidadArgumentos)-1].pagina){
 								PROXIMAPAG=pcb->indiceStack[pcb->posicionStack].argumentos[(pcb->indiceStack[pcb->posicionStack].cantidadArgumentos)-1].pagina;
 							}
 							else
 							{
 								if (pcb->indiceStack[pcb->posicionStack].variables[(pcb->indiceStack[pcb->posicionStack].cantidadVariables)-1].offset > pcb->indiceStack[pcb->posicionStack].argumentos[(pcb->indiceStack[pcb->posicionStack].cantidadArgumentos)-1].offset)
 								{
 									PROXIMOOFFSET=(pcb->indiceStack[pcb->posicionStack].variables[(pcb->indiceStack[pcb->posicionStack].cantidadVariables)-1].offset)+4;
 								}
 								else{
 								PROXIMOOFFSET=(pcb->indiceStack[pcb->posicionStack].argumentos[(pcb->indiceStack[pcb->posicionStack].cantidadArgumentos)-1].offset)+4;
								}
 							}

 							
 							if (PROXIMOOFFSET==TAMPAGINA)
 							{
 								PROXIMOOFFSET=0;
 								PROXIMAPAG++;
 							}

							escribirEnArchivoLog("recibo pcb", &CPULog,nombreLog);
							PID=pcb->pid;
							printf("Proceso %i:\n", pcb->pid);
							printf("\tEstado: %i\n", pcb->estado);
							printf("\tPC: %i\n", pcb->programCounter);
							// printf("\tReferencia a tabla de archivos: %i\n", pcb->referenciaATabla);
							printf("\tPaginas de codigo: %i\n", pcb->paginasCodigo);
							printf("\tPosicion stack: %i\n", pcb->posicionStack);
							printf("\tCantidad de instrucciones: %i\n", pcb->cantidadInstrucciones);
							printf("\tIndice Codigo:\n");
							for (j = 0; j < pcb->cantidadInstrucciones; j++)
							{
								printf("\t\tInstruccion: %i,\tOffset: %i.\n", pcb->indiceCodigo[j].start, pcb->indiceCodigo[j].offset);
							}
							printf("\tCantidad de stack: %i\n", pcb->cantidadStack);
							printf("etiquetas_size: %i\n", pcb->indiceEtiquetas.etiquetas_size);
							printf("\tEtiquetas: %.*s\n", pcb->indiceEtiquetas.etiquetas_size,pcb->indiceEtiquetas.etiquetas);
							printf("\tExit Code: %i\n", pcb->exitCode);

							peticion=malloc(sizeof(t_peticionBytes));
							peticion->pid=PID;
					 		peticion->pagina=pcb->indiceCodigo[pcb->programCounter].start/TAMPAGINA;
							peticion->offset=pcb->indiceCodigo[pcb->programCounter].start;
							peticion->size=pcb->indiceCodigo[pcb->programCounter].offset+1;	
							peticion->offset=peticion->offset%TAMPAGINA;
							printf("%i,%i,%i,%i\n", peticion->pid,peticion->pagina,peticion->offset,peticion->size);		
							enviarDinamico(SOLICITUDBYTES,socketMemoria,(void *) peticion);
							escribirEnArchivoLog("envio solicitud bytes", &CPULog,nombreLog);
							while(0>recv(socketMemoria,&rv,sizeof(int),0)){
								perror("asd:");
							}
							if (rv==1)
							{
								linea=calloc(1,peticion->size+1);
								printf("esperandoLinea\n");
								while(0>recv(socketMemoria,linea,peticion->size,0)){
									perror("asd:");
								}
								linea=strcat(linea,"\0");
			 					iniciarEjecucion(linea);
							}
							else{
								enviarDinamico(PAGINAINVALIDA,socketKernel,1);
								escribirEnArchivoLog("envio pagina invalida", &CPULog,nombreLog);
							}
							free(peticion);
							free(linea);
		 					break;							
		case CONTINUAR:
							printf("en continuar\n");
							escribirEnArchivoLog("en case continuar", &CPULog,nombreLog);
						// printf("\tIndice Stack:\n");
						// for (j = 0; j < pcb->cantidadStack; j++)
						// {
						// 	printf("\t[%i]\n", j);
						// 	printf("\t\tCantidad Args: %i\n", pcb->indiceStack[j].cantidadArgumentos);
						// 	if (pcb->indiceStack[j].cantidadArgumentos!=0)
						// 	{
						// 		printf("\t\tArgumentos:\n");
						// 		for (k = 0; k < pcb->indiceStack[j].cantidadArgumentos; k++)
						// 		{
						// 			printf("\t\tId: %c,\t",pcb->indiceStack[j].argumentos[k].id);printf("Pagina: %i,\t",pcb->indiceStack[j].argumentos[k].pagina);printf("Offset: %i,\t",pcb->indiceStack[j].argumentos[k].offset);printf("Size: %i.\n",pcb->indiceStack[j].argumentos[k].size);
						// 		}
						// 	}
						// 	printf("\t\tCantidad Vars: %i\n", pcb->indiceStack[j].cantidadVariables);
						// 	if (pcb->indiceStack[j].cantidadVariables!=0)
						// 	{
						// 		printf("\t\tVariables:\n");
						// 		for (k = 0; k < pcb->indiceStack[j].cantidadVariables; k++)
						// 		{
						// 			printf("\t\tId: %c,\t",pcb->indiceStack[j].variables[k].id);printf("Pagina: %i,\t",pcb->indiceStack[j].variables[k].pagina);printf("Offset: %i,\t",pcb->indiceStack[j].variables[k].offset);printf("Size: %i.\n",pcb->indiceStack[j].variables[k].size);
						// 		}
						// 	}
						// 	if (j!=0)
						// 	{
						// 		printf("\t\tPosicion Retorno: %i\n", pcb->indiceStack[j].posRetorno);
						// 		printf("\t\tVariable Retorno:\n");
						// 		printf("\t\t\tPagina: %i,\t",pcb->indiceStack[j].varRetorno.pagina);printf("Offset: %i,\t",pcb->indiceStack[j].varRetorno.offset);printf("Size: %i.\n",pcb->indiceStack[j].varRetorno.size);
						// 	}	
						// }

							peticion=malloc(sizeof(t_peticionBytes));
							peticion->pid=PID;
					 		peticion->pagina=pcb->indiceCodigo[pcb->programCounter].start/TAMPAGINA;
							peticion->offset=pcb->indiceCodigo[pcb->programCounter].start;
							peticion->size=pcb->indiceCodigo[pcb->programCounter].offset+1;		
							peticion->offset=peticion->offset%TAMPAGINA;
							printf("%i,%i,%i,%i\n", peticion->pid,peticion->pagina,peticion->offset,peticion->size);	
							enviarDinamico(SOLICITUDBYTES,socketMemoria,(void *) peticion);
							escribirEnArchivoLog("envio solicitud bytes", &CPULog,nombreLog);
							while(0>recv(socketMemoria,&rv,sizeof(int),0)){
								perror("asd:");
							}
							if (rv==1)
							{
								linea=calloc(1,peticion->size+1);
								printf("esperandoLinea\n");
								while(0>recv(socketMemoria,linea,peticion->size,0)){
									perror("asd:");
								}
								linea=strcat(linea,"\0");
								printf("linea: %s.\n", linea);
								printf("linea: %i.\n",*(int*) linea);
			 					iniciarEjecucion(linea);
							}
							else{
								enviarDinamico(PAGINAINVALIDA,socketKernel,1);
								escribirEnArchivoLog("envio pagina invalida", &CPULog,nombreLog);
							}
							free(peticion);
							free(linea);
		break;

 		case FINALIZARPROCESO: 
 								escribirEnArchivoLog("en case finalizar proceso", &CPULog,nombreLog);
 								enviarDinamico(PCBFINALIZADOPORCONSOLA,socketKernel,pcb);
 								escribirEnArchivoLog("envio finalizar proceso", &CPULog,nombreLog);
 								send(socketKernel,&flagSignal,sizeof(int),0);
 								if (flagSignal)
 								{
 									exit(0);
 								}
 								liberarContenidoPcb();
 								free(pcb);
 								primerAcceso=1;
 		break;
 		case FINALIZARPORERROR: 
 								escribirEnArchivoLog("en case finalizar por error", &CPULog,nombreLog);
 								enviarDinamico(PCBERROR,socketKernel,pcb);
 								escribirEnArchivoLog("envio finalizar por error", &CPULog,nombreLog);
 								send(socketKernel,&flagSignal,sizeof(int),0);
 								if (flagSignal)
 								{
 									exit(0);
 								}
 								liberarContenidoPcb();
 								free(pcb);
 								primerAcceso=1;
 		break;
 		case FINQUANTUM: 		
 								escribirEnArchivoLog("en case fin de quantum", &CPULog,nombreLog);
 								enviarDinamico(PCBQUANTUM,socketKernel,pcb);
 								escribirEnArchivoLog("envio fin quamtum", &CPULog,nombreLog);
 								send(socketKernel,&flagSignal,sizeof(int),0);
 								if (flagSignal)
 								{
 									exit(0);
 								}
 								liberarContenidoPcb();
 								free(pcb);
 								primerAcceso=1;
 		break;
 		case PARAREJECUCION:
 								escribirEnArchivoLog("en case parar ejecucion", &CPULog,nombreLog);
 								enviarDinamico(PCBBLOQUEADO,socketKernel,pcb);
 								escribirEnArchivoLog("envio pcb bloquead", &CPULog,nombreLog);
 								send(socketKernel,&flagSignal,sizeof(int),0);
 								if (flagSignal)
 								{
 									exit(0);
 								}
 								liberarContenidoPcb();
 								free(pcb);	
 								primerAcceso=1;	
 		break;
}
free(seleccionador);
}
}	





 







//tiene que estar siempre en contacto con la memoria
//cuando se conecta con el kernel, queda a la espera de recibir el pcb de un proceso
//tiene que aumentar el program counter del PCB
//utilizo el indice de codigo para solicitar a memoria la proxima linea a ejecutar
//cuando la ejecuto se debe acutualizar los valores del programa en la memoria
//actualizar el program counter del PCB
//cuando termina todo el codigo de ejecutar avisarle al kernel para que libere ese espacio que ocupaba





void signal_handler(int signal){
	if (signal==SIGUSR1){
		flagSignal=1;
	}
}




int main(){
	struct sigaction sigact;
    sigact.sa_handler = signal_handler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGUSR1, &sigact, (struct sigaction *)NULL);


	t_config * CFG;
	CFG = config_create("cpuCFG.txt");
	IP_KERNEL=(char*) config_get_string_value(CFG ,"IP_KERNEL");
	IP_MEMORIA= (char*)config_get_string_value(CFG, "IP_MEMORIA");
	PUERTO_KERNEL= (char*)config_get_string_value(CFG ,"PUERTO_KERNEL");
	PUERTO_MEMORIA= (char*)config_get_string_value(CFG,"PUERTO_MEMORIA");
	printf("Configuración:\nIP_KERNEL = %s,\nIP_MEMORIA=%s,\nPUERTO_KERNEL = %s.\n,PUERTO_MEMORIA=%s. \n",IP_KERNEL,IP_MEMORIA,PUERTO_KERNEL,PUERTO_MEMORIA);
	
	system("clear");
	/*
	*
	*/
	char * nombreLog=calloc(1,200);
	strcpy(nombreLog,"logCPU-");
	horaActual=calloc(1,200);
	horaYFechaActual(&horaActual);
	printf("%s\n", horaActual);
	stripLog(&horaActual);
	printf("horaActual: %s\n", horaActual);
	strcat(horaActual,".txt");
	strcat(nombreLog,horaActual);
	printf("nombreLog: %s\n", nombreLog);
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(IP_MEMORIA,PUERTO_MEMORIA,&hints,&serverInfo);
    // Print pid, so that we can send signals from other shells
    printf("My pid is: %d\n", getpid());


	socketMemoria = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);


	connect(socketMemoria, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);
	void * unBuffer;
	unBuffer=malloc(sizeof(int));
	handshakeCliente(socketMemoria, CPU, &unBuffer);	
	CPULog=fopen(nombreLog,"w+");
	signal(SIGINT,cortar);
	recv(socketMemoria, &TAMPAGINA, sizeof(int), 0);




	// config_destroy(CFG);
	pthread_mutex_init(&mutexPcb,NULL);
	pthread_t conectarKernelA;
	printf("MARCO SIZE: %i\n", TAMPAGINA);
	pthread_create(&conectarKernelA, NULL, (void *) conectarKernel,NULL);
	pthread_join(conectarKernelA,NULL);
	free(pcb);
}


