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
#include <time.h>

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
 	#define PROGRAMASALIDA 5
#define CONSOLA 2
	#define INICIARPROGRAMA 0
	#define FINALIZARPROGRAMA 1
	#define DESCONECTARCONSOLA 2
	#define LIMPIARMENSAJES 3
	//------------------------------	
	#define MENSAJES 0
	#define PID 1
	#define PCB 11
	#define PATH 3

#define CPU 3
#define FS 4
#define TRUE 1
#define FALSE 0
#define OK 1
#define FAIL 0
#define LIBRE 1
#define OCUPADO 0
#define BLOQUE 20


void horaYFechaActual (char horaActual[19]) {
    time_t tiempo = time(0);      //al principio no tengo ningún valor en la variable tiempo
    struct tm *fechaYHora = localtime(&tiempo);


    strftime(horaActual, 128, "%d/%m/%Y %H:%M:%S", fechaYHora); 
     //string-format-time = formato de tiempo a asignarse a la cadena

}

void handshakeServer(int unSocket,int unServer, void * unBuffer)
{	
	
	// while(0>=recv(unSocket,unBuffer,sizeof(int),0));

	recv(unSocket,unBuffer,sizeof(int),0);
	
	void * otroBuffer=malloc(sizeof(int));
	memcpy(otroBuffer,&unServer,sizeof(int));

	send(unSocket,otroBuffer,sizeof(int),0);
	

}
void handshakeCliente(int unSocket, int unCliente, void * unBuffer)
{

	void * otroBuffer=malloc(sizeof(int));
	memcpy(otroBuffer,&unCliente,sizeof(int));
	send(unSocket,otroBuffer, sizeof(int),0);
	// while(0>=recv(unSocket,unBuffer,sizeof(int),0));

	recv(unSocket,unBuffer,sizeof(int),0);


}
 void buscarProcesosActivos(t_list * procesosActivos, t_estructuraADM * bloquesAdmin, int MARCOS)
 {
 	int unaAdmin;
 	for (unaAdmin= 0; unaAdmin < MARCOS; unaAdmin++)
 	{
		if(bloquesAdmin[unaAdmin].pid!=-1)
			{ list_add(procesosActivos,(void*)bloquesAdmin[unaAdmin].pid);}

	}
}
void generarDumpAdministrativas(t_estructuraADM * bloquesAdmin, int MARCOS)
{
	t_list * procesosActivos;
	procesosActivos=list_create();
	buscarProcesosActivos(procesosActivos,bloquesAdmin,MARCOS);
	int unaAdmin;
	for (unaAdmin= 0; unaAdmin < MARCOS; unaAdmin++)
	{fflush(stdout);printf("%s","tabla de paginas");
		fflush(stdout);printf("%i",bloquesAdmin[unaAdmin].frame);
		fflush(stdout);printf("%i",bloquesAdmin[unaAdmin].pid);
		fflush(stdout);printf("%i",bloquesAdmin[unaAdmin].hashPagina);
		fflush(stdout);printf("%i",bloquesAdmin[unaAdmin].estado);

	}
	int cantidadProcesos=list_size(procesosActivos);
	int unProceso;
	t_list * proceso;
	proceso=list_create();
	for (unProceso = 0; unProceso < cantidadProcesos; unProceso++)
	{	proceso=list_get(procesosActivos,unProceso);
		// fflush(stdout);printf("procesos activos: %i",proceso);
	}
}

int cantidadBloquesLibres(int MARCOS, t_estructuraADM * bloquesAdmin)
{	int cantidadBloques=0;
	int unaAdmin;
	for (unaAdmin= 0; unaAdmin < MARCOS; unaAdmin++)
	{
		if(bloquesAdmin[unaAdmin].estado==LIBRE){cantidadBloques++;}	
	}
	return cantidadBloques;
}

int cantidadBloquesOcupados(int MARCOS, t_estructuraADM * bloquesAdmin)
 {	int cantidadBloques=MARCOS-cantidadBloquesLibres(MARCOS,bloquesAdmin);
 	return cantidadBloques;
}


