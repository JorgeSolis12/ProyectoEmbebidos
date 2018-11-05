#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include "ClienteTCP.h"
#include <stdint.h>

#define PUERTO 5000
#define TAM_BUFFER 512
#define BUFFSIZE 1
#define	ERROR	-1
#define DIR_IP "192.168.100.7"
#define URL    "calle1.bmp"

unsigned char *imagenRGB, *imagenGray;
bmpInfoHeader info;
FILE *archivo;

int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in direccion_servidor;
	char leer_mensaje[TAM_BUFFER];

	displayInfo( &info );
	printf("-----------------------------------------------\n");
/*	
 *	AF_INET - Protocolo de internet IPV4
 *  htons - El ordenamiento de bytes en la red es siempre big-endian, por lo que
 *  en arquitecturas little-endian se deben revertir los bytes
 */	
	memset (&direccion_servidor, 0, sizeof (direccion_servidor));
	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_port = htons(PUERTO);
/*	
 *	inet_pton - Convierte direcciones de texto IPv4 en forma binaria
 */	
	if( inet_pton(AF_INET, DIR_IP, &direccion_servidor.sin_addr) <= 0 )
	{
		perror("Ocurrio un error al momento de asignar la direccion IP");
		exit(1);
	}
/*
 *	Creacion de las estructuras necesarias para el manejo de un socket
 *  SOCK_STREAM - Protocolo orientado a conexión
 */
	printf("Creando Socket ....\n");
	if( (sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("Ocurrio un problema en la creacion del socket");
		exit(1);
	}
/*
 *	Inicia el establecimiento de una conexion mediante una apertura
 *	activa con el servidor
 *  connect - ES BLOQUEANTE
 */
	printf ("Estableciendo conexion ...\n");
	if( connect(sockfd, (struct sockaddr *)&direccion_servidor, sizeof(direccion_servidor) ) < 0) 
	{
		perror ("Ocurrio un problema al establecer la conexion");
		exit(1);
	}
/*
 *	Inicia la transferencia de datos entre cliente y servidor
	*/
	printf("Abriendo la imagen !!! \n");
	archivo = fopen("calle1.bmp", "rb");
	if(archivo == NULL){
		perror("Archivo No encontrado");
		exit(1);
	}
	printf("Comenzando a enviar imagen\n");

	int byteread = fread(leer_mensaje,1,sizeof(leer_mensaje),archivo);
	while(!feof(archivo)){
		send(sockfd, leer_mensaje, byteread,0);
		byteread = fread(leer_mensaje,1,sizeof(leer_mensaje),archivo);

	}
	printf ("Recibiendo contestacion del servidor ...\n");
	/*if (read (sockfd, &leer_mensaje, TAM_BUFFER) < 0)
	{	
		perror ("Ocurrio algun problema al recibir datos del cliente");
		exit(1);
	}
	
	printf ("El servidor envio el siguiente mensaje: \n%s\n", leer_mensaje);*/
	printf ("Cerrando la aplicacion cliente\n");
/*
 *	Cierre de la conexion
 */
	fclose(archivo);
	close(sockfd);

	return 0;
}
	
unsigned char *reservarMemoria(uint32_t width, uint32_t height) {
	unsigned char *imagen;
	imagen = (unsigned char*) malloc(width*height*sizeof(unsigned char));
	if(imagen == NULL)
	{
		perror("Error al asignar memoria \n");
		exit(EXIT_FAILURE);
	}
	return imagen;
}
 