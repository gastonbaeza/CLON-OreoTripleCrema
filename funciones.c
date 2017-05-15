#include "funciones.h"
#include "estructuras.h"
#include <commons/collections/list.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/txt.h>
#include <errno.h>
#define clear() printf("\033[H\033[J")
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stddef.h>
#include <openssl/md5.h>
#define OK 1
#define FAIL 0

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
void * limpiarPagina(int tamanioPagina,void * punteroAPagina)
{	int posicion=0;
	while(posicion<tamanioPagina)
	{	
	*punteroAPagina=0;
	&punteroAPagina++;
	posicion ++;
	}

}
int estaLibrePagina(int tamanioPagina,void * punteroAPagina)
{	int posicion=0;

	while(posicion<(tamanioPagina/4))
	{	
	if(*punteroAPagina==0)	
		{&punteroAPagina++;
		posicion++
		}	
	else return OCUPADO;	
	}
	return LIBRE;

}
int estaLibreMarco(int tamanioFrame, t_marco marco,int tamanioPagina)
{
		unaPagina=0;
		while(unaPagina<3)
		{
			if(estaLibrePagina(tamanioPagina,marco.numeroPagina[unaPagina]))
			{
				unaPagina++;
			}
			else{return OCUPADO	;}
		}
		return LIBRE;
}
int calcularPaginas(int tamanioPagina,int tamanio)
{
	double cantidadPaginas;
	int cantidadChains;
	int CantidadReal;
	cantidadPaginas=header->tamanio/tamPagina;
 	cantidadChains=ceil(cantidadPaginas);
 	cantidadReal=ceil((header->tamanio+cantidadChains*sizeof(unsigned int))/tamPagina);
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
int buscarPaginasLibres(int cantidadPaginas, t_list * paginasParaUsar,int tamanioFrame, t_marco marco,int tamanioPagina)
{	int resultado;
	
	
}	
/////////// LEEEEEEEMEEEEE ME OLVIDE DE HACER FREE DE LOS PAQUETES UNA VEZ QUE LOS MANDO A LA WEA/////

char * enviarDinamico(int unaInterfaz,int tipoPaquete,int UnSocket,void * paquete, int tamanioPaquete, t_header * header)
{ unsigned char unHash[MD5_DIGEST_LENGTH];
 	 MD5_CTX mdContext;	
 	 					header->seleccionador={unaInterfaz,tipoPaquete,tamanioPaquete};	
						envio=send(unSocket, header, 3*sizeof(int),0); 
						envio=send(unSocket,paquete,tamanioPaquete,0);
						MD5_Init (&mdContext);
	 					MD5_Update (&mdContext, paquete, tamanioPaquete);
	 					MD5_Final (unHash,&mdContext);
						return unHash;	 
}
char * recibirDinamico(int unSocket, void * paquete)
{	 unsigned char unHash[MD5_DIGEST_LENGTH];
 	 MD5_CTX mdContext;		
 	 int tamanioEstructura;			
								unPaquete=recv(unSocket,tamanioEstructura,sizeof(int),0);
								paquete=malloc(tamanioEstructura);
								unPaquete=recv(unSocket,paquete,tamanioEstructura,0);
	 							MD5_Init (&mdContext);
	 							MD5_Update (&mdContext, paquete, tamanioEstructura);
	 							MD5_Final (unHash,&mdContext);
								return unHash;	

}
int strlenConBarraN(char * unString){
	int cantidad=0
	while( *unString!= '\n'){cantidad++;unString=unString+sizeof(char);} //desplaza el puntero un char //TODO revisar si el string conserva las \n
	return cantidad;
}

t_programaSalida * obtenerPrograma( char * unPath){
	FILE * punteroAlArchivo;
	char * lineaDeCodigo;
	char * copiaLineaDecodigo;
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
						
						list_add(programa,lineaDeCodigo);
						
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
				
				memcpy(estructuraPrograma->elPrograma,&programaOut.unPrograma,dimensionPrograma*sizeof(char));
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