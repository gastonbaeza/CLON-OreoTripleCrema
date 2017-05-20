#ifndef FUNCIONES_H
#define FUNCIONES_H
#include "estructuras.h"
#include <commons/collections/list.h>

int strlenConBarraN(char * unString);

t_programaSalida * obtenerPrograma(char * unPath);

void cargarPaginas(t_list * paginasParaUsar,int stackRequeridas, char * codigo, int tamaPagina);
int calcularPaginas(int tamanioPagina,int tamanio);
int buscarPaginas(int paginasRequeridas, t_list * paginasParaUsar, t_marco * asignadorSecuencial, t_marco * marcos,int MARCOS, unsigned int tamanioAdministrativas);

void enviarDinamico(int, int, int, void *,int);
void recibirDinamico(int, void *,int);
void handshakeServer(int unSocket,int unServer, void * unBuffer);
void handshakeCliente(int unSocket, int unCliente, void * unBuffer);
#endif