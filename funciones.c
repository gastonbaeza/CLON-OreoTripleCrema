
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
	#define SOLICITUDLINEA 62
	#define ALMACENARBYTES 95
#define CPU 3
#define FS 4
#define TRUE 1
#define FALSE 0
#define OK 1
#define FAIL 0
#define LIBRE 0
#define OCUPADO 1
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
void generarDumpCache( t_estructuraCache* memoriaCache, int ENTRADAS_CACHE, int MARCO_SIZE)
{
	int unFrame;
	for(unFrame=0;unFrame<ENTRADAS_CACHE;unFrame++)
	{

		printf("%i\n",memoriaCache[unFrame].frame);
		printf("%i\n",memoriaCache[unFrame].pid);
		printf("%p\n",memoriaCache[unFrame].contenido);
	}
}
void generarDumpMemoria(t_marco * marcos, int MARCOS)
{
	int unMarco=0;
	for(unMarco;unMarco<MARCOS;unMarco++)
	{
		printf("%p\n",marcos[unMarco].numeroPagina);
	}
}
int estaEnCache(int unPid,int pagina, t_estructuraCache * memoriaCache, int  ENTRADAS_CACHE)// si esta presente en cache me devuelve la posicion de la entrada, sino devuelve -1
{	int paginaEncontrada=0;
	int paginasRecorridas=0;
	for(paginasRecorridas;paginasRecorridas<ENTRADAS_CACHE;paginasRecorridas++)
	{
		if(memoriaCache[paginasRecorridas].pid == unPid && memoriaCache[paginasRecorridas].frame==pagina) return paginasRecorridas;
	}return -1;
}
void incrementarAntiguedadPorAcceso(t_estructuraCache* memoriaCache, int ENTRADAS_CACHE)//cada vez que hago un acceso tengo que cambiar la antiguedad de todos
{
	int unaEntrada=0;
	for (unaEntrada; unaEntrada< ENTRADAS_CACHE; unaEntrada++)
	{
			memoriaCache[unaEntrada].antiguedad+=1;
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
void escribirEnCache(int unPid, int pagina,void * contenido,t_estructuraCache * memoriaCache, int ENTRADAS_CACHE ,int offset, int tamanio)
{	
	int entrada;
	if(-1!=(entrada=estaEnCache(unPid,pagina, memoriaCache, ENTRADAS_CACHE)))
	{		
			
			memcpy(memoriaCache[entrada].contenido+offset,contenido,tamanio);
	}
	else
	{
		if(-1!=(entrada=hayEspacioEnCache(memoriaCache, ENTRADAS_CACHE)))
		{
			
			memcpy(memoriaCache[entrada].contenido+offset,contenido,tamanio);
		}
		else //este es el LRU
		{
			entrada=buscarEntradaMasAntigua(memoriaCache,ENTRADAS_CACHE);
			memcpy(memoriaCache[entrada].contenido+offset,contenido,tamanio);
		}
	}
	memoriaCache[entrada].antiguedad=0;
	incrementarAntiguedadPorAcceso(memoriaCache,ENTRADAS_CACHE); 
}
void * solicitarBytescache(int unPid, int pagina, t_estructuraCache * memoriaCache, int ENTRADAS_CACHE ,int offset, int tamanio)
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
			if(memoriaCache[unaEntrada].pid!=-1) return unaEntrada;
	}return -1;
	
}
void * solicitarBytes(int unPid, int pagina, t_marco * marcos, int MARCOS,int offset, int tamanio)// en memoria despues de queme solicitan o almacenan bytes tengo que escribirlos en cache, no los hago aca porque sino esta funcion hace mas de lo que debe
{	void * buffer=malloc(tamanio);
	int entrada=buscarPagina(unPid, pagina,  marcos,  MARCOS);
	memcpy(buffer, marcos[entrada].numeroPagina+offset,tamanio);
	return buffer;

}
void almacenarBytes(int unPid, int pagina,void * contenido,t_marco * marcos, int MARCOS ,int offset, int tamanio )
{
	int entrada=buscarPagina(unPid,pagina,marcos, MARCOS); //agregar retardo por lectura
	memcpy(marcos[entrada].numeroPagina+offset,contenido,tamanio); // agregar retardo por escritura
}
int buscarPagina(int unPid,int pagina, t_marco * marcos, int MARCOS)
{

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
		fflush(stdout);printf("%i\n",bloquesAdmin[unaAdmin].frame);
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
		while(bloquesAdmin[unFrame].estado==LIBRE && cantidadRestantes!=0){
			cantidadRestantes --;
    		unFrame++;encontradas++;
    	}
    	if(cantidadRestantes==0){
    		return encontradas;
    	}
    	unFrame++;	
    	paginasRecorridas++;
    }
return FAIL;
	
}
void cargarPaginas(t_list * paginasParaUsar,int stackRequeridas, char * codigo, int marco_size)
{
int unaPagina;

int paginasRequeridas=list_size(paginasParaUsar)-stackRequeridas;
for ( unaPagina = 0; unaPagina < paginasRequeridas; unaPagina++)
{
memcpy((void *)list_get(paginasParaUsar,unaPagina),codigo,marco_size-1);
}

for(unaPagina=paginasRequeridas;unaPagina<paginasRequeridas+stackRequeridas;unaPagina++)
{
	char * stack=calloc(marco_size,marco_size);
	memcpy((void *)list_get(paginasParaUsar,unaPagina),stack,marco_size);
}

}




int calcularPaginas(int tamanioPagina,int tamanio)
{
	double cantidadPaginas;
	int cantidadReal;
	cantidadPaginas=(tamanio+tamanioPagina-1)/tamanioPagina;
 	 return cantidadPaginas;
 							
}
 							

