#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "pgm.h"

#define REPETITIONS 10

int getPixelCPU(PGMData *input,int y, int x)
{
	if( x<0 || x>=input->width )
		return 0;
	if( y<0 || y>=input->height )
		return 0;
	return input->image[y*input->width+x];
}

void sobelCPU(PGMData *input, PGMData *output)
{
	int i, j;
	int Gx, Gy;
	int tempPixel;

	//za vsak piksel v sliki
	for(i=0; i<(input->height); i++)
		for(j=0; j<(input->width); j++)
		{
			Gx = -getPixelCPU(input,i-1,j-1)
				 -2*getPixelCPU(input,i-1,j)
				 -getPixelCPU(input,i-1,j+1)
				 +getPixelCPU(input,i+1,j-1)
				 +2*getPixelCPU(input,i+1,j)
				 +getPixelCPU(input,i+1,j+1);

			Gy = -getPixelCPU(input,i-1,j-1)
				 -2*getPixelCPU(input,i,j-1)
				 -getPixelCPU(input,i+1,j-1)
				 +getPixelCPU(input,i-1,j+1)
				 +2*getPixelCPU(input,i,j+1)
				 +getPixelCPU(input,i+1,j+1);

			tempPixel = sqrt((float)(Gx*Gx+Gy*Gy));
			if( tempPixel>255 )
				output->image[i*output->width+j] = 255;
			else
				output->image[i*output->width+j] = tempPixel;
		}
}


int main(int argc, char *argv[])
{
	clock_t startCPU, endCPU;
	PGMData slikaInput, slikaCPU;

	readPGM("slika.pgm",&slikaInput);
	slikaCPU.height=slikaInput.height;
	slikaCPU.width=slikaInput.width;
	slikaCPU.max_gray=slikaInput.max_gray;

	//CPU del
	slikaCPU.image=(int *)malloc(slikaCPU.height*slikaCPU.width*sizeof(int));
	startCPU=clock();
	for(int i=0; i<REPETITIONS; i++)
		sobelCPU(&slikaInput,&slikaCPU);
	endCPU=clock();
	printf("Time CPU: %d\n",endCPU-startCPU);
	writePGM("sobelCPU.pgm",&slikaCPU);

	return 0;
}
