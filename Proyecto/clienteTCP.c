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
#include <errno.h>

#define PUERTO 5000
#define TAM_BUFFER 512
#define BUFFSIZE 1
#define	ERROR	-1
//#define DIR_IP "192.168.100.7"
#define DIR_IP "192.168.0.136"
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
	char* fr_name = "Sobel_paralelo2.bmp";
	char* fr_nombre = "Sobel_paraleloRev.bmp";

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
	fclose(archivo);
	sleep(5);
	printf ("Recibiendo contestacion del servidor ...\n");
	/*if (read (sockfd, &leer_mensaje, TAM_BUFFER) < 0)
	{	
		perror ("Ocurrio algun problema al recibir datos del cliente");
		exit(1);
	}*/

	
	//printf ("El servidor envio el siguiente mensaje: \n%s\n", leer_mensaje);
	/*FILE* archivo = fopen("Sobel_paralelo2.bmp", "wb");

         //Se abre el archivo para escritura
         int recibido ;
         while((recibido= recv(sockfd,leer_mensaje, TAM_BUFFER, 0)) != 0){
            printf("recibiendo... \n");
            fwrite(leer_mensaje,sizeof(unsigned char),recibido,archivo);
            fflush(stdout);
         }
         fclose(archivo);
         printf("Archivo Recibido!!!!!!!!!!\n");*/
	/*Receive File from Server 
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
		/*char* fs_name = "calle1.bmp";
		char sdbuf[TAM_BUFFER]; 
		printf("[Client] Sending %s to the Server... ", fs_name);
		FILE *fs = fopen(fs_name, "r");
		if(fs == NULL)
		{
			printf("ERROR: File %s not found.\n", fs_name);
			exit(1);
		}

		bzero(sdbuf, TAM_BUFFER); 
		int fs_block_sz; 
		while((fs_block_sz = fread(sdbuf,1, sizeof(sdbuf), fs)) > 0)
		{
		    if(send(sockfd, sdbuf, fs_block_sz, 0) < 0)
		    {
		        fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
		        break;
		    }
		    bzero(sdbuf, TAM_BUFFER);
		}
		printf("Ok File %s from Client was Sent!\n", fs_name);
	//}*/
	sleep(5);
	/* Receive File from Server */
	printf("[Client] recibiendo Archivo\n");
	FILE *fr = fopen(fr_name, "a");
	char sdbuf[TAM_BUFFER]; 
	if(fr == NULL)
		printf("Archivo %s no pudo abrise.\n", fr_name);
	else
	{
		bzero(revbuf, TAM_BUFFER);
		/*int recibido ;
         while((recibido= recv(sockfd,revbuf, TAM_BUFFER, 0)) > 0){
            printf("recibiendo...\n");
            fwrite(revbuf,sizeof(unsigned char),recibido,fr);
            fflush(stdout);
         }
         fclose(fr);*/
		int fr_block_sz = 0;
	    while((fr_block_sz = recv(sockfd, revbuf, TAM_BUFFER, 0)) > 0)
	    {
			int write_sz = fwrite(revbuf, sizeof(unsigned char), fr_block_sz, fr);
	        if(write_sz < fr_block_sz)
			{
	            perror("Fallo al abrir el archivo.\n");
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
			if (errno == EAGAIN)
			{
				printf("recv() timed out.\n");
			}
			else
			{
				fprintf(stderr, "recv() failed due to errno = %d\n", errno);
			}
		}
		 
	    printf("Ok recibido del servidor!\n");
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