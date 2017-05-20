#include "funciones.h"
#include "estructuras.h"
#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/txt.h>
#include <errno.h>
#define clear() printf("\033[H\033[J")
#include <sys/time.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stddef.h>
#include <math.h>
#include <stdint.h>
#include <commons/config.h>

#define OK 1
#define FAIL 0

#define BACKLOG 5
#define KERNEL 0
	#define ARRAYPIDS 5
#define MEMORIA 1
	#define SOLICITUDMEMORIA 0
	#define SOLICITUDINFOPROG 1
	#define ESCRIBIRMEMORIA 2
	#define LIBERARMEMORIA 3
 	#define ACTUALIZARPCB 4
#define CONSOLA 2
	#define INICIARPROGRAMA 0
	#define FINALIZARPROGRAMA 1
	#define DESCONECTARCONSOLA 2
	#define LIMPIARMENSAJES 3
	//------------------------------	
	#define MENSAJES 0
	#define PID 1

#define CPU 3
#define FS 4
#define TRUE 1
#define FALSE 0
#define OK 1
#define FAIL 0
#define BLOQUE 20

void handshakeServer(int unSocket,int unServer, void * unBuffer)
{	
	
	recv(unSocket,unBuffer, sizeof(int),0);
	
	void * otroBuffer=malloc(sizeof(int));
	memcpy(otroBuffer,&unServer,sizeof(int));

	send(unSocket,otroBuffer,sizeof(int),0);
	

}
void handshakeCliente(int unSocket, int unCliente, void * unBuffer)
{

	void * otroBuffer=malloc(sizeof(int));
	memcpy(otroBuffer,&unCliente,sizeof(int));
	send(unSocket,otroBuffer, sizeof(int),0);
	recv(unSocket,unBuffer,sizeof(int),0);


}

void cargarPaginas(t_list * paginasParaUsar,int stackRequeridas, char * codigo, int tamPagina)
{
int unaPagina;
int tamanioCodigoChain;
t_chain * chainPrograma=malloc(tamPagina);
int paginasRequeridas=list_size(paginasParaUsar)-stackRequeridas;
tamanioCodigoChain=tamPagina-sizeof(unsigned int);
for ( unaPagina = 0; unaPagina < paginasRequeridas; unaPagina++)
{
	if(unaPagina==0)
	{ 	memcpy(chainPrograma->codigo,codigo,tamanioCodigoChain);} //si es la primera apunto al comienzo de lo que copie

	else
	{memcpy((void *)chainPrograma->codigo,&codigo+(tamanioCodigoChain*unaPagina),tamanioCodigoChain);} //si no es la primera tengo que desplazarme en funcion del tamaño de la pagina
	if(unaPagina+1==paginasRequeridas)
		{chainPrograma->chain=NULL;}
	else{chainPrograma->chain=list_get(paginasParaUsar,unaPagina+1);}
memcpy((void *)list_get(paginasParaUsar,unaPagina),chainPrograma,tamPagina);

}

for(unaPagina=0;unaPagina<stackRequeridas;unaPagina++)
{
	memset((void *)chainPrograma->codigo,1,tamPagina-sizeof(unsigned int));
	chainPrograma->chain=list_get(paginasParaUsar,unaPagina+1);
	memcpy((void *)listget(paginasParaUsar,unaPagina),(void *)chainPrograma,tamPagina);
}
free(chainPrograma);
}




int calcularPaginas(int tamanioPagina,int tamanio)
{
	double cantidadPaginas;
	int cantidadChains;
	int cantidadReal;
	cantidadPaginas=tamanio/tamanioPagina;
 	cantidadChains=ceil(cantidadPaginas);
 	cantidadReal=ceil((tamanio+cantidadChains*sizeof(unsigned int))/tamanioPagina);
 		if((ceil(cantidadPaginas))<cantidadReal)
 			{ 	
				cantidadPaginas=floor(cantidadPaginas ++);
 			}
 			else
 			{
 				cantidadPaginas= ceil(cantidadPaginas);					
 			}
 			return cantidadPaginas;
 							
}
int buscarPaginas(int paginasRequeridas, t_list * paginasParaUsar,  t_marco * asignadorSecuencial, t_marco * marcos,int MARCOS,unsigned int tamanioAdministrativas)
{	int cantidadPaginas=0;
	
	while(cantidadPaginas<paginasRequeridas || (void *)asignadorSecuencial!= marcos[MARCOS-tamanioAdministrativas-1].numeroPagina[3])
	{	
		if(((void *)asignadorSecuencial)==(asignadorSecuencial->numeroPagina[3]))
		{	list_add(paginasParaUsar, (void *)asignadorSecuencial);
			asignadorSecuencial=asignadorSecuencial+sizeof(int)+sizeof(void *); //no me pregunten solo soy una chica//es porque se recorre secuencialmente y el int me rompe las pelotas
		}
		else
		{	
			list_add(paginasParaUsar,(void *)asignadorSecuencial);
			asignadorSecuencial=asignadorSecuencial+sizeof(void *);
		}

		cantidadPaginas++;

	}
	if((void *)asignadorSecuencial!= marcos[MARCOS-tamanioAdministrativas-1].numeroPagina[3])
		//{usarElOtroAlgoritmo();}
	{return FAIL;}
	else{return OK;}
}	