int hayPaginasLibres(int unaCantidad, t_estructuraADM * bloquesAdmin, int MARCOS)
{
int encontradas=0;
int unBloque=0;
while(encontradas<unaCantidad)
	{
		if(bloquesAdmin[unBloque].estado==LIBRE)
		{
			encontradas++;
		}
		unBloque++;
		if(unBloque == MARCOS)
			{return FAIL;}
	}	
return OK;
	
}
void cargarPaginas(t_list * paginasParaUsar,int stackRequeridas, char * codigo, int marco_size)
{
int unaPagina;
int desplazamiento=0;
int paginasRequeridas=list_size(paginasParaUsar)-stackRequeridas;
for ( unaPagina = 0; unaPagina < paginasRequeridas; unaPagina++)
{
memcpy((void *)list_get(paginasParaUsar,unaPagina),codigo+desplazamiento,marco_size);
desplazamiento+=marco_size;
}

for(unaPagina=0;unaPagina<stackRequeridas;unaPagina++)
{
	char * stack=calloc(marco_size,marco_size);
	memcpy((void *)list_get(paginasParaUsar,unaPagina),stack,marco_size);
}

}




int calcularPaginas(int tamanioPagina,int tamanio)
{
	double cantidadPaginas;
	int cantidadChains;
	int cantidadReal;
	cantidadPaginas=ceil(tamanio/tamanioPagina);
 	 return cantidadPaginas;
 							
}
 int buscarPaginas(int paginasRequeridas, t_list * paginasParaUsar, int MARCOS, t_estructuraADM * bloquesAdmin, t_marco * marcos)
 {	int cantidadPaginas=0;
 	int paginasRecorridas=0;
 	 time_t tiempo;
    srand((unsigned) time(&tiempo));
    int unFrame;
    while(cantidadPaginas<paginasRequeridas && paginasRecorridas<MARCOS)
    {
    unFrame=rand() % MARCOS;

    if(bloquesAdmin[unFrame].estado==LIBRE)		
    	{
    		list_add(paginasParaUsar,(marcos[unFrame]).numeroPagina);
    		bloquesAdmin[unFrame].estado==OCUPADO;
    		cantidadPaginas++;
    	}
    	paginasRecorridas++;
    } return OK;
 }	

int buscarAdministrativa(int pid,t_pcb * unPcb, t_estructuraADM * bloquesAdministrativas,int MARCOS)
{

	
	int marcosRecorridos;
	while(marcosRecorridos<MARCOS)
	{
		if(bloquesAdministrativas->pid==pid)
			{memcpy((void *)unPcb,&(bloquesAdministrativas->pid),sizeof(t_pcb))	;return OK;}

		else{bloquesAdministrativas=bloquesAdministrativas+sizeof(t_estructuraADM);}
		marcosRecorridos++;
	}
	return FAIL;
}
/////////// LEEEEEEEMEEEEE ME OLVIDE DE HACER FREE DE LOS PAQUETES UNA VEZ QUE LOS MANDO A LA WEA/////
void dserial_string(char * unString,int unSocket)
{	int tamanio;
	int  unChar;
	int * buffer1=malloc(sizeof(int));
	int b=1;
	memcpy(buffer1,&b,sizeof(int));

	while(0>recv(unSocket,&tamanio,sizeof(int),0));
	unString=realloc(unString,tamanio);
	send(unSocket,buffer1, sizeof(int),0);
	for (unChar= 0; unChar <tamanio; unChar++)
	{
		while(0>=recv(unSocket, &unString[unChar],sizeof(char),0));
		send(unSocket,buffer1, sizeof(int),0);

	}

free(buffer1);

}
void serial_string(char * unString,int tamanio,int unSocket)
{	int  unChar;
	int * buffer=malloc(sizeof(int));
	
	int a=1;
	memcpy(buffer,&a,sizeof(int));

	send(unSocket,&tamanio,sizeof(int),0);

	while(0>=recv(unSocket,buffer, sizeof(int),0));
	for (unChar= 0; unChar < tamanio; unChar++)
	{
		send(unSocket, &unString[unChar],sizeof(char),0);
		while(0>=recv(unSocket,buffer, sizeof(int),0));
	}
	free(buffer);
}
void dserial_tablaArchivosDeProcesos(t_tablaArchivosDeProcesos * tablaProcesos, int unSocket)
{
	int  unChar;
	int * buffer1=malloc(sizeof(int));
	int b=1;
	memcpy(buffer1,&b,sizeof(int));

	while(0>recv(unSocket, &(tablaProcesos->descriptor),sizeof(int),0));
	send(unSocket,buffer1, sizeof(int),0);
	while(0>recv(unSocket, &(tablaProcesos->flag),sizeof(int),0));
	send(unSocket,buffer1, sizeof(int),0);
	while(0>recv(unSocket, &(tablaProcesos->posicionTablaGlobal),sizeof(int),0));free(buffer1);
}
void dserial_tablaGlobalArchivos(t_tablaGlobalArchivos * tablaGlobalArchivos, int unSocket)
{	int * buffer=malloc(sizeof(int));
	int a=1;
	memcpy(buffer,&a,sizeof(int));
	while(0>recv(unSocket,&(tablaGlobalArchivos->vecesAbierto),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	dserial_string(tablaGlobalArchivos->path,unSocket); free(buffer);
}

void serial_tablaGlobalArchivos(t_tablaGlobalArchivos * tablaGlobalArchivos, int unSocket)
{	int * buffer=malloc(sizeof(int));
	int a=1;
	memcpy(buffer,&a,sizeof(int));
	send(unSocket,&(tablaGlobalArchivos->vecesAbierto),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	serial_string(tablaGlobalArchivos->path,tablaGlobalArchivos->tamanioPath,unSocket); free(buffer);
}
void serial_pcb(t_pcb * pcb, int unSocket)
{	int * buffer=malloc(sizeof(int));
	int a=1;
	memcpy(buffer,&a,sizeof(int));
	send(unSocket,&(pcb->pid),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->programCounter),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->estado),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->referenciaATabla),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->paginasCodigo),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->posicionStack),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->indiceCodigo),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->indiceEtiquetas),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->exitCode),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));free(buffer);
}

