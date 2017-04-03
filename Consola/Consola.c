 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <errno.h>
 #include <string.h>
 #include <netdb.h>
 #include <sys/types.h>
 #include <netinet/in.h>
 #include <sys/socket.h>
 #define MAXDATASIZE 100 // máximo número de bytes que se pueden leer de una vez 

int main(int argc, char *argv[])
    {
	/*
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    LECTURA DE LOS ARCHIVOS DE CONFIGURACION
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    */
    FILE *cfg = fopen ("/home/utnso/tp-2017-1c-Oreo-Triple-Crema/Consola/consolaCFG.txt", "r");
    int IP_KERNEL,PUERTO_KERNEL;
    fscanf(cfg, "IP_KERNEL=%i\n",&IP_KERNEL);
    fscanf(cfg, "PUERTO_KERNEL=%i",&PUERTO_KERNEL);

    fclose(cfg);
    /*
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    */
        int socketNuevo, numbytes;  
        char buf[MAXDATASIZE];
        struct hostent *nombreHost;
        struct sockaddr_in dirDestino; // información de la dirección de destino 

        if (argc != 2) {
            			fprintf(stderr,"usage: client hostname\n");
            			exit(1);
        				}

        if ((nombreHost=gethostbyname(argv[1])) == NULL) 
        				{  
        				// obtener información de máquina 
            			perror("gethostbyname");
            			exit(1);
        				}

        if ((socketNuevo = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        				{
        				perror("socket");
        				exit(1);
        				}

dirDestino.sin_family = AF_INET;    // Ordenación de bytes de la máquina 
dirDestino.sin_port = htons(PUERTO);  // short, Ordenación de bytes de la red 
dirDestino.sin_addr = *((struct in_addr *)nombreHost->h_addr);
memset(&(dirDestino.sin_zero), 8);  // poner a cero el resto de la estructura 

        if (connect(socketNuevo, (struct sockaddr *)&dirDestino,sizeof(struct sockaddr)) == -1) 
        				{
            			perror("connect");
            			exit(1);
        				}

        if ((numbytes=recv(socketNuevo, buf, MAXDATASIZE-1, 0)) == -1) 
        				{
            			perror("recv");
            			exit(1);
        				}

buf[numbytes] = '\0';
printf("Received: %s",buf);
close(socketNuevo);
return 0;
    } 