int buscarAdministrativa(t_solicitudInfoProg * infoProg,t_pcb * unPcb, t_estructuraADM * bloquesAdministrativas,int MARCOS)
{

	int pid;
	pid=infoProg->pid;
	int marcosRecorridos;
	while(marcosRecorridos<MARCOS)
	{
		if(bloquesAdministrativas->proceso.pid==pid)
			{memcpy((void *)unPcb,&(bloquesAdministrativas->proceso),sizeof(t_pcb))	;return OK;}

		else{bloquesAdministrativas=bloquesAdministrativas+sizeof(t_estructuraADM);}
		marcosRecorridos++;
	}
	return FAIL;
}
/////////// LEEEEEEEMEEEEE ME OLVIDE DE HACER FREE DE LOS PAQUETES UNA VEZ QUE LOS MANDO A LA WEA/////

void enviarDinamico(int unaInterfaz,int tipoPaquete,int unSocket,void * paquete, int tamanioPaquete)
{ 
	t_header * header;
 	
 	 					header->seleccionador.unaInterfaz=unaInterfaz;
 	 					header->seleccionador.tipoPaquete=tipoPaquete;
 	 					header->tamanio=tamanioPaquete;
						send(unSocket, header, 3*sizeof(int),0); 
						send(unSocket,paquete,tamanioPaquete,0);
						
						
}
void recibirDinamico(int unSocket, void * paquete, int tamanioEstructura)
{	 
 	 		
								recv(unSocket,paquete,tamanioEstructura,0);
	 							

}
int strlenConBarraN(char * unString){
	int cantidad=0;
	while( *unString!= '\n'){cantidad++;unString=unString+sizeof(char);} //desplaza el puntero un char //TODO revisar si el string conserva las \n
	return cantidad;
}

t_programaSalida * obtenerPrograma( char * unPath){
	FILE * punteroAlArchivo;
	char * lineaDeCodigo;
	char * copiaLineaDecodigo;
	int dimensionPrograma;
	int tamanioLinea=0;
	t_list * programa;
	int tamanioLista;
	t_programa  programaOut;
	t_programaSalida * estructuraPrograma;
	lineaDeCodigo=malloc(100*sizeof(char));
	if((punteroAlArchivo=fopen(unPath,"r"))==NULL)
		{
			fflush(stdout); 
			printf("el archivo no existe" ); 
		}
	
	else{		
				programa=list_create();
				//semaforo
				while(!feof(punteroAlArchivo))
						{
						fscanf(punteroAlArchivo,"%s\n",lineaDeCodigo);
						
						list_add(programa,(void *)lineaDeCodigo);
						
					    }
				//
			    tamanioLista=list_size(programa);
			    programaOut.dimension=tamanioLista;

			    int posicion;
			    for (posicion = 0; posicion < tamanioLista; posicion++)  //hay otra forma de maloquear una estructura.... es tan compleja e precisa como un jogo de ajedrez bien jugado https://www.youtube.com/watch?v=Ba8NfkYC5ss
			    {
			    	copiaLineaDecodigo=list_get(programa,posicion);
			    	tamanioLinea=strlenConBarraN(copiaLineaDecodigo)+tamanioLinea;

			    }
			    dimensionPrograma=tamanioLinea; //para que la wea sea coherente, queda horrible que el tamanio de un programa sea "el tamanio de la linea"
			    programaOut.unPrograma=malloc(dimensionPrograma*sizeof(char));
				for (posicion = 0; posicion < tamanioLista; posicion++)
				{	
					programaOut.unPrograma[posicion]=list_get(programa,posicion); 
					
				}
				estructuraPrograma->elPrograma=malloc(dimensionPrograma*sizeof(char)+sizeof(int));
				
				memcpy((void *)estructuraPrograma->elPrograma,&(programaOut.unPrograma),dimensionPrograma*sizeof(char));
				estructuraPrograma->tamanio=dimensionPrograma*sizeof(char)+sizeof(int);

				return estructuraPrograma;	


			}
							      }


