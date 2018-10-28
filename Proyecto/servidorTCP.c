#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>
#include <math.h>
#include "ClienteTCP.h"

#define PUERTO 			5000	//Número de puerto asignado al servidor
#define COLA_CLIENTES 	5 		//Tamaño de la cola de espera para clientes
#define TAM_BUFFER 		100
#define EVER            1 
#define BUFFSIZE        1
#define ERROR           -1
#define DIMASK          3 //especifica las dimensiones de la mascara, aqui especificamos una de 3x3
#define SIGMA           2
#define NUM_HILOS       4

void ISRsw(int sig);
void *filtroImagen(void *arg);
unsigned char *abrirBMPEnviado(bmpInfoHeader *bInfoHeader,FILE *f);
void GraytoRGB( unsigned char *imagenGray, unsigned char *imagenRGB, uint32_t width, uint32_t height);
unsigned char *reservarMemoria(uint32_t width, uint32_t height);

bmpInfoHeader info;
pthread_mutex_t bloqueo;
unsigned char *imagenRev, *imagenGray, *imagenFiltro;

int main(int argc, char **argv)
{
      pid_t pid;
   	int sockfd, cliente_sockfd;
   	struct sockaddr_in direccion_servidor; //Estructura de la familia AF_INET, que almacena direccion
   	char leer_mensaje[TAM_BUFFER];
      FILE *archivo;
      register int nh;
      pthread_t tids[NUM_HILOS];
      int nhs[NUM_HILOS] , *res;
/*	
 *	AF_INET - Protocolo de internet IPV4
 *  htons - El ordenamiento de bytes en la red es siempre big-endian, por lo que
 *  en arquitecturas little-endian se deben revertir los bytes
 *  INADDR_ANY - Se elige cualquier interfaz de entrada
 */	
   	memset (&direccion_servidor, 0, sizeof (direccion_servidor));	//se limpia la estructura con ceros
   	direccion_servidor.sin_family 		= AF_INET;
   	direccion_servidor.sin_port 		= htons(PUERTO);
   	direccion_servidor.sin_addr.s_addr 	= INADDR_ANY;

      if(signal(SIGUSR1, ISRsw) == SIG_ERR){
         perror("Error en la ISR");
         exit(EXIT_FAILURE);
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
 *  bind - Se utiliza para unir un socket con una dirección de red determinada
 */
   	printf("Configurando socket ...\n");
   	if( bind(sockfd, (struct sockaddr *) &direccion_servidor, sizeof(direccion_servidor)) < 0 )
	{
		perror ("Ocurrio un problema al configurar el socket");
		exit(1);
   	}
/*
 *  listen - Marca al socket indicado por sockfd como un socket pasivo, esto es, como un socket
 *  que será usado para aceptar solicitudes de conexiones de entrada usando "accept"
 *  Habilita una cola asociada la socket para alojar peticiones de conector procedentes
 *  de los procesos clientes
 */
   	printf ("Estableciendo la aceptacion de clientes...\n");
	if( listen(sockfd, COLA_CLIENTES) < 0 )
	{
		perror("Ocurrio un problema al crear la cola de aceptar peticiones de los clientes");
		exit(1);
   	}
/*
 *  accept - Aceptación de una conexión
 *  Selecciona un cliente de la cola de conexiones establecidas
 *  se crea un nuevo descriptor de socket para el manejo
 *  de la nueva conexion. Apartir de este punto se debe
 *  utilizar el nuevo descriptor de socket
 *  accept - ES BLOQUEANTE 
 */
for(;EVER;){
   	printf ("En espera de peticiones de conexión ...\n");
   	if( (cliente_sockfd = accept(sockfd, NULL, NULL)) < 0 )
	{
		 perror("Ocurrio algun problema al atender a un cliente");
		 exit(1);
   	}
      pid = fork();
      if(!pid){ //proceso hijo...
/*
 *	Inicia la transferencia de datos entre cliente y servidor
 */
      	printf("Se aceptó un cliente, atendiendolo \n");
      	if( read (cliente_sockfd, &leer_mensaje, TAM_BUFFER) < 0 )
   	   {
   		perror ("Ocurrio algun problema al recibir datos del cliente");
   		exit(1);
      	}
         char buffer[BUFFSIZE];
         int recibido = -1;

          /*Se abre el archivo para escritura*/
         while((recibido = recv(sockfd, buffer, BUFFSIZE, 0)) > 0){
            printf("%s",buffer);
            fwrite(buffer,sizeof(char),1,archivo);
         }

         fclose(archivo);
         printf("Archivo Recibido!!!!!!!!!!\n");

         imagenRev = abrirBMPEnviado(&info,archivo);
         imagenGray = RGBtoGray( imagenRev,  info.width, info.height);
         imagenFiltro = reservarMemoria (info.width, info.height);

         printf("creando hilos \n");
         for(nh =0; nh<NUM_HILOS; nh++){
            nhs[nh] = nh;
            pthread_create(&tids[nh], NULL,filtroImagen,(void*)&nhs[nh]);
         }
         for(nh =0; nh<NUM_HILOS; nh++){

            pthread_join(tids[nh], (void **)&res);
         }

         pthread_mutex_destroy(&bloqueo);
         GraytoRGB(imagenFiltro, imagenRev, info.width, info.height) ;

         guardarBMP( "Sobel_paralelo.bmp", &info, imagenRev);
/*
      	printf ("El cliente nos envio el siguiente mensaje: \n %s \n", leer_mensaje);
      	if( write (cliente_sockfd, "Bienvenido cliente", 19) < 0 )
   	  {
   		perror("Ocurrio un problema en el envio de un mensaje al cliente");
   		exit(1);
      	}
*/
      	printf("Concluimos la ejecución de la aplicacion Servidor \n");
   /*
    *	Cierre de las conexiones
    */

      	close (cliente_sockfd);
         kill(getppid(),SIGUSR1);
         exit(0);
         //fin del proceso Hijo
      }
      
}
      free(imagenGray);
      close (sockfd);

	return 0;
}

void ISRsw(int sig){
   
   int status;
   pid_t pid;
   if(sig == SIGUSR1){//SEÑAL ENVIADA DEL PROCESO HIJO
      pid = wait( &status );
      printf("señal recibida y proceso terminado del hijo %d \n", pid);
   }
}

unsigned char *RGBtoGray( unsigned char *imagenRGB, uint32_t width, uint32_t height)
{
   register int y,x ;
   int indiceRGB, indiceGray;
   unsigned char grayLevel;
   unsigned char *imagenGray;
   imagenGray = reservarMemoria(width,height);
   if(imagenGray == NULL)
   {
      perror("Error al asignar memoria \n");
      exit(EXIT_FAILURE);
   }

   for ( y = 0; y < height ; y++ )
      for ( x = 0; x < width; x++ )
      {
         indiceGray = (width * y + x);
         //indiceRGB = indiceGray * 3 ;//multiplicar por 3 con corrimientos
         indiceRGB = ((indiceGray << 1) + indiceGray);
         grayLevel = (30*imagenRGB[indiceRGB]+59*imagenRGB[indiceRGB+1]+11*imagenRGB[indiceRGB+2])/100;
         imagenGray[indiceGray] = grayLevel;
      }
   printf("Imagen en Escala de grises creada\n");
   return imagenGray;
}

void *filtroImagen(void *arg){

   int nh = *(int*)arg;
   int tambloque=info.width/NUM_HILOS;
   int inicio = nh*tambloque;
   int final= inicio+tambloque;
   register int x,y,xm,ym;
   int conv1,conv2, indice,indicem;
   char GradF[] =
            {1,0,-1,
             2,0,-2,
             1,0,-1};
   char GradC[] =
            {-1,-2,-1,
              0, 0,-0,
              1, 2, 1};

   for( y = 0; y <= (info.height-DIMASK); y++)//Con esta pareja de ciclos se recorre la imagen
      for ( x = inicio; x <= final; x++) //por completo
      {
            conv1 = 0;
            conv2 = 0;
            int magnitud=0;
            for(ym=y;ym< y+DIMASK;ym++)//for (ym = 0; ym < DIMASK; ym++ )//con esta pareja podemos recorrer
            {  
               for(xm=x;xm< x+DIMASK;xm++)//for (xm = 0; xm < DIMASK; xm++ )//la mascara
               {
                  //indice = (width * (y +ym) + (x+xm));
                  indice = ym*info.width+xm;
                  indicem = (ym-y)*DIMASK+(xm-x);
                  conv1 += (int)imagenGray[indice]*GradF[indicem];
                  conv2 += (int)imagenGray[indice]*GradC[indicem];
               }
            }
               conv1 /= 4;//la division se hace con la suma de los coeficientes de la mascara, en este caso 9.
               conv2 /= 4;
               int add = pow(conv1,2)+pow(conv2,2);
               magnitud = sqrt(add);
               indice = ((y+1)*info.width + (x+1));
               pthread_mutex_lock(&bloqueo);
               imagenFiltro[indice]= (unsigned char)magnitud;
               pthread_mutex_unlock(&bloqueo);
      }
   pthread_exit(arg);
}

unsigned char *abrirBMPEnviado(bmpInfoHeader *bInfoHeader ,FILE *f){

   bmpFileHeader header;     /* cabecera */
   unsigned char *imgdata;   /* datos de imagen */
   uint16_t type;        /* 2 bytes identificativos */

   f = fopen ("archivoRecibido", "r");
   if( f  == NULL )
   {
      perror("Error al abrir el archivo en lectura");
      exit(EXIT_FAILURE);
   }

   /* Leemos los dos primeros bytes */
   fread( &type, sizeof(uint16_t), 1, f );
   if( type != 0x4D42 )        /* Comprobamos el formato */
      {
      printf("Error en el formato de imagen, debe ser BMP de 24 bits");
            fclose(f);
            return NULL;
      }

   /* Leemos la cabecera de fichero completa */
   fread( &header, sizeof(bmpFileHeader), 1, f );

   /* Leemos la cabecera de información completa */
   fread( bInfoHeader, sizeof(bmpInfoHeader), 1, f );

   /* Reservamos memoria para la imagen, ¿cuánta?
      Tanto como indique imgsize */
   imgdata = (unsigned char *)malloc( bInfoHeader->imgsize );
   if( imgdata == NULL )
   {
      perror("Error al asignar memoria");
      exit(EXIT_FAILURE);
   }
   /* Nos situamos en el sitio donde empiezan los datos de imagen,
      nos lo indica el offset de la cabecera de fichero*/
   fseek(f, header.offset, SEEK_SET);

   /* Leemos los datos de imagen, tantos bytes como imgsize */
   fread(imgdata, bInfoHeader->imgsize,1, f);

   /* Cerramos el apuntador del archivo de la imagen*/
   fclose(f);

   /* Devolvemos la imagen */
   return imgdata;
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

void GraytoRGB( unsigned char *imagenGray, unsigned char *imagenRGB, uint32_t width, uint32_t height){
   register int y,x ;
   int indiceRGB, indiceGray;
   //imagenGray = (unsigned char*) malloc(width*height*sizeof(unsigned char));
        for ( y = 0; y < height ; y++ )
                for ( x = 0; x < width; x++ )
                {
         indiceGray = (y*width+x);
         indiceRGB = indiceGray * 3;

         imagenRGB[indiceRGB+0] = imagenGray [indiceGray];
         imagenRGB[indiceRGB+1] = imagenGray [indiceGray];
         imagenRGB[indiceRGB+2] = imagenGray [indiceGray];

      }
}