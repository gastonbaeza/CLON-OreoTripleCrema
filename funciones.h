#ifndef FUNCIONES_H
#define FUNCIONES_H

int strlenConBarraN(char * unString);

t_programaSalida * obtenerPrograma(char * unPath);
void * limpiarPagina(int tamanioPagina,void * punteroAPagina);
int estaLibreMarco(int tamanioFrame,t_marco marco,int tamanioPagina);


char * enviarDinamico(int, int, int, void *);
char * recibirDinamico(int, int, int, void *);
void handshakeServer(int unSocket,int unServer, void * unBuffer);
void handshakeCliente(int unSocket, int unCliente, void * unBuffer);
#endif