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
//#define DIR_IP "192.168.100.7"
#define DIR_IP "192.168.100.181"
#define URL    "calle1.bmp"

unsigned char *imagenRGB, *imagenGray;
bmpInfoHeader info;
FILE *archivo;
FILE *fr;

int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in direccion_servidor;
	char leer_mensaje[TAM_BUFFER];
	char revbuf[TAM_BUFFER];

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

	/*int byteread = fread(leer_mensaje,1,sizeof(leer_mensaje),archivo);
	while(!feof(archivo)){
		send(sockfd, leer_mensaje, byteread,0);
		byteread = fread(leer_mensaje,1,sizeof(leer_mensaje),archivo);

	}
	fclose(archivo);
	printf ("Recibiendo contestacion del servidor ...\n");
	if (read (sockfd, &leer_mensaje, TAM_BUFFER) < 0)
	{	
		perror ("Ocurrio algun problema al recibir datos del cliente");
		exit(1);
	}

	
	//printf ("El servidor envio el siguiente mensaje: \n%s\n", leer_mensaje);
	FILE* archivo = fopen("Sobel_paralelo2.bmp", "wb");

         //Se abre el archivo para escritura
         int recibido ;
         while((recibido= recv(sockfd,leer_mensaje, TAM_BUFFER, 0)) != 0){
            printf("recibiendo... \n");
            fwrite(leer_mensaje,sizeof(unsigned char),recibido,archivo);
            fflush(stdout);
         }
         fclose(archivo);
         printf("Archivo Recibido!!!!!!!!!!\n");
	//Receive File from Server 
	fr = fopen("Sobel_paralelo2.bmp", "a");
	if(fr == NULL){
		perror("No se pudo abrir archivo");
		exit(EXIT_FAILURE);
	}
	else{
	    bzero(revbuf, TAM_BUFFER); 
	    int fr_block_sz = 0;
	        while((fr_block_sz = recv(sockfd, revbuf, TAM_BUFFER, 0))>= 0){
	            if(fr_block_sz == 0){
	                perror("Error en archivo recivido.\n");
	                exit(EXIT_FAILURE);
	            }
	            int write_sz = fwrite(revbuf, sizeof(unsigned char), fr_block_sz, fr);
	            if(write_sz < fr_block_sz){
	                perror("Error, archivo no escrito.\n");
	                exit(EXIT_FAILURE);
	            }
	            bzero(revbuf, TAM_BUFFER);
	        }
			if(fr_block_sz < 0){
                perror("Error en archivo recibido.\n");
                exit(EXIT_FAILURE);
            }
	        printf("Ok received from server!\n");
	        fclose(fr);
         */
	/* Send File to Server */
	//if(!fork())
	//{
		char sdbuf[TAM_BUFFER]; 
		FILE *fs = fopen("calle1.bmp", "r");
		if(fs == NULL)
		{
			printf("ERROR: Archivo no encontrado.\n");
			exit(1);
		}

		bzero(sdbuf, TAM_BUFFER); 
		int fs_block_sz; 
		while((fs_block_sz = fread(sdbuf, sizeof(unsigned char), TAM_BUFFER, fs)) > 0)
		{
		    if(send(sockfd, sdbuf, fs_block_sz, 0) < 0)
		    {
		        perror("Error al abirir el archivo");
		        exit(EXIT_FAILURE);
		        break;
		    }
		    bzero(sdbuf, TAM_BUFFER);
		}
		printf("Ok File from Client was Sent!\n");
	//}

	/* Receive File from Server */
	printf("Recibiendo archivo del cliente");
	FILE *fr = fopen("Sobel_paralelo2.bmp", "a");
	if(fr == NULL)
		printf("No se pudo abrir el archivo.\n");
	else
	{
		bzero(revbuf, TAM_BUFFER); 
		int fr_block_sz = 0;
	    while((fr_block_sz = recv(sockfd, revbuf, TAM_BUFFER, 0)) > 0)
	    {
			int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
	        if(write_sz < fr_block_sz)
			{
	            perror("File write failed.\n");
	            exit(EXIT_FAILURE);
	        }
			bzero(revbuf, TAM_BUFFER);
			if (fr_block_sz == 0 || fr_block_sz != 512) 
			{
				break;
			}
		}
		if(fr_block_sz < 0)
        {
			printf("error en el recv\n");
			exit(EXIT_FAILURE);
		}
	    printf("Ok received from server!\n");
	    fclose(fr);
	}
		printf ("Cerrando la aplicacion cliente\n");
	
/*	
 *	Cierre de la conexion
 */
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