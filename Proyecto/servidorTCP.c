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
#include <errno.h>

#define PUERTO      5000  //Número de puerto asignado al servidor
#define COLA_CLIENTES   5     //Tamaño de la cola de espera para clientes
#define TAM_BUFFER    512
#define EVER            1 
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
unsigned char *imagenRGB, *imagenGray, *imagenFiltro, *imagenRev;

void error(const char *msg)
{
  perror(msg);
  exit(1);
}
int main(int argc, char **argv)
{
      pid_t pid;
    int sockfd, cliente_sockfd;
    struct sockaddr_in direccion_servidor; //Estructura de la familia AF_INET, que almacena direccion
      register int nh;
      pthread_t tids[NUM_HILOS];
      int nhs[NUM_HILOS] , *res;
      char leer_mensaje[TAM_BUFFER];
      char sdbuf[TAM_BUFFER];
      char revbuf[TAM_BUFFER];

/*  
 *  AF_INET - Protocolo de internet IPV4
 *  htons - El ordenamiento de bytes en la red es siempre big-endian, por lo que
 *  en arquitecturas little-endian se deben revertir los bytes
 *  INADDR_ANY - Se elige cualquier interfaz de entrada
 */ 
    memset (&direccion_servidor, 0, sizeof (direccion_servidor)); //se limpia la estructura con ceros
    direccion_servidor.sin_family     = AF_INET;
    direccion_servidor.sin_port     = htons(PUERTO);
    direccion_servidor.sin_addr.s_addr  = INADDR_ANY;

      if(signal(SIGUSR1, ISRsw) == SIG_ERR){
         perror("Error en la ISR");
         exit(EXIT_FAILURE);
      }
/*
 *  Creacion de las estructuras necesarias para el manejo de un socket
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
 *  Inicia la transferencia de datos entre cliente y servidor
 */
         char buffer[TAM_BUFFER];
         printf("Se aceptó un cliente, atendiendolo \n");
        /*if( read (cliente_sockfd, buffer, TAM_BUFFER) < 0 )
       {
      perror ("Ocurrio algun problema al recibir datos del cliente");
      exit(1);
        }
         if( write (cliente_sockfd, "Bienvenido Cliente", 19) < 0 )
         {
            perror("Ocurrio un problema en el envio de un mensaje al cliente");
            exit(1);
         }*/
         printf("Comenzando a recibir datos\n");
         
         FILE* archivo = fopen("Sobel_Rev.bmp", "wb");

         //Se abre el archivo para escritura
         int recibido ;
         while((recibido= recv(cliente_sockfd,buffer, TAM_BUFFER, 0)) != 0){
            printf("recibiendo... \n");
            fwrite(buffer,sizeof(unsigned char),recibido,archivo);
            fflush(stdout);
         }
         fclose(archivo);
         printf("Archivo Recibido!!!!!!!!!!\n");
         
        
      /*Receive File from Client */
    /*char* fr_name = "Sobel_Rev.bmp";
    FILE *fr = fopen(fr_name, "a");
    if(fr == NULL)
      printf("File %s Cannot be opened file on server.\n", fr_name);
    else
    {
      bzero(revbuf, TAM_BUFFER); 
      int fr_block_sz = 0;
      while((fr_block_sz = recv(cliente_sockfd, revbuf, TAM_BUFFER, 0)) > 0) 
      {
          int write_sz = fwrite(revbuf, sizeof(unsigned char), fr_block_sz, fr);
        if(write_sz < fr_block_sz)
          {
              error("File write failed on server.\n");
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
          exit(1);
              }
          }
      printf("Ok received from client!\n");
      fclose(fr); 
    }*/

      imagenRGB = abrirBMP("Sobel_Rev.bmp",&info);
      imagenGray = RGBtoGray(imagenRGB, info.width, info.height);
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
      GraytoRGB(imagenFiltro, imagenRGB, info.width, info.height) ;

      guardarBMP( "Sobel_paralelo.bmp", &info, imagenRGB);
      //printf("Enviando imagen a cliente \n");

      /*archivo = fopen("Sobel_paralelo.bmp", "rb");
      if(archivo == NULL){
         perror("Archivo No encontrado");
         exit(1);
      }*/
      /* Send File to Client */

           /*printf("Enviando archivo");
           FILE *fs = fopen("Sobel_paralelo", "r");
           if(fs == NULL)
           {
             printf("ERROR: al enviar el archivo");
             exit(1);
           }

           bzero(sdbuf, TAM_BUFFER); 
           int fs_block_sz; 
           while((fs_block_sz = fread(sdbuf, sizeof(unsigned char), TAM_BUFFER, fs))>0)
           {
               if(send(cliente_sockfd, sdbuf, fs_block_sz, 0) < 0)
               {
                   printf("ERROR:Al enviar el archivo ");
                   exit(1);
               }
               bzero(sdbuf, TAM_BUFFER);
           }
           printf("Ok sent to client!\n");
   }*/
}
      /* Send File to Client */
      //if(!fork())
      //{
          /* Send File to Client */
    //if(!fork())
    //{
        char* fs_name = "Sobel_paralelo.bmp";
        printf("[Server] Sending %s to the Client...", fs_name);
        FILE *fs = fopen(fs_name, "r");
        if(fs == NULL)
        {
            fprintf(stderr, "ERROR: File %s not found on server. (errno = %d)\n", fs_name, errno);
        exit(1);
        }

        bzero(sdbuf, TAM_BUFFER); 
        int fs_block_sz; 
        /*while((fs_block_sz = fread(sdbuf, sizeof(unsigned char), TAM_BUFFER, fs))>0)
        {
            if(send(cliente_sockfd, sdbuf, fs_block_sz, 0) < 0)
            {
                fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
                exit(1);
            }
            bzero(sdbuf, TAM_BUFFER);
        }*/
        int byteread = fread(leer_mensaje,1,sizeof(leer_mensaje),fs);
        while(!feof(fs)){
          send(sockfd, leer_mensaje, byteread,0);
          byteread = fread(leer_mensaje,1,sizeof(leer_mensaje),fs);
        }
  fclose(archivo);
        printf("Ok sent to client!\n");
        close(cliente_sockfd);
        printf("[Server] Connection with Client closed. Server will wait now...\n");
        while(waitpid(-1, NULL, WNOHANG) > 0);
    //}

      free(imagenGray);
 /*
    * Cierre de las conexiones
    */
      }
         close (cliente_sockfd);
         kill(getppid(),SIGUSR1);
         exit(0);
         //fin del proceso Hijo

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

void *filtroImagen(void *arg){

   int nh = *(int*)arg;
   int tambloque=info.width/NUM_HILOS;
   int inicio = nh*tambloque;
   int final= inicio+tambloque;
   register int x,y,xm,ym;
   int conv1,conv2, indice,indicem;
   int GradF[] =
            {1,0,-1,
             2,0,-2,
             1,0,-1};
   int GradC[] =
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