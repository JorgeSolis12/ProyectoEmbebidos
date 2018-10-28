#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "imagen.h"
#include <stdint.h>
#define DIMASK 3 //especifica las dimensiones de la mascara, aqui especificamos una de 3x3
#define SIGMA 2
#define NUM_HILOS 4

void *funHilo(void*arg);
int * memoria();
void llenarArreglo(int * datos);
void imprimir(int *datos);	
void producto(void );
unsigned char *RGBtoGray( unsigned char *imagenRGB, uint32_t width, uint32_t height);
void GraytoRGB( unsigned char *imagenGray, unsigned char *imagenRGB, uint32_t width, uint32_t height);
void brilloImagen (unsigned char *imagenGray, uint32_t width, uint32_t height);
unsigned char *reservarMemoria(uint32_t width, uint32_t height);
void *filtroImagen(void *arg);

unsigned char *imagenRGB, *imagenGray, *imagenFiltro;
bmpInfoHeader info;
pthread_mutex_t bloqueo;

int main(){
	register int nh;
	pthread_t tids[NUM_HILOS];
	int nhs[NUM_HILOS] , *res;
	

	printf("Abriendo la imagen !!! \n");
	imagenRGB = abrirBMP("calle1.bmp", &info);
	displayInfo( &info );

	imagenGray = RGBtoGray( imagenRGB,  info.width, info.height);
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

	guardarBMP( "Sobel_paralelo.bmp", &info, imagenRGB);//filtro pasabajas
	free(imagenRGB);
	free(imagenGray);
	//free(imagenFiltro);
	return 0;
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
	return imagenGray;
}

void brilloImagen (unsigned char *imagenGray, uint32_t width, uint32_t height)
{
	register int p;
	unsigned short int pixel;
	for (p = 0 ; p < width*height ; p++ )
	{
		pixel = imagenGray[p] + 80;
		imagenGray[p] = (pixel>255)? 255 : (unsigned char)pixel;//si el valor de pixel es mayor a 255, devuelve 255 sino se castea el valor de pixel a unsigned char y se guarda en imagenGray.
	}
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

