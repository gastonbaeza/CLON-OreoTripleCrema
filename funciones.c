
#include "estructuras.h"
#include "funciones.h"
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
	#define ARRAYPIDS 51
#define MEMORIA 1
	#define SOLICITUDMEMORIA 0
	#define RESULTADOINICIARPROGRAMA 23
	#define SOLICITUDINFOPROG 1
	#define ESCRIBIRMEMORIA 2
	#define LIBERARMEMORIA 3
 	#define ACTUALIZARPCB 4
 	#define PROGRAMASALIDA 5
#define CONSOLA 2
	#define INICIARPROGRAMA 0
	#define FINALIZARPROGRAMA 88
	#define DESCONECTARCONSOLA 2
	#define LIMPIARMENSAJES 3
	//------------------------------	
	#define MENSAJE 7
	#define PID 1
	#define PCB 17
	#define PATH 10
	#define LINEA 19
#define CPU 3
#define FS 4
#define TRUE 1
#define FALSE 0
#define OK 1
#define FAIL 0
#define LIBRE 0
#define OCUPADO 1
#define BLOQUE 20
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
#define VALIDARARHIVO 58
#define FINALIZARPORERROR 59
#define PCBERROR 60
#define PAGINAINVALIDA 61
#define STACKOVERFLOW 62
#define ASIGNARPAGINAS 63
#define CREARARCHIVOFS 64
#define GUARDARDATOSFS 65
#define OBTENERDATOSFS 66
#define PAQUETEFS 67
#define BORRARARCHIVOFS 68
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
	free(otroBuffer);
	

}
void handshakeCliente(int unSocket, int unCliente, void * unBuffer)
{

	void * otroBuffer=malloc(sizeof(int));
	memcpy(otroBuffer,&unCliente,sizeof(int));
	send(unSocket,otroBuffer, sizeof(int),0);
	// while(0>=recv(unSocket,unBuffer,sizeof(int),0));

	recv(unSocket,unBuffer,sizeof(int),0);


}
void generarDumpCache( t_estructuraCache* memoriaCache, int ENTRADAS_CACHE, int MARCO_SIZE)
{
	int unFrame;
	for(unFrame=0;unFrame<ENTRADAS_CACHE;unFrame++)
	{

		printf("el numero de pagina  es:%i\n",memoriaCache[unFrame].frame);
		printf("el pid del proceso es:%i\n",memoriaCache[unFrame].pid);
		printf("su antiguedad es :%i\n", memoriaCache[unFrame].antiguedad);
		printf("el contenido en cache es: %.*s\n",MARCO_SIZE,(char*)memoriaCache[unFrame].contenido);
	}
}
void generarDumpMemoria(t_marco * marcos, int MARCOS,int MARCO_SIZE)
{
	int unMarco=0;
	for(unMarco;unMarco<MARCOS;unMarco++)
	{	printf("el numero de frame es: %i\n", unMarco);
		printf("el contenido del frame es :%.*s\n",MARCO_SIZE,(char*)marcos[unMarco].numeroPagina);
	}
}
int estaEnCache(int unPid,int pagina, t_estructuraCache * memoriaCache, int  ENTRADAS_CACHE)// si esta presente en cache me devuelve la posicion de la entrada, sino devuelve -1
{	int paginaEncontrada=0;
	int paginasRecorridas=0;
	for(paginasRecorridas;paginasRecorridas<ENTRADAS_CACHE;paginasRecorridas++)
	{
		if((memoriaCache[paginasRecorridas].pid == unPid )&& (memoriaCache[paginasRecorridas].frame==pagina)) return paginasRecorridas;
	}return -1;
}
void incrementarAntiguedadPorAcceso(t_estructuraCache* memoriaCache, int ENTRADAS_CACHE)//cada vez que hago un acceso tengo que cambiar la antiguedad de todos
{
	int unaEntrada=0;
	for (unaEntrada; unaEntrada< ENTRADAS_CACHE; unaEntrada++)
	{		if(memoriaCache[unaEntrada].antiguedad!=-1)	memoriaCache[unaEntrada].antiguedad+=1;
	}
}
int buscarEntradaMasAntigua(t_estructuraCache * memoriaCache, int ENTRADAS_CACHE)
{	int entradaMasAntigua=-1;
	int unaEntrada=0;
	for (unaEntrada; unaEntrada< ENTRADAS_CACHE; unaEntrada++)
	{
			if(memoriaCache[unaEntrada].antiguedad>entradaMasAntigua)entradaMasAntigua=memoriaCache[unaEntrada].antiguedad;

	}
return entradaMasAntigua;
}
void escribirEnCache(int unPid, int pagina,void *buffer,t_estructuraCache *memoriaCache, int ENTRADAS_CACHE ,int offset, int tamanio)
{	
	int entrada;
	if(-1!=(entrada=estaEnCache(unPid,pagina, memoriaCache, ENTRADAS_CACHE)));
			
	
	
	else
	{	
		if(-1!=(entrada=hayEspacioEnCache(memoriaCache, ENTRADAS_CACHE)));
		
		else //este es el LRU
		{	
			entrada=buscarEntradaMasAntigua(memoriaCache,ENTRADAS_CACHE);
			
		}
	} 
	
	memmove((memoriaCache[entrada]).contenido,buffer,tamanio+1);
	
	// printf("memoriaCache[entrada].contenido: %s.\n",(char*)(memoriaCache[entrada]).contenido );
	
	(memoriaCache[entrada]).antiguedad=0;
	(memoriaCache[entrada]).pid=unPid;
	(memoriaCache[entrada]).frame=pagina;
	incrementarAntiguedadPorAcceso(memoriaCache,ENTRADAS_CACHE); 
}
void * solicitarBytesCache(int unPid, int pagina, t_estructuraCache * memoriaCache, int ENTRADAS_CACHE ,int offset, int tamanio)
{	void * buffer=malloc(tamanio);
	int entrada=estaEnCache(unPid,pagina, memoriaCache, ENTRADAS_CACHE);
	memcpy(buffer, memoriaCache[entrada].contenido+offset,tamanio);
	incrementarAntiguedadPorAcceso(memoriaCache,ENTRADAS_CACHE); 
	return buffer;
}
int hayEspacioEnCache(t_estructuraCache * memoriaCache, int ENTRADAS_CACHE)// si esta llena me devuelve un -1 sino, la primer entrada libre
{	int unaEntrada=0;
	for (unaEntrada; unaEntrada< ENTRADAS_CACHE; unaEntrada++)
	{
			if(memoriaCache[unaEntrada].pid==-1) return unaEntrada;
	}return -1;
	
}
void * solicitarBytes(int unPid, int pagina, t_marco * marcos, int MARCOS,int offset, int tamanio, t_estructuraADM * bloquesAdmin, t_list**overflow)// en memoria despues de queme solicitan o almacenan bytes tengo que escribirlos en cache, no los hago aca porque sino esta funcion hace mas de lo que debe
{	void * buffer=malloc(tamanio);	int indice=calcularPosicion(unPid,pagina,MARCOS);
	int entrada=buscarEnOverflow(indice, unPid, pagina,bloquesAdmin,MARCOS,overflow);
	memcpy(buffer, marcos[entrada].numeroPagina+offset,tamanio);
	return buffer;

}
void almacenarBytes(int unPid, int pagina,char * contenido,t_marco * marcos, int MARCOS ,int offset, int tamanio,t_estructuraADM * bloquesAdmin , t_list**overflow,t_estructuraCache * memoriaCache,int ENTRADAS_CACHE, int MARCO_SIZE)
{	
	int indice=calcularPosicion(unPid,pagina,MARCOS); printf("el indice en almacenar bytes es: %i\n",indice );
	int entrada=buscarEnOverflow(indice, unPid,pagina,bloquesAdmin,MARCOS,overflow); printf("la entrada en almacenarbytes de buscar overflow : %i\n",entrada );
	strcpy((char*)(marcos[entrada].numeroPagina+offset),contenido);
	escribirEnCache(unPid,pagina,marcos[entrada].numeroPagina,memoriaCache,ENTRADAS_CACHE,0,MARCO_SIZE);
 								
}