void dserial_pcb(t_pcb* pcb, int unSocket)
{	
	int * buffer=malloc(sizeof(int));
	int a=1;
	memcpy(buffer,&a,sizeof(int));
	while(0>recv(unSocket,&(pcb->pid),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->programCounter),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->estado),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->referenciaATabla),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->paginasCodigo),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->posicionStack),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->indiceCodigo),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->indiceEtiquetas),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->exitCode),sizeof(int),0));
        send(unSocket,buffer, sizeof(int),0);free(buffer);
}
void dserial_programaSalida(t_programaSalida * programaSalida, int unSocket)
{	
	dserial_string(programaSalida->elPrograma,unSocket);}

void serial_programaSalida(t_programaSalida * programaSalida, int unSocket)
{
	
	serial_string(programaSalida->elPrograma,programaSalida->tamanio,unSocket); 
}
void dserial_path(t_path * path, int unSocket)
{	
	dserial_string(path->path,unSocket);}

void serial_path(t_path * path, int unSocket)
{
	
	serial_string(path->path,path->tamanio,unSocket); 
}
void serial_tablaArchivosDeProcesos(t_tablaArchivosDeProcesos * tablaProcesos, int unSocket)
{
	int  unChar;
	int * buffer=malloc(sizeof(int));
	int b=1;
	memcpy(buffer,&b,sizeof(int));
	send(unSocket, &(tablaProcesos->descriptor),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket, &(tablaProcesos->flag),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket, &(tablaProcesos->posicionTablaGlobal),sizeof(int),0);
	free(buffer);
}
void serial_solicitudMemoria(t_solicitudMemoria * solicitud,int  unSocket)
	{	int * buffer=malloc(sizeof(int));
	int a=1;
	memcpy(buffer,&a,sizeof(int));
	send(unSocket,&(solicitud->tamanioCodigo),sizeof(int),0);

	while(0>=recv(unSocket,buffer, sizeof(int),0));
	
	
	serial_string(solicitud->codigo,solicitud->tamanioCodigo,unSocket);
	

	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(solicitud->cantidadPaginasCodigo),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(solicitud->cantidadPaginasStack),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(solicitud->pid),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(solicitud->respuesta),sizeof(int),0);
free(buffer);
	

	
}

