#ifndef FUNCIONES_H
#define FUNCIONES_H
#include "estructuras.h"
#include <commons/collections/list.h>

int strlenConBarraN(char * unString);

t_programaSalida * obtenerPrograma(char * unPath);

int buscarEnOverflow(int indice, int pid, int pagina,t_estructuraADM * bloquesAdmin,int MARCOS, t_list**overflow);
int esPaginaCorrecta(int frame, int pid, int pagina,t_estructuraADM * bloquesAdmin, int MARCOS);


unsigned int calcularPosicion(int pid, int num_pagina,int MARCOS);
void inicializarOverflow(int MARCOS, t_list**overflow);
void agregarSiguienteEnOverflow(int pos_inicial, int * nro_frame, t_list**overflow);
void borrarDeOverflow(int pos_inicial, int frame, t_list**overflow);

void cargarPaginas(t_list * paginasParaUsar,int stackRequeridas, char * codigo, int tamaPagina);
int calcularPaginas(int tamanioPagina,int tamanio);
int buscarPaginas(int paginasRequeridas, t_list * paginasParaUsar, int MARCOS, t_estructuraADM * bloquesAdmin, t_marco * marcos,int unPid);
void serial_solicitudMemoria(t_solicitudMemoria * solicitud,int  unSocket);
int dserial_string(char ** unString,int unSocket);
void dserial_solicitudMemoria(t_solicitudMemoria * solicitud, int unSocket);
void buscarProcesosActivos(t_list * procesosActivos, t_estructuraADM * bloquesAdmin, int MARCOS);
void serial_string(char * unString,int tamanio,int unSocket);
int cantidadBloquesLibres(int MARCOS, t_estructuraADM * bloquesAdmin);
int cantidadBloquesOcupados(int MARCOS, t_estructuraADM * bloquesAdmin);
void handshakeServer(int unSocket,int unServer, void * unBuffer);
void handshakeCliente(int unSocket, int unCliente, void * unBuffer);
void pagaraprata();

#endif