void calcularTamanioProceso(int pid, t_estructuraADM * bloquesAdmin, int MARCOS)//expandir despues esa funcion para que informe cosas mas lindas
{

	int encontrados=0;
	int unMarco=0;
	for(unMarco;unMarco<MARCOS;unMarco++)
	{
		if(bloquesAdmin[unMarco].pid==pid)
		{encontrados++;
			
		}

	}
	if(encontrados==0) printf("%s\n","El pid no se encuentra en memoria" );
	else{printf("Se encontraron %i %s\n",encontrados,"paginas asociadas a ese proceso" );}
}
void generarDumpProceso(t_estructuraADM * bloquesAdmin, int MARCOS, int pid,t_marco * marcos)
{	int encontrados=0;
	int unMarco=0;
	for(unMarco;unMarco<MARCOS;unMarco++)
	{
		if(bloquesAdmin[unMarco].pid==pid)
		{encontrados++;
			printf("%p\n",marcos[unMarco].numeroPagina);
		}

	}
	if(encontrados==0) printf("%s\n","El pid no se encuentra en memoria" );
}
void generarDumpAdministrativas(t_estructuraADM * bloquesAdmin, int MARCOS)
{
	t_list * procesosActivos;
	procesosActivos=list_create();
	buscarProcesosActivos(procesosActivos,bloquesAdmin,MARCOS);
	int unaAdmin;
	for (unaAdmin= 0; unaAdmin < MARCOS; unaAdmin++)
	{fflush(stdout);printf("%s\n","tabla de paginas");
	
		fflush(stdout);printf("%i\n",bloquesAdmin[unaAdmin].pid);
		fflush(stdout);printf("%i\n",bloquesAdmin[unaAdmin].pagina);
		fflush(stdout);printf("%i\n",bloquesAdmin[unaAdmin].estado);

	}
	int cantidadProcesos=list_size(procesosActivos);
	int unProceso=0;
	int * proceso=malloc(sizeof(int));
	printf("%s\n","lista de procesos activos:" ); 
	for (unProceso = 0; unProceso < cantidadProcesos; unProceso++)
	{	
		memcpy(proceso,list_get(procesosActivos,unProceso),sizeof(int));
		
		fflush(stdout);printf("proceso: %i|",*proceso);
	}printf("%s\n"," " );
}
void buscarProcesosActivos(t_list * procesosActivos, t_estructuraADM * bloquesAdmin, int MARCOS)
 {
 	int unaAdmin;
 	for (unaAdmin= 0; unaAdmin < MARCOS; unaAdmin++)
 	{
		if(bloquesAdmin[unaAdmin].pid!=(-1))
			{ list_add(procesosActivos,&bloquesAdmin[unaAdmin].pid);}

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
{int cantidadRestantes;
	int encontradas;
 	int paginasRecorridas=0;
 	int unFrame=0;
 	while(paginasRecorridas+unaCantidad<MARCOS){ 
 	 	cantidadRestantes=unaCantidad;
    	encontradas=0;
		while(bloquesAdmin[unFrame].estado==0 && cantidadRestantes!=0){
			cantidadRestantes --;
    		unFrame++;encontradas++;
    	}
    	if(cantidadRestantes==0){
    		return encontradas;
    	}
    	unFrame++;	
    	paginasRecorridas++;
    }
return -1;
	
}




int calcularPaginas(int tamanioPagina,int tamanio)
{
	double cantidadPaginas;
	int cantidadReal;
	cantidadPaginas=(tamanio+tamanioPagina-1)/tamanioPagina;
 	 return cantidadPaginas;
 							
}
 							


int buscarAdministrativa(int pid,int pagina, t_estructuraADM * bloquesAdministrativas,int MARCOS)
{

	
	int marcosRecorridos=0;
	while(marcosRecorridos<MARCOS)
	{
		if(bloquesAdministrativas[marcosRecorridos].pid==pid && bloquesAdministrativas[marcosRecorridos].pagina==pagina)
			{return marcosRecorridos;}

		else{bloquesAdministrativas=bloquesAdministrativas+sizeof(t_estructuraADM);}
		marcosRecorridos++;
	}
	return FAIL;
}
/////////// LEEEEEEEMEEEEE ME OLVIDE DE HACER FREE DE LOS PAQUETES UNA VEZ QUE LOS MANDO A LA WEA/////
void serial_int(int * numero,int unSocket)
{	
	int * buffer=malloc(sizeof(int));
	int b=1;
	memcpy(buffer,&b,sizeof(int));
	send(unSocket,numero,sizeof(int),0);	
	while(0>=recv(unSocket,buffer, sizeof(int),0));
}
void dserial_int(int * numero,int unSocket)
{
	int * buffer=malloc(sizeof(int));
	int b=1;
	memcpy(buffer,&b,sizeof(int));
	while(0>=recv(unSocket,numero,sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
}
int dserial_string(char ** unString,int unSocket)
{	int tamanio;
	int  unChar;
	int * buffer1=malloc(sizeof(int));
	int b=1;
	memcpy(buffer1,&b,sizeof(int));
	while(0>recv(unSocket,&tamanio,sizeof(int),0));
	*unString=calloc(1,tamanio+1);
	send(unSocket,buffer1, sizeof(int),0);
	// for (unChar= 0; unChar <tamanio; unChar++)
	// {
	// 	while(0>=recv(unSocket, &unString[unChar],sizeof(char),0));
	// 	send(unSocket,buffer1, sizeof(int),0);

	// }
	if (tamanio>0)
	{
	while(0>recv(unSocket,*unString,tamanio,0));
	send(unSocket,buffer1, sizeof(int),0);
	}
	free(buffer1);
return tamanio;
}
void serial_string(char * unString,int tamanio,int unSocket)
{	int  unChar;
	int * buffer=malloc(sizeof(int));
	int a=1;
	memcpy(buffer,&a,sizeof(int));
	send(unSocket,&tamanio,sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	// for (unChar= 0; unChar < tamanio; unChar++)
	// {
	// 	send(unSocket, &unString[unChar],sizeof(char),0);
	// 	while(0>=recv(unSocket,buffer, sizeof(int),0));
	// }
	if (tamanio>0)
	{
	send(unSocket,unString,tamanio,0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	}
	free(buffer);
}

void serial_pcb(t_pcb * pcb, int unSocket)
{	int * buffer=malloc(sizeof(int));
	int a=1;
	int i,j;
	memcpy(buffer,&a,sizeof(int));
	send(unSocket,&(pcb->pid),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->programCounter),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->estado),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->rafagasEjecutadas),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->privilegiadasEjecutadas),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->paginasHeap),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->alocaciones),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->bytesAlocados),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->liberaciones),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->bytesLiberados),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->cantidadArchivos),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	for (i = 0; i < pcb->cantidadArchivos; i++)
	{
		send(unSocket,&(pcb->referenciaATabla[i].flags),sizeof(t_banderas),0);
		while(0>=recv(unSocket,buffer, sizeof(int),0));
		send(unSocket,&(pcb->referenciaATabla[i].globalFd),sizeof(int),0);
		while(0>=recv(unSocket,buffer, sizeof(int),0));
		send(unSocket,&(pcb->referenciaATabla[i].cursor),sizeof(int),0);
		while(0>=recv(unSocket,buffer, sizeof(int),0));
	}
	send(unSocket,&(pcb->paginasCodigo),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->posicionStack),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(pcb->cantidadInstrucciones),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	for (i= 0; i < pcb->cantidadInstrucciones; i++)
	{
		send(unSocket, &(pcb->indiceCodigo[i]),sizeof(t_intructions),0);
		while(0>=recv(unSocket,buffer, sizeof(int),0));
	}
	send(unSocket,&(pcb->indiceEtiquetas.etiquetas_size),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	serial_string(pcb->indiceEtiquetas.etiquetas,pcb->indiceEtiquetas.etiquetas_size,unSocket);
	send(unSocket,&(pcb->cantidadStack),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	for (i= 0; i < pcb->cantidadStack; i++)
	{	
		send(unSocket,&(pcb->indiceStack[i].cantidadArgumentos),sizeof(int),0);
		while(0>=recv(unSocket,buffer, sizeof(int),0));
		for (j = 0; j < pcb->indiceStack[i].cantidadArgumentos; j++)
		{
			send(unSocket, &(pcb->indiceStack[i].argumentos[j]),sizeof(t_argumento),0);
			while(0>=recv(unSocket,buffer, sizeof(int),0));
		}
		send(unSocket,&(pcb->indiceStack[i].cantidadVariables),sizeof(int),0);
		while(0>=recv(unSocket,buffer, sizeof(int),0));
		for (j = 0; j < pcb->indiceStack[i].cantidadVariables; j++)
		{
			send(unSocket, &(pcb->indiceStack[i].variables[j]),sizeof(t_variable),0);
			while(0>=recv(unSocket,buffer, sizeof(int),0));
		}
		send(unSocket,&(pcb->indiceStack[i].posRetorno),sizeof(int),0);
		while(0>=recv(unSocket,buffer, sizeof(int),0));
		send(unSocket,&(pcb->indiceStack[i].varRetorno),sizeof(t_posMemoria),0);
		while(0>=recv(unSocket,buffer, sizeof(int),0));
	}
	send(unSocket,&(pcb->exitCode),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));free(buffer);
}