/*
case KERNEL:
			switch (tipoPaquete){
			case MENSAJE:// recibe mensajes para mandar por eco a otro cliente
								return recibirDinamico(unSocket,paquete);
	 					break;
	 		case PATH: // fijarse si hay memoria para ejectutar el proceso, espera un tam pagina, si hay espacio, lo manda a cpu, sino manda una excepcion a consola
	 					
	 							return recibirDinamico(unSocket,paquete);
	 					break;
	 		case TAMPAGINA: // dice el tam de paagina al comienzo de la memoria
	 							return recibirDinamico(unSocket,paquete);

	 					break;
	 		case PIDFinalizacion:
	 							return recibirDinamico(unSocket,paquete);
	 					break;
	 		case PIDINFO:		return recibirDinamico(unSocket,paquete);
	 					break;
	 		case PAGINAS: // creo que es igual que tampag, porque cpu pide a memoria, memoria pide al kernel y le dice, che quiero usar esto de heap para este proceso, puedo? ma vale perro/ quien te juna memoria
	 							return recibirDinamico(unSocket,paquete);
	 					break;
	 		case FLAGS:		
	 							return recibirDinamico(unSocket,paquete);
	 					break;
	 		case NOTIFICACION: //notificacion de fs a pedido de kernel
	 							return recibirDinamico(unSocket,paquete);
	 					break;

	 		case EXCEPCION: // fs y de memoria
	 							return recibirDinamico(unSocket,paquete);
	 					break;
	 		case SYSCALL: // devuelve el codigo de la syscall para saber como atenderla
	 							return recibirDinamico(unSocket,paquete);

	 					break;
	 		case FD:  //ESTO TIENE QUE ENVIAR A CONSOLA ERO NO ACA, EN EL KERNEL ups mayus*
	 							return recibirDinamico(unSocket,paquete);
	 					break;
	 		case RESPUESTAOKMEMORIA:
	 							return recibirDinamico(unSocket,paquete);
	 		break;
	 		case LIBERARMEMORIA:
	 							return recibirDinamico(unSocket,paquete);

	 		break;	}
 							
 
case MEMORIA: //pagprogstack va en memoria o en kernel?
		switch (tipoPaquete){
		case SOLICITUDMEMORIA: // [Identificador del Programa] [Páginas requeridas]// paginas necesarias para guardar el programa y el stack
							return recibirDinamico(unSocket,paquete);
 					break;
 		case CODIGO: // codigo del programa
 							return recibirDinamico(unSocket,paquete);
 					break;
 		case SOLICITUDINFOPROG:// informacion del programa en ejecucion (memoria)
							return recibirDinamico(unSocket,paquete);					
 					break;
 		case ESCRIBIRMEMORIA:
 							return recibirDinamico(unSocket,paquete);
 					break;
 		case LIBERARMEMORIA:
 							return recibirDinamico(unSocket,paquete);
 									
 					break;

			 }
			 
case CONSOLA:
			switch (tipoPaquete){
 		case MENSAJECONSOLA:// recibe mensajes para imprimirlos por pantalla
 							return recibirDinamico(unSocket,paquete);
 					break;
 		
					}


	
case CPU:
		switch (tipoPaquete){
		
 		case PCB: // recibe el PCB de un programa para ejecutarlo
 							return recibirDinamico(unSocket,paquete);
 					break;	
 		case INFOPROG:
 							return recibirDinamico(unSocket,paquete);
 		break;			
				}
		
case FS:	
 		switch (tipoPaquete){
 			case INTERACTUARARCHIVO: //validar leer crear borrar:// peticion del kernel para leer el archivo
 							return recibirDinamico(unSocket,paquete);
 					break;
 			case OBTENERDATOS: //: [Path, Offset, Size, Buffer]
 							return recibirDinamico(unSocket,paquete);
 			break;
 			case GUARDARDATOS: // [Path, Offset, Size, Buffer]
 							return recibirDinamico(unSocket,paquete);
 		 		}
 	
 
 	
 
 					}


}
*/