int buscarPaginas(int paginasRequeridas, t_list * paginasParaUsar, int MARCOS, t_estructuraADM * bloquesAdmin, t_marco * marcos,int unPid)
 {	int cantidadPaginas=0;
 	int paginasRecorridas=0;
 	int unFrame=0;
 	int cantidadRestantes;
    int numeroPagina=0;
    
    printf("cantidad de paginas requeridas : %i\n",paginasRequeridas);
    while(paginasRecorridas+paginasRequeridas<MARCOS){
    	cantidadRestantes=paginasRequeridas;
    
		while(bloquesAdmin[unFrame].estado==LIBRE && cantidadRestantes!=0){
	    	cantidadRestantes --;
	    		unFrame++;
	    }
    	if(cantidadRestantes==0){
    		unFrame-=paginasRequeridas;
    		for(unFrame;unFrame<paginasRequeridas+unFrame;unFrame++){
	    		list_add(paginasParaUsar,(marcos[unFrame]).numeroPagina); // ACA ES DONDE TENGO QUE EMPEZAR A ENCARAR EL HASHEO INTENSO
	    		bloquesAdmin[unFrame].estado=OCUPADO;
	    		bloquesAdmin[unFrame].pid=unPid;
	    		bloquesAdmin[unFrame].pagina=numeroPagina;
	    		paginasRequeridas--; numeroPagina++;
    		}
    		return OK;
    	} 
    	unFrame++;
    	paginasRecorridas++;
    }
    return FAIL;
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
int dserial_string(char * unString,int unSocket)
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
return tamanio;
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
	int i,j;
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
	while(0>recv(unSocket,&(pcb->referenciaATabla),sizeof(int),0));
	send(unSocket,buffer, sizeof(int),0);
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
	pcb->indiceEtiquetas.etiquetas=malloc(pcb->indiceEtiquetas.etiquetas_size);
	dserial_string(pcb->indiceEtiquetas.etiquetas,unSocket);
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
	programaSalida->tamanio=dserial_string(programaSalida->elPrograma,unSocket);}

void dserial_resultadoIniciarPrograma(t_resultadoIniciarPrograma* resultadoIniciarPrograma, int unSocket){

	dserial_int(&(resultadoIniciarPrograma->pid),unSocket);
	dserial_int(&(resultadoIniciarPrograma->resultado),unSocket);

}
void serial_resultadoIniciarPrograma(t_resultadoIniciarPrograma* resultadoIniciarPrograma, int unSocket){

	serial_int(&(resultadoIniciarPrograma->pid),unSocket);
	serial_int(&(resultadoIniciarPrograma->resultado),unSocket);
}
void serial_arrayPids(int * arraypids,int unSocket){

	serial_int(arraypids,unSocket);
}

void serial_programaSalida(t_programaSalida * programaSalida, int unSocket)
{
	
	serial_string(programaSalida->elPrograma,programaSalida->tamanio,unSocket); 
}
void dserial_path(t_path * path, int unSocket)
{	
	path->tamanio=dserial_string(path->path,unSocket);}

void serial_path(t_path * path, int unSocket)
{
	
	serial_string(path->path,path->tamanio,unSocket); 
}
void dserial_mensaje(t_mensaje * mensaje, int unSocket)
{	
	mensaje->tamanio=dserial_string(mensaje->mensaje,unSocket);}

void serial_mensaje(t_mensaje * mensaje, int unSocket)
{
	
	serial_string(mensaje->mensaje,mensaje->tamanio,unSocket); 
}
void dserial_linea(t_linea * linea, int unSocket)
{	
	linea->tamanio=dserial_string(linea->linea,unSocket);}

void serial_linea(t_linea * linea, int unSocket)
{
	
	serial_string(linea->linea,linea->tamanio,unSocket); 
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
	t_seleccionador * seleccionador=malloc(sizeof(t_seleccionador));
	seleccionador->tipoPaquete=tipoPaquete;
	int * buffer=malloc(sizeof(int));
	int a=1;
	memcpy(buffer,&a,sizeof(int));
	printf("antes seleccionador\n");
	send(unSocket,seleccionador,sizeof(t_seleccionador),0);
	printf("tipoPaquete en enviarDinamico: %i \n", seleccionador->tipoPaquete);
	while(0>=recv(unSocket,buffer, sizeof(int),0));
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

		case FINALIZARPROGRAMA: 
			serial_int((int*)paquete,unSocket);
			
		break;

		case MENSAJE:	
			serial_mensaje((t_mensaje * )paquete,unSocket);			
		break;

		case LINEA:	
			serial_linea((t_linea * )paquete,unSocket);			
		break;

		case PCB:
			printf("case pcb de enviar\n");	
			serial_pcb((t_pcb *)paquete,unSocket);
		break;
		case ALMACENARBYTES:
		break;
		case ARRAYPIDS:
			serial_arrayPids((int *)paquete,unSocket);
		break;
		case RESULTADOINICIARPROGRAMA:
			serial_resultadoIniciarPrograma((t_resultadoIniciarPrograma*)paquete,unSocket);
		break;
		default : fflush(stdout); printf("%s\n","el paquete que quiere enviar es de un formato desconocido"); 
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
		printf("case pcb de recibir\n");
					dserial_pcb((t_pcb *)paquete,unSocket);
		break;
		case RESULTADOINICIARPROGRAMA:
					dserial_resultadoIniciarPrograma((t_resultadoIniciarPrograma*)paquete,unSocket);
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
		estructuraPrograma->elPrograma = malloc (estructuraPrograma->tamanio);
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