void dserial_pcb(t_pcb* pcb, int unSocket)
{	
	int * buffer=malloc(sizeof(int));
	int a=1,j,i;
	memcpy(buffer,&a,sizeof(int));
	while(0>recv(unSocket,&(pcb->pid),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->programCounter),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->estado),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->rafagasEjecutadas),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->privilegiadasEjecutadas),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->paginasHeap),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->alocaciones),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->bytesAlocados),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->liberaciones),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->bytesLiberados),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->cantidadArchivos),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	pcb->referenciaATabla=malloc(sizeof(t_tablaArchivosDeProcesos)*pcb->cantidadArchivos);
	for (i = 0; i < pcb->cantidadArchivos; i++)
	{
		while(0>recv(unSocket,&(pcb->referenciaATabla[i].flags),sizeof(t_banderas),0));
		send(unSocket,buffer, sizeof(int),0);
		while(0>recv(unSocket,&(pcb->referenciaATabla[i].globalFd),sizeof(int),0));
		send(unSocket,buffer, sizeof(int),0);
		while(0>recv(unSocket,&(pcb->referenciaATabla[i].cursor),sizeof(int),0));
		send(unSocket,buffer, sizeof(int),0);			
	}
	while(0>recv(unSocket,&(pcb->paginasCodigo),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->posicionStack),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(pcb->cantidadInstrucciones),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	pcb->indiceCodigo=malloc(pcb->cantidadInstrucciones*sizeof(t_intructions));
	for (i = 0; i < pcb->cantidadInstrucciones; i++)
	{
		while(0>recv(unSocket,&(pcb->indiceCodigo[i]),sizeof(t_intructions),0));
		send(unSocket,buffer, sizeof(int),0);
	}
	while(0>recv(unSocket,&(pcb->indiceEtiquetas.etiquetas_size),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	dserial_string(&(pcb->indiceEtiquetas.etiquetas),unSocket);
	while(0>recv(unSocket,&(pcb->cantidadStack),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	pcb->indiceStack=malloc(pcb->cantidadStack*sizeof(t_stack));
	for (i = 0; i < pcb->cantidadStack; i++)
	{
		while(0>recv(unSocket,&(pcb->indiceStack[i].cantidadArgumentos),sizeof(int),0));
		send(unSocket,buffer, sizeof(int),0);
		pcb->indiceStack[i].argumentos=malloc(pcb->indiceStack[i].cantidadArgumentos*sizeof(t_argumento));
		for (j = 0; j < pcb->indiceStack[i].cantidadArgumentos; j++)
		{
			while(0>recv(unSocket,&(pcb->indiceStack[i].argumentos[j]),sizeof(t_argumento),0));
			send(unSocket,buffer, sizeof(int),0);
		}
		while(0>recv(unSocket,&(pcb->indiceStack[i].cantidadVariables),sizeof(int),0));
		send(unSocket,buffer, sizeof(int),0);
		pcb->indiceStack[i].variables=malloc(pcb->indiceStack[i].cantidadVariables*sizeof(t_variable));
		for (j = 0; j < pcb->indiceStack[i].cantidadVariables; j++)
		{
			while(0>recv(unSocket,&(pcb->indiceStack[i].variables[j]),sizeof(t_variable),0));
			send(unSocket,buffer, sizeof(int),0);
		}
		while(0>recv(unSocket,&(pcb->indiceStack[i].posRetorno),sizeof(int),0));
		send(unSocket,buffer, sizeof(int),0);
		while(0>recv(unSocket,&(pcb->indiceStack[i].varRetorno),sizeof(t_posMemoria),0));
		send(unSocket,buffer, sizeof(int),0);
	}
	while(0>recv(unSocket,&(pcb->exitCode),sizeof(int),0));
    send(unSocket,buffer, sizeof(int),0);free(buffer);
}
void dserial_programaSalida(t_programaSalida * programaSalida, int unSocket)
{	
	programaSalida->tamanio=dserial_string(&(programaSalida->elPrograma),unSocket);}

void dserial_resultadoIniciarPrograma(t_resultadoIniciarPrograma* resultadoIniciarPrograma, int unSocket){

	dserial_int(&(resultadoIniciarPrograma->pid),unSocket);
	dserial_int(&(resultadoIniciarPrograma->resultado),unSocket);

}
void serial_resultadoIniciarPrograma(t_resultadoIniciarPrograma* resultadoIniciarPrograma, int unSocket){

	serial_int(&(resultadoIniciarPrograma->pid),unSocket);
	serial_int(&(resultadoIniciarPrograma->resultado),unSocket);
}
void serial_arrayPids(t_arrayPids * arraypids,int unSocket){
	int i;
	serial_int(&(arraypids->cantidad),unSocket);
	for (i = 0; i < arraypids->cantidad; i++)
	{
		serial_int(&(arraypids->pids[i]),unSocket);
	}
}
void dserial_arrayPids(t_arrayPids * arraypids,int unSocket){
	int i;
	dserial_int(&(arraypids->cantidad),unSocket);
	for (i = 0; i < arraypids->cantidad; i++)
	{
		dserial_int(&(arraypids->pids[i]),unSocket);
	}
}

void serial_programaSalida(t_programaSalida * programaSalida, int unSocket)
{
	
	serial_string(programaSalida->elPrograma,programaSalida->tamanio,unSocket); 
}
void dserial_path(t_path * path, int unSocket)
{	
	path->tamanio=dserial_string(&(path->path),unSocket);
}

void serial_path(t_path * path, int unSocket)
{
	serial_string(path->path,path->tamanio,unSocket); 
}
void dserial_mensaje(t_mensaje * mensaje, int unSocket)
{	
	mensaje->tamanio=dserial_string(&(mensaje->mensaje),unSocket);}

void serial_mensaje(t_mensaje * mensaje, int unSocket)
{
	serial_string(mensaje->mensaje,mensaje->tamanio,unSocket); 
}
void dserial_linea(t_linea * linea, int unSocket)
{	
	linea->tamanio=dserial_string(&(linea->linea),unSocket);}

void serial_linea(t_linea * linea, int unSocket)
{
	
	serial_string(linea->linea,linea->tamanio,unSocket); 
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
	int * buffer=malloc(sizeof(int));
	int b=1;
	memcpy(buffer,&b,sizeof(int));

	while(0>=recv(unSocket,&(solicitud->tamanioCodigo),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	dserial_string(&(solicitud->codigo),unSocket);
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
void serial_peticion(t_peticionBytes * peticion, int unSocket)
{	int * buffer=malloc(sizeof(int));
	int a=1;
	memcpy(buffer,&a,sizeof(int));
	send(unSocket,&(peticion->pid),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(peticion->pagina),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(peticion->offset),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(peticion->size),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	

	free(buffer);
}

void serial_bytes(t_almacenarBytes * bytes, int unSocket)
{	int * buffer=malloc(sizeof(int));
	int a=1;
	memcpy(buffer,&a,sizeof(int));
	send(unSocket,&(bytes->pid),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(bytes->pagina),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(bytes->offset),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	send(unSocket,&(bytes->size),sizeof(int),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
	serial_string(bytes->valor,20,unSocket);

	free(buffer);
}
void dserial_bytes(t_almacenarBytes * bytes, int unSocket)
{	
	int * buffer=malloc(sizeof(int));
	int a=1;
	memcpy(buffer,&a,sizeof(int));
	while(0>recv(unSocket,&(bytes->pid),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(bytes->pagina),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(bytes->offset),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(bytes->size),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	dserial_string(&(bytes->valor),unSocket);
	free(buffer);
}

void dserial_peticion(t_peticionBytes* peticion, int unSocket)
{	
	int * buffer=malloc(sizeof(int));
	int a=1;
	memcpy(buffer,&a,sizeof(int));
	while(0>recv(unSocket,&(peticion->pid),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(peticion->pagina),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(peticion->offset),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	while(0>recv(unSocket,&(peticion->size),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
	free(buffer);
}

void serial_solicitudValorVariable(t_solicitudValorVariable * solicitudValorVariable, int unSocket)
{
	
	serial_string(solicitudValorVariable->variable,solicitudValorVariable->tamanioNombre,unSocket); 
}

void dserial_solicitudValorVariable(t_solicitudValorVariable * solicitudValorVariable, int unSocket)
{	
	solicitudValorVariable->tamanioNombre=dserial_string(&(solicitudValorVariable->variable),unSocket);}

void serial_asignarVariableCompartida(t_asignarVariableCompartida * asignarVariableCompartida, int unSocket)
{
	
	serial_string(asignarVariableCompartida->variable,asignarVariableCompartida->tamanioNombre,unSocket); 
	serial_int(&(asignarVariableCompartida->valor),unSocket);
}
void dserial_asignarValorCompartida(t_asignarVariableCompartida * asignarVariableCompartida, int unSocket)
{	
	asignarVariableCompartida->tamanioNombre=dserial_string(&(asignarVariableCompartida->variable),unSocket);
	dserial_int(&(asignarVariableCompartida->valor),unSocket);}

void serial_solicitudSemaforo(t_solicitudSemaforo * solicitudSemaforo, int unSocket)
{
	
	serial_string(solicitudSemaforo->identificadorSemaforo,solicitudSemaforo->tamanioIdentificador,unSocket); 
	serial_int(&(solicitudSemaforo->estadoSemaforo),unSocket);
}
void dserial_solicitudSemaforo(t_solicitudSemaforo * solicitudSemaforo, int unSocket)
{	
	solicitudSemaforo->tamanioIdentificador=dserial_string(&(solicitudSemaforo->identificadorSemaforo),unSocket);
	dserial_int(&(solicitudSemaforo->estadoSemaforo),unSocket);}

void serial_reservarEspacioMemoria(t_reservarEspacioMemoria * reservarEspacioMemoria, int unSocket)
{
	serial_int(&(reservarEspacioMemoria->espacio),unSocket);
}
void dserial_reservarEspacioMemoria(t_reservarEspacioMemoria * reservarEspacioMemoria, int unSocket)
{	
	dserial_int(&(reservarEspacioMemoria->espacio),unSocket);}

void serial_liberarMemoria(t_liberarMemoria * liberarMemoria, int unSocket)
{
	serial_int(&(liberarMemoria->direccionMemoria),unSocket);
}
void dserial_liberarMemoria(t_liberarMemoria * liberarMemoria, int unSocket)
{	
	dserial_int(&(liberarMemoria->direccionMemoria),unSocket);}

void serial_abrirArchivo(t_abrirArchivo * abrirArchivo, int unSocket){
	int * buffer=malloc(sizeof(int));
	int a=1;
	memcpy(buffer,&a,sizeof(int));
	serial_string(abrirArchivo->direccionArchivo,abrirArchivo->tamanio,unSocket);
	send(unSocket,&(abrirArchivo->flags),sizeof(t_banderas),0);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
}

void dserial_abrirArchivo(t_abrirArchivo * abrirArchivo,int unSocket){
	int * buffer=malloc(sizeof(int));
	int a=1;
	memcpy(buffer,&a,sizeof(int));
	abrirArchivo->tamanio=dserial_string(&(abrirArchivo->direccionArchivo),unSocket);
	while(0>recv(unSocket,&(abrirArchivo->flags),sizeof(t_banderas),0));
	send(unSocket,buffer, sizeof(int),0);
}
void serial_borrarArchivo(t_borrarArchivo * borrarArchivo, int unSocket)
{
	serial_int(&(borrarArchivo->fdABorrar),unSocket);
}
void dserial_borrarArchivo(t_borrarArchivo * borrarArchivo, int unSocket)
{	
	dserial_int(&(borrarArchivo->fdABorrar),unSocket);
}
void serial_cerrarArchivo(t_cerrarArchivo * cerrarArchivo, int unSocket)
{
	serial_int(&(cerrarArchivo->descriptorArchivo),unSocket);
}
void dserial_cerrarArchivo(t_cerrarArchivo * cerrarArchivo, int unSocket)
{	
	dserial_int(&(cerrarArchivo->descriptorArchivo),unSocket);
}
void serial_fdParaLeer(t_fdParaLeer * fdParaLeer, int unSocket)
{
	serial_int(&(fdParaLeer->fd),unSocket);
}
void dserial_fdParaLeer(t_fdParaLeer * fdParaLeer, int unSocket)
{	
	dserial_int(&(fdParaLeer->fd),unSocket);
}
void serial_moverCursor(t_moverCursor * moverCursor, int unSocket)
{
	serial_int(&(moverCursor->descriptorArchivo),unSocket);
	serial_int(&(moverCursor->posicion),unSocket);
}
void dserial_moverCursor(t_moverCursor * moverCursor, int unSocket)
{	
	dserial_int(&(moverCursor->descriptorArchivo),unSocket);
	dserial_int(&(moverCursor->posicion),unSocket);
}
void serial_obtenerDatosFs(t_leerArchivoFS * leerArchivoFS, int unSocket)
{
	serial_string(leerArchivoFS->path,leerArchivoFS->tamanioPath,unSocket);
	serial_int(&(leerArchivoFS->offset),unSocket);
	serial_int(&(leerArchivoFS->size),unSocket);

}
void dserial_obtenerDatosFs(t_leerArchivoFS * leerArchivoFS, int unSocket)
{	
	leerArchivoFS->tamanioPath=dserial_string(&(leerArchivoFS->path),unSocket);
	dserial_int(&(leerArchivoFS->offset),unSocket);
	dserial_int(&(leerArchivoFS->size),unSocket);

}
void serial_guardarDatosFs(t_escribirArchivoFS * escribirArchivoFS, int unSocket)
{
	serial_string(escribirArchivoFS->path,escribirArchivoFS->tamanioPath,unSocket);
	serial_int(&(escribirArchivoFS->offset),unSocket);
	serial_string(escribirArchivoFS->buffer,escribirArchivoFS->size,unSocket);
}
void dserial_guardarDatosFs(t_escribirArchivoFS * escribirArchivoFS, int unSocket)
{	
	escribirArchivoFS->tamanioPath=dserial_string(&(escribirArchivoFS->path),unSocket);
	dserial_int(&(escribirArchivoFS->offset),unSocket);
	escribirArchivoFS->size=dserial_string(&(escribirArchivoFS->buffer),unSocket);
	}
void serial_paqueteFs(t_paqueteFS * paqueteFS, int unSocket)
{
	serial_string(paqueteFS->paquete,paqueteFS->tamanio,unSocket);
}
void dserial_paqueteFs(t_paqueteFS * paqueteFS, int unSocket)
{	
	paqueteFS->tamanio=dserial_string(&(paqueteFS->paquete),unSocket);
}
void serial_escribirArchivo(t_escribirArchivo * escribirArchivo, int unSocket)
{
	serial_string(escribirArchivo->informacion,escribirArchivo->tamanio,unSocket); 
	serial_int(&(escribirArchivo->fdArchivo),unSocket);
}
void dserial_escribirArchivo(t_escribirArchivo * escribirArchivo, int unSocket)
{	
	escribirArchivo->tamanio=dserial_string(&(escribirArchivo->informacion),unSocket);
	dserial_int(&(escribirArchivo->fdArchivo),unSocket);
}
void serial_leerArchivo(t_leerArchivo * leerArchivo, int unSocket)
{
	serial_int(&(leerArchivo->descriptor),unSocket);
	serial_int(&(leerArchivo->tamanio),unSocket);
}
void dserial_leerArchivo(t_leerArchivo * leerArchivo, int unSocket)
{	
	dserial_int(&(leerArchivo->descriptor),unSocket);
	dserial_int(&(leerArchivo->tamanio),unSocket);
}

void enviarDinamico(int tipoPaquete,int unSocket,void * paquete)
{ 	
	t_seleccionador * seleccionador=calloc(2,4);
	seleccionador->tipoPaquete=tipoPaquete;
	int * buffer=calloc(1,4);
	int a=1;
	memcpy(buffer,&a,sizeof(int));
	send(unSocket,seleccionador,sizeof(t_seleccionador),0);
	printf("tipoPaquete en enviarDinamico: %i \n", seleccionador->tipoPaquete);
	switch(tipoPaquete){
		case SOLICITUDMEMORIA:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_solicitudMemoria((t_solicitudMemoria *)paquete,unSocket);
		break;

		case PROGRAMASALIDA:	
	while(0>=recv(unSocket,buffer, sizeof(int),0));//este tambien sirve cuando queremos mandar un string con su tamaño
			serial_programaSalida((t_programaSalida * )paquete,unSocket);			
		break;

		case PATH:	
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_path((t_path * )paquete,unSocket);			
		break;

		case FINALIZARPROGRAMA: 
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_int((int*)paquete,unSocket);
			
		break;

		case MENSAJE:	
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_mensaje((t_mensaje * )paquete,unSocket);			
		break;

		case LINEA:	
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_linea((t_linea * )paquete,unSocket);			
		break;

		case PCB:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_pcb((t_pcb *)paquete,unSocket);
		break;
		case ALMACENARBYTES:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_bytes((t_almacenarBytes *)paquete,unSocket);
		break;
		case SOLICITUDBYTES:
			while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_peticion((t_peticionBytes *)paquete,unSocket);
			
		break;
		case RESULTADOINICIARPROGRAMA:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_resultadoIniciarPrograma((t_resultadoIniciarPrograma*)paquete,unSocket);
		break;
		case SOLICITUDVALORVARIABLE:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_solicitudValorVariable((t_solicitudValorVariable *)paquete,unSocket);
		break;
		case ASIGNARVARIABLECOMPARTIDA:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_asignarVariableCompartida((t_asignarVariableCompartida *)paquete,unSocket);
		break;
		case SOLICITUDSEMWAIT:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_solicitudSemaforo((t_solicitudSemaforo *)paquete,unSocket);
		break;
		case SOLICITUDSEMSIGNAL:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_solicitudSemaforo((t_solicitudSemaforo *)paquete,unSocket);
		break;
		case RESERVARESPACIO:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_reservarEspacioMemoria((t_reservarEspacioMemoria *)paquete, unSocket);
		break;
		case LIBERARESPACIOMEMORIA:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_liberarMemoria((t_liberarMemoria *)paquete,unSocket);
		break;
		case ARRAYPIDS:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_arrayPids((t_arrayPids *)paquete,unSocket);
		break;
		case ABRIRARCHIVO:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_abrirArchivo((t_abrirArchivo *)paquete,unSocket);
		break;
		case BORRARARCHIVO:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_borrarArchivo((t_borrarArchivo *)paquete,unSocket);
		break;
		case CERRARARCHIVO:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
		 	serial_cerrarArchivo((t_cerrarArchivo *)paquete,unSocket);
		 break;
		 case MOVERCURSOR:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
		 	serial_moverCursor((t_moverCursor *)paquete,unSocket);
		 break;
		 case ESCRIBIRARCHIVO:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
		 	serial_escribirArchivo((t_escribirArchivo *)paquete,unSocket);

		 break;
		 case LEERARCHIVO:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
		 	serial_leerArchivo((t_leerArchivo *)paquete,unSocket);
		 case OBTENERDATOSFS:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
		 	serial_obtenerDatosFs((t_leerArchivoFS *)paquete,unSocket);
		 case GUARDARDATOSFS:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
		 	serial_guardarDatosFs((t_escribirArchivoFS *)paquete,unSocket);
		 case PAQUETEFS:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
		 	serial_paqueteFs((t_paqueteFS *)paquete,unSocket);
		 break;
		 case BORRARARCHIVOFS:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
		 	serial_path((t_path *)paquete,unSocket);
		 break;
		 case VALIDARARHIVO:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
		 	serial_path((t_path*)paquete,unSocket);
		 break;
		 case CREARARCHIVOFS:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
		 	serial_path((t_path*)paquete,unSocket);
		 break;
		 case ESPERONOVEDADES: case FINALIZARPROCESO: case FINALIZARPORERROR: case FINQUANTUM: case PARAREJECUCION: case PAGINAINVALIDA: case STACKOVERFLOW:
		 break;
		 case CONTINUAR:
		 break;
		 case PCBFINALIZADO: case PCBFINALIZADOPORCONSOLA: case PCBERROR: case PCBQUANTUM: case PCBBLOQUEADO:
	while(0>=recv(unSocket,buffer, sizeof(int),0));
			serial_pcb((t_pcb *)paquete,unSocket);
		 break;
		 
		default : fflush(stdout); printf("%s: %i.\n","el paquete que quiere enviar es de un formato desconocido",tipoPaquete); 
		// pagaraprata();
		break;

		
	}
	free(buffer);
	free(seleccionador);
						
}
void recibirDinamico(int tipoPaquete,int unSocket, void * paquete)
{	
	int * buffer=malloc(sizeof(int));
	int b=1;
	memcpy(buffer,&b,sizeof(int));
	send(unSocket,buffer, sizeof(int),0);
	t_path * path;
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

		case FINALIZARPROGRAMA:
					dserial_int((int*)paquete,unSocket);
		break;

		case MENSAJE:	
					dserial_mensaje((t_mensaje * )paquete,unSocket);			
		break;

		case LINEA:	
					dserial_linea((t_linea * )paquete,unSocket);			
		break;

		case PCB:	
					dserial_pcb((t_pcb *)paquete,unSocket);
		break;
		case PCBFINALIZADO:	case PCBFINALIZADOPORCONSOLA: case PCBERROR: case PCBQUANTUM: case PCBBLOQUEADO:
					dserial_pcb((t_pcb *)paquete,unSocket);
		break;
		case RESULTADOINICIARPROGRAMA:
					dserial_resultadoIniciarPrograma((t_resultadoIniciarPrograma*)paquete,unSocket);
		break;
		case SOLICITUDBYTES:
			dserial_peticion((t_peticionBytes *)paquete,unSocket);
		break;
		case ALMACENARBYTES:
			dserial_bytes((t_almacenarBytes *)paquete,unSocket);
			break;
		case SOLICITUDVALORVARIABLE:
			dserial_solicitudValorVariable((t_solicitudValorVariable *)paquete, unSocket);
		break;
		case ASIGNARVARIABLECOMPARTIDA:
			dserial_asignarValorCompartida((t_asignarVariableCompartida *)paquete,unSocket);
		break;
		case SOLICITUDSEMWAIT:
			dserial_solicitudSemaforo((t_solicitudSemaforo *)paquete,unSocket);
		break;
		case ARRAYPIDS:	
					dserial_arrayPids((t_arrayPids *)paquete,unSocket);
		break;
		case SOLICITUDSEMSIGNAL:
			dserial_solicitudSemaforo((t_solicitudSemaforo *)paquete,unSocket);
		break;
		case RESERVARESPACIO:
			dserial_reservarEspacioMemoria((t_reservarEspacioMemoria *)paquete,unSocket);
		break;
		case LIBERARESPACIOMEMORIA:
			dserial_liberarMemoria((t_liberarMemoria *)paquete,unSocket);
		break;
		case ABRIRARCHIVO:
			dserial_abrirArchivo((t_abrirArchivo *)paquete,unSocket);
		break;
		case BORRARARCHIVO:
			dserial_borrarArchivo((t_borrarArchivo *)paquete,unSocket);
		break;
		case CERRARARCHIVO:
			dserial_cerrarArchivo((t_cerrarArchivo *)paquete,unSocket);
		break;
		case MOVERCURSOR:
			dserial_moverCursor((t_moverCursor *)paquete,unSocket);
		break;
		case ESCRIBIRARCHIVO:
			dserial_escribirArchivo((t_escribirArchivo *)paquete,unSocket);
		break;
		case LEERARCHIVO:
			dserial_leerArchivo((t_leerArchivo *)paquete,unSocket);
		break;
		case OBTENERDATOSFS:
			dserial_obtenerDatosFs((t_leerArchivoFS *)paquete,unSocket);
		break;
		case GUARDARDATOSFS:
			dserial_guardarDatosFs((t_escribirArchivoFS *)paquete,unSocket);
		break;
		case BORRARARCHIVOFS:
			dserial_path((t_path *)paquete,unSocket);
		break;
		case PAQUETEFS:
			dserial_paqueteFs((t_paqueteFS *)paquete,unSocket);
		break;
		case VALIDARARHIVO:
		 	dserial_path((t_path*)paquete,unSocket);
		break;
		case CREARARCHIVOFS:
		 	dserial_path((t_path*)paquete,unSocket);
		break;
		case ESPERONOVEDADES: case FINALIZARPROCESO: case FINALIZARPORERROR: case FINQUANTUM: case PARAREJECUCION: case STACKOVERFLOW: case PAGINAINVALIDA:
		break;
		case CONTINUAR:
		break;
		
		default : fflush(stdout); printf("%s\n","el paquete que quiere enviar es de un formato desconocido"); 
		// pagaraprata();
		break;

		
	}
	free(buffer);
}


t_programaSalida * obtenerPrograma( char * unPath){
	FILE * punteroAlArchivo;
	if((punteroAlArchivo=fopen(unPath,"r"))==NULL)
		{
			fflush(stdout); 
			printf("el archivo no existe" ); 
		}
	
	else{
		t_programaSalida * estructuraPrograma=malloc(sizeof(t_programaSalida));
		fseek (punteroAlArchivo, 0, SEEK_END);
		estructuraPrograma->tamanio = ftell (punteroAlArchivo);
		fseek (punteroAlArchivo, 0, SEEK_SET);
		estructuraPrograma->elPrograma = calloc(1,estructuraPrograma->tamanio);
		fread (estructuraPrograma->elPrograma, 1, estructuraPrograma->tamanio, punteroAlArchivo);
		fclose (punteroAlArchivo);
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

int estaLibreMarco(int unMarco,t_estructuraADM * bloquesAdmin)// devuelve 1 si esta libre
{
    if (bloquesAdmin[unMarco].estado==0){return 1;}
    else {return 0; }
}
int buscarMarcoLibre(t_marco *marcos,int MARCOS,t_estructuraADM * bloquesAdmin) //devuelve -1 en falta de memoria o el marco libre
{
    int unMarco=0;
    for(unMarco;unMarco<MARCOS;unMarco++)
    {
        if(estaLibreMarco(unMarco,bloquesAdmin)) return unMarco;

    }
    return -1;
}

int reservarYCargarPaginas(int paginasCodigo,int paginasStack, int MARCOS, t_estructuraADM * bloquesAdmin, t_marco ** marcos,int tamanioCodigo,int unPid,char** codigo, int  MARCO_SIZE, t_list**overflow,int ENTRADAS_CACHE,t_estructuraCache * memoriaCache)
 {    int indice;
     int unFrame=0;
     int * marco=calloc(1,sizeof(int));
     int paginasCargadas=0;
     int tamanioAPegar=MARCO_SIZE*sizeof(char);
     int acumulador=0;
    
     int paginasRequeridas=paginasCodigo+paginasStack;
     for(unFrame;unFrame<paginasRequeridas;unFrame++)
     {   
     	indice=calcularPosicion(unPid,unFrame,MARCOS); printf("que carajos este indice %i\n",indice );
         *marco=buscarMarcoLibre(*marcos,MARCOS,bloquesAdmin); printf("a ver ese marco %i\n", *marco );
         if(*marco!=-1)
         {
         	agregarSiguienteEnOverflow(indice,marco,overflow);
        	 bloquesAdmin[*marco].estado=1;
        	bloquesAdmin[*marco].pid=unPid;
        	bloquesAdmin[*marco].pagina=unFrame;
        	
    		if(paginasCargadas<paginasCodigo)
	    		{ if(acumulador+(MARCO_SIZE*sizeof(char))>tamanioCodigo) 
	    				{
	    				tamanioAPegar=tamanioCodigo-acumulador;
	    				}
	    			
	    			
					memcpy((*marcos)[*marco].numeroPagina,(*codigo)+acumulador,tamanioAPegar);
					
					escribirEnCache(unPid,unFrame,(void*)(*codigo)+acumulador,memoriaCache,ENTRADAS_CACHE,0,tamanioAPegar);
					
					acumulador+=tamanioAPegar*sizeof(char);
	    		}
    		
    	} else {return -1;}
    	paginasCargadas++;
     }
     free(marco);
     return 1;
    }

 
void generarDumpOverflow(t_list**overflow, int MARCOS)
{	int i,j;
	for (i = 0; i < MARCOS; i++)
	{
		printf("Indice: %i.\n",i);
		for (j = 0; j < list_size(overflow[i]); j++)
		{
			printf("\t\tFrame: %i.\n", *(int*)list_get(overflow[i],j));
		}
	}
}
/* Función Hash */
unsigned int calcularPosicion(int pid, int num_pagina,int MARCOS) {
    char str1[20];
    char str2[20];
    sprintf(str1, "%d", pid);
    sprintf(str2, "%d", num_pagina);
    strcat(str1, str2);
    unsigned int indice = atoi(str1) % MARCOS;
    return indice;
}

/* Inicialización vector overflow. Cada posición tiene una lista enlazada que guarda números de frames.
 * Se llenará a medida que haya colisiones correspondientes a esa posición del vector. */
void inicializarOverflow(int MARCOS, t_list**overflow) {
    int i;
    for (i = 0; i < MARCOS; i++) { /* Una lista por frame */
        overflow[i] = list_create();
    }
}

/* En caso de colisión, busca el siguiente frame en el vector de overflow.
 * Retorna el número de frame donde se encuentra la página. */
int buscarEnOverflow(int indice, int pid, int pagina,t_estructuraADM * bloquesAdmin,int MARCOS, t_list**overflow) {
    printf("%s\n","estoy en buscarEnOverlow" );
    int i = 0;
    int frameDelIndice;printf("el indice que entra es :%i\n", indice);
    int  *miFrame=malloc(sizeof(int)); printf("%s\n","declare miFrame" );printf("size %i\n", list_size(overflow[indice]));
    for (i = 0; i < list_size(overflow[indice]); i++) { printf("%s\n","mira mama un for" );
    	frameDelIndice=*(int*)list_get(overflow[indice], i);printf("frameDelIndice es: %i\n",frameDelIndice );
        if ((esPaginaCorrecta(frameDelIndice, pid, pagina,bloquesAdmin,MARCOS))!=-1) { printf("%s\n","pase el casteo chamanico ultraduper" );
            miFrame= (int*)(list_get(overflow[indice], i)); printf("el señor frame es : %i\n",*miFrame );
            return *miFrame;
        }
    }return -1;
}

/* Agrega una entrada a la lista enlazada correspondiente a una posición del vector de overflow */
void agregarSiguienteEnOverflow(int pos_inicial, int * nro_frame, t_list**overflow) {
	int * aux=malloc(4);
	printf(" el marco en agregarsiguente es %i\n",*nro_frame );
	memcpy(aux,nro_frame,sizeof(int)); printf(" el aux vale%i\n", *aux );
    list_add(overflow[pos_inicial], aux);
    
}

/* Elimina un frame de la lista enlazada correspondiente a una determinada posición del vector de overflow  */
void borrarDeOverflow(int posicion, int frame, t_list**overflow) {
    int i = 0;
    int index_frame;

    for (i = 0; i < list_size(overflow[posicion]); i++) {
        if (frame == (int) list_get(overflow[posicion], i)) {
            index_frame = i;
            i = list_size(overflow[posicion]);
        }
    }

    list_remove(overflow[posicion], index_frame);
}

/* A implementar por el alumno. Devuelve 1 a fin de cumplir con la condición requerida en la llamada a la función */
int esPaginaCorrecta(int frame, int pid, int pagina,t_estructuraADM * bloquesAdmin, int MARCOS) {
printf("%s\n","no puede haber seg fault" );printf("dime que frame eres :%i\n", frame);
if(((bloquesAdmin[frame].pid)==pid) && ((bloquesAdmin[frame]).pagina==pagina)) {printf("%s\n","claramente esto no puede tener segfaultx2" );return 1;}
	else {return -1;}
}
void liberarPaginas(int * pidALiberar, t_estructuraADM * bloquesAdmin, t_marco * marcos, int MARCOS, t_list ** overflow,int MARCO_SIZE)
{	int unMarco;
	int entrada;
	int indice;
	for ( unMarco = 0; unMarco < MARCOS; unMarco++)
	{
		if (bloquesAdmin[unMarco].pid==*pidALiberar)
		{
			indice=calcularPosicion(bloquesAdmin[unMarco].pid,bloquesAdmin[unMarco].pagina,MARCOS);
			entrada=buscarEnOverflow(indice,bloquesAdmin[unMarco].pid,bloquesAdmin[unMarco].pagina,bloquesAdmin,MARCOS,overflow);
			marcos[entrada].numeroPagina=calloc(1,MARCO_SIZE);
			bloquesAdmin[entrada].estado=0;
			bloquesAdmin[entrada].pid=-1;
			bloquesAdmin[entrada].pagina=-1;

		}
	}
}
void compactarYAlocar(int entrada, int MARCO_SIZE,t_marco * marcos, int MARCOS) 
{	
	int offset=0;
	int offsetPagina=0;
	int aDesplazar;
	int freeNuevo;
	t_heapMetaData * unHeap=calloc(1,sizeof(t_heapMetaData));
	void * pagina=calloc(1,MARCO_SIZE);
	int fragmentacion=cuantaFragExternaHay(entrada,marcos,MARCO_SIZE);
	
		while(offset<MARCO_SIZE)
		{	
			if(((t_heapMetaData*)marcos[entrada].numeroPagina+offset)->isFree==0)
			{	
				
				memcpy(pagina+offsetPagina,marcos[entrada].numeroPagina+offset,((t_heapMetaData*)marcos[entrada].numeroPagina+offset)->size+sizeof(t_heapMetaData));
				offsetPagina+=((t_heapMetaData*)marcos[entrada].numeroPagina+offset)->size+sizeof(t_heapMetaData);
			}
			offset+=((t_heapMetaData*)marcos[entrada].numeroPagina+offset)->size+sizeof(t_heapMetaData);

		}
		freeNuevo=MARCO_SIZE-offsetPagina-sizeof(t_heapMetaData);
		unHeap->size=freeNuevo;
		unHeap->isFree=1;
		memcpy(pagina+offsetPagina,unHeap,sizeof(t_heapMetaData));
		memcpy(marcos[entrada].numeroPagina,pagina,MARCO_SIZE);
}

int cuantaFragExternaHay(int unaEntrada, t_marco * marcos,int MARCO_SIZE)
{	int espacioFragmentado;
	int offset=0;
	while(offset<MARCO_SIZE)
	{	
		if(((t_heapMetaData*)marcos[unaEntrada].numeroPagina+offset)->isFree)
		{
			espacioFragmentado+=((t_heapMetaData*)marcos[unaEntrada].numeroPagina+offset)->size;
		}
		offset+=((t_heapMetaData*)marcos[unaEntrada].numeroPagina+offset)->size+sizeof(t_heapMetaData);
	}
	return espacioFragmentado;
}
int malloqueameEsta(int unaEntrada, int unTamanio,t_marco * marcos,int MARCO_SIZE) //devuelve 1 si puede alocar y -1 sino
{	int offset=0;
	int noEncontreEspacio=1;
	while(offset<MARCO_SIZE && noEncontreEspacio )
	{
		if(((t_heapMetaData*)marcos[unaEntrada].numeroPagina+offset)->isFree)
		{
			if(unTamanio<((t_heapMetaData*)marcos[unaEntrada].numeroPagina+offset)->size)
			{	noEncontreEspacio=0;
				return 1;
			}
		}

		offset+=((t_heapMetaData*)marcos[unaEntrada].numeroPagina+offset)->size+sizeof(t_heapMetaData);
	}
	if(noEncontreEspacio)
	{
		if(unTamanio<cuantaFragExternaHay(unaEntrada,marcos,MARCO_SIZE))
		{
			//compactarYAlocar(unaEntrada);
			return 1;
		}
		else
		{ return -1;}
	}


}
void liberarEnHeap(int unaEntrada,int offset, t_marco * marcos, int MARCO_SIZE)
{
	((t_heapMetaData*)marcos[unaEntrada].numeroPagina+offset)->isFree=1;

}
int existePagina(int unPid, int pagina,t_estructuraADM * bloquesAdmin, int MARCOS)
{
int unaAdmin;
for (unaAdmin = 0; unaAdmin < MARCOS; unaAdmin++) 
	{
		if(bloquesAdmin[unaAdmin].pid == unPid && bloquesAdmin[unaAdmin].pagina == pagina)
			return 1;
	
	}return -1;
}
int buscarUltimaPaginaAsignada(int unPid, t_estructuraADM * bloquesAdmin, int MARCOS)
{
int unaAdmin;
int ultimaPagina=0;
	for (unaAdmin = 0; unaAdmin < MARCOS; unaAdmin++) 
	{
		if(bloquesAdmin[unaAdmin].pid==unPid)
		{
			if(bloquesAdmin[unaAdmin].pagina>ultimaPagina)
			{ultimaPagina=bloquesAdmin[unaAdmin].pagina;}	
		}										
													
	
	}return ultimaPagina;
}