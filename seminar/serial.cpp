// "s"cepec preberemo iz datoteke
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <math.h>
#include <limits.h>
#include "pgm.h"
#include <time.h>
#include <sys/time.h>

double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

int getPixelCPU(PGMData *input,int y, int x)
{
	if( x<0 || x>=input->width )
		return 0;
	if( y<0 || y>=input->height )
		return 0;
	return input->image[y*input->width+x];
}

void sobelCPU(PGMData *input, PGMData *output) {
    // Sobel filter for finding edges.
    output->height=input->height;
	output->width=input->width;
	output->max_gray=input->max_gray;
	output->image=(int *)malloc(input->height * input->width * sizeof(int));

	int i, j;
	int Gx, Gy;
	int tempPixel;

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

int getPixel(PGMData *input, int y, int x) {
    // Get pixel for finding minimal seam.
    // If we are accessing out of image, return big value, so it won't be
    // included in minimal path
	if( x<0 || x>=input->width )
		return INT_MAX;
	if( y<0 || y>=input->height )
		return INT_MAX;
	return input->image[y*input->width+x];
}

int minPixel(PGMData *input, int y, int x) {
    // Get minimum pixel value of neighbors above the current pixel.
    int min;
    y = y - 1;

    int up_left = getPixel(input, y, x-1);
    int up = getPixel(input, y, x);
    int up_right = getPixel(input, y, x+1);

    min = up_left > up ? up : up_left;
    min = up_right > min ? min : up_right;


    return min;
}

int minPixelIndex(PGMData *input, int y, int x) {
    // Get index of pixel with minimum value from neighbors above the current
    // pixel.
    int index;
    int min;
    y = y - 1;

    int up_left = getPixel(input, y, x-1);
    int up = getPixel(input, y, x);
    int up_right = getPixel(input, y, x+1);

    if (up_left > up) {
        if (up_right > up) {
            return x+0;
        } else {
            return x+1;
        }
    } else {
        if (up_right > up_left) {
            return x-1;
        } else {
            return x+1;
        }
    }
}

void findSeam(PGMData *input, PGMData *output) {
    // find smallest seam and mark it with -1.
    PGMData img = *input;
    PGMData out = *output;

    // calculate cost of each seam
    for(int i = 1; i < img.height; i++) {
		for(int j = 0; j < img.width; j++) {
            img.image[i*img.width+j] =
                img.image[i*img.width+j] + minPixel(&img, i, j);
        }
    }

    // find smallest seam
    int last_row = img.height - 1;
    int col = 0;
    int smallest_seam = img.image[last_row*img.width];
    int tmp_seam;

    for(int j = 1; j < img.width; j++) {
        tmp_seam = img.image[last_row*img.width+j];
        if(smallest_seam > tmp_seam) {
            smallest_seam = tmp_seam;
            col = j;
        }
    }

    // reconstruct seam path
    for(int i = img.height-1; i >= 0; i--) {
        int id = i*out.width+col;
        if (id >= 0) {
            out.image[id] = -1;
            // printf("id_____ %d, %d \n", id, col);
        } else {
            out.image[0] = -1;
            // printf("id_fault %d, %d \n", id, col);
        }
        col = minPixelIndex(&img, i, col);
    }

}

void removeSeam(PGMData *input, PGMData *output, int seam_num) {
    // Remove pixels marked with -1.

    output->height = input->height;
	output->width = input->width - seam_num;
	output->max_gray = input->max_gray;
	output->image = (int *)malloc(input->height * input->width * sizeof(int));


    int offset;
    int pixel;
    for(int i = 0; i < input->height; i++) {
		offset = 0;
        for(int j = 0; j < input->width; j++) {
            pixel = input->image[i*input->width+j];
            if (pixel == -1) {
                offset -= 1;
            } else {
                output->image[i*output->width+j+offset] = pixel;
            }
        }
    }
}

void transpose(PGMData *input, PGMData *output) {
    // Transpose image.

    output->height = input->width;
	output->width = input->height;

	output->max_gray = input->max_gray;
	output->image = (int *)malloc(input->height * input->width * sizeof(int));

    int c = 0;
        for(int j = 0; j < input->width; j++) {
            for(int i = input->height-1; i >= 0; i--) {
            printf("%d, %d\n", i*output->width+j, c);
            output->image[c] = input->image[i*input->width+j];
            c++;
        }
    }
}

int main(void) {

    PGMData imgA;
	PGMData imgB;

    // read image
	const char filename[] = "images/tower.pgm";
	readPGM(filename, &imgA);

    int width = imgA.width;
    int height = imgA.height;

    double start = get_wall_time();
        for(int i=0; i<400; i++) {

            // get energy with sobel filter
            sobelCPU(&imgA, &imgB);

            // find smallest seam
            findSeam(&imgB, &imgA);

            free(imgB.image);

            // remove seam
            removeSeam(&imgA, &imgB, 1);

            free(imgA.image);
            imgA.image = imgB.image;
            imgA.width = imgB.width;
            imgA.height = imgB.height;
        }
    double cpu_time = get_wall_time() - start;

    printf("Original image %d × %d \n", width, height);
    printf("Out image %d × %d \n", imgA.width, imgA.height);
    printf("CPU time: %.15fs\n", cpu_time);

    // PGMData imgC;
    // transpose(&imgA, &imgC);
    // writePGM("out_t.pgm", &imgC);

	writePGM("out.pgm", &imgA);

	return 0;
}