void dserial_solicitudMemoria(t_solicitudMemoria * solicitud, int unSocket)
{
	char * code=malloc(sizeof(char)*100);
	int * buffer=malloc(sizeof(int));
	int b=1;
	memcpy(buffer,&b,sizeof(int));

	while(0>=recv(unSocket,&(solicitud->tamanioCodigo),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	solicitud->codigo=malloc(solicitud->tamanioCodigo*sizeof(char));
	dserial_string(solicitud->codigo,unSocket);
	send(unSocket,buffer, sizeof(int),0);

	while(0>=recv(unSocket,&(solicitud->cantidadPaginasCodigo),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>=recv(unSocket,&(solicitud->cantidadPaginasStack),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>=recv(unSocket,&(solicitud->pid),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>=recv(unSocket,&(solicitud->respuesta),sizeof(int),0));
		
	free(buffer);
}

void enviarDinamico(int tipoPaquete,int unSocket,void * paquete)
{ 
	switch(tipoPaquete){
		case SOLICITUDMEMORIA:
			serial_solicitudMemoria((t_solicitudMemoria *)paquete,unSocket);
		break;

		case PROGRAMASALIDA:	//este tambien sirve cuando queremos mandar un string con su tamaño
			serial_programaSalida((t_programaSalida * )paquete,unSocket);			
		break;

		case PATH:	
			serial_path((t_path * )paquete,unSocket);			
		break;

		case PCB:	
			serial_pcb((t_pcb *)paquete,unSocket);
		break;

		default : fflush(stdout); printf("%s\n","el paquete que quiere enviar es de un formato desconocido"); 
		// pagaraprata();
		break;

		
	}
						
}
void recibirDinamico(int tipoPaquete,int unSocket, void * paquete)
{	
	switch(tipoPaquete){
		case SOLICITUDMEMORIA:
			dserial_solicitudMemoria(paquete,unSocket);
		break;
		case PROGRAMASALIDA:	//este tambien sirve cuando queremos mandar un string con su tamaño
					dserial_programaSalida((t_programaSalida * )paquete,unSocket);			
		break;

		case PATH:	
					dserial_path((t_path * )paquete,unSocket);			
		break;

		case PCB:	
					dserial_pcb((t_pcb *)paquete,unSocket);
		break;

		default : fflush(stdout); printf("%s\n","el paquete que quiere enviar es de un formato desconocido"); 
		// pagaraprata();
		break;

		
	}
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

 void pagaraprata()
{

fflush(stdout);printf("%s\n","+''#########++''++'+####+'+++++'####++''++''####+'++'+#########'+++++'#####+++++++");                                                                                            
fflush(stdout);printf("%s\n","+++:`````` `.#++++++````#+++++++ ````+''+++++``` #+'+# `       ++++++'+``` #++++++");                                                                                            
fflush(stdout);printf("%s\n","+++:`````` ```+'+++,````++++++'` `````+'++++,```.#+++#````  ``  '++++',````##+++++");                                                                                            
fflush(stdout);printf("%s\n","+++,````@+````##++#`````:@+++#``` +````@+++# ````:@++# ``` . ` `.#+''#`````:@+++++");                                                                                            
fflush(stdout);printf("%s\n","+++,````@@````+@+'#``````@#++'.```@:```@#++#``````@#+@ ```,@``` `@#'+@```` `@#++++");                                                                                            
fflush(stdout);printf("%s\n","+++,````@@``` '@+''``````@#'+`````@:`  #@++;`.``` @#+#````.@.``` @@++'.`````@#++++");                                                                                            
fflush(stdout);printf("%s\n","+++,````@@````'@#+. :````#@+#`````#@+@@@@++.`:.```@@'#`````@,`` `@@++.`:```.@@++++");                                                                                            
fflush(stdout);printf("%s\n","+++,````@#````@@##``+````'@##`````@@+@@@@+# `+````;#'#```` @````'@@+#``+``` ;@++++");                                                                                            
fflush(stdout);printf("%s\n","+++,`````` `` @@#@.:@````.@+@`````;``` :++#`:@ ```.@+#```` `  `+@@@'#`:#``` .@++++");                                                                                            
fflush(stdout);printf("%s\n","+++,`````````;@@+'.## ````@#@`````;````:++'`+@```` @#@``````` ` +@#++.+@ ````@#+++");                                                                                            
fflush(stdout);printf("%s\n","+++,````````'#@@#.`:'  `` @#@`````+.```:@#.`;;```` @#@````````` ,#++.`;;`````@#+++");                                                                                            
fflush(stdout);printf("%s\n","+++,````@@@@@@@## ``````` '@#`````@;```'@@```` ````'@@ ````@```` @##`` `` ```'@'++");                                                                                            
fflush(stdout);printf("%s\n","+++,````@@#####+#```` ````.@+,````@;```+@#` ```.```,@@ ``` @,``` @##`` ``````,@+++");                                                                                            
fflush(stdout);printf("%s\n","+++,````@@++++++'`#@@# ````###`.` #:`  @@;`+@#@```` @@`````@,``` @@'`+@@@ ````@#'+");                                                                                            
fflush(stdout);printf("%s\n","+++,````@@++++++.`@@@@` `` @@#  ``.` `:@@.`@@@@```  @@`````@,``` @@.`@@@@```` @#'+");                                                                                            
fflush(stdout);printf("%s\n","+++,````@@+++'+#` @@#+,``` #@+#````` `@@@``#@#+,````#@`````@,``` @@  @@#+,````#@++");                                                                                            
fflush(stdout);printf("%s\n","++':````@@+'+'+#`.@@+';.``.;@+++. ` :@@@@`,@@+''```.;#`` `.@;`   +# .@@++;```.;#++");                                                                                            
fflush(stdout);printf("%s\n","++++'#@@@@++++++'#@@++''@@##@+++#@@@#@@#+'#@@++++@@@@@##@@@@#+@@@@@##@@++'+@@@@@#+");                                                                                            
fflush(stdout);printf("%s\n","+++'+#####++++''+###+'++######''++#@@##+'++##++++#####++#####+##@###+##++++#####+'");                                                                                            
fflush(stdout);printf("%s\n","++++++'+++'+++++++++++++'+++++++'+++++++++'+'+''++++++''+++++++++''+++'+++''++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++++++++++++++++''+++'+++++++++'+'++'+++++'++++++++++++++++++'++'+++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++++++++++++++++++++++++++++++++'++++'+++++++++++++'++++++++++++++++++++++");                                                                                            
fflush(stdout);printf("%s\n","+++++++++''++++++++++++++++++++++++++++''''+++++++++++++'+++'+++++'+++++++++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++'+++++++++++'++#+++++++++'++++++++#+++++++++++++++++++++++++++++++++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++````` ``.#'+;      ``,#'+++'+:```,+'++,`      ` .++'+#..` +'++++++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++```` ``   #+;`````` ` .+++++#```` #+'+,``````````+++'#````:+++++++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++'+ ```.;.```.#;``    ` ``+++++#`````@#'+.````````` @#++'```` @#+++++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++'+`````@;``  @;````++ ` `;@++++`````'@++';'`````';;@@++``````@#+++++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++`````@'.`` @'`` `'@````:@#++,`````:@++++#`````@@@@@+#``````##+++++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++`````@;````@;````;@````,@#+#    ```@#'+++`````@@#+#+#`;.```,@+++++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++`````@;``` @;````'@ `` +@#'#.:```` @#+'++`````@@+'+':`@`````#+++++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++'+ ````:`````@'````,. ``+@@#+;`#:````#@++++.````@@++++``@ ````@#++++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++ ````.````;@;``` `````;@@++.`@#````;@++++`````@@+++# `@,````##++++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++``` ` `` ;@@;````````` ##+#``@@`````#++++`````@@++++.;@;````+@++++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++`````@@@@@@@;````,, ` `,#+#````.``` @#+++.````@@+++;.```````.@+'++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++```` @@@@@##;````;@ ```,@#'`````````##+++.````@@+'+````` ``` @#'++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++```` @@+++++;````'@ ```,@@.`;'':````'@+++.````@@++#``;''`````@#'++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++```` @@++++';````'@ `` :@@  @@@#`````@+++.````@@++#.;@@@:````+@+++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++```` @@++++';````'@ ```.@@`,@@#@```  @#++.````@@++;`+@@#+````,@+++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++```` @@++++';````'@```  @'`'@#'+`````##++`````@@++``@@#'@``  `@#++++++++");                                                                                            
fflush(stdout);printf("%s\n","+++++++++++#@@@@@+++++##@@@@@@##@@@@+#@#+++#@@@@@++++@@@@@+++#@@#'++#@@@@#++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++++#@@@#++++'++#@@@@###@@@#+###++++###@#++'+#@@@#++++##+++'##@@@#++++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++'++++++++++++++++++++++++++++++'++++++++++++++++++++++++++++++++++'+++++++");                                                                                            
fflush(stdout);printf("%s\n","++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");



}
