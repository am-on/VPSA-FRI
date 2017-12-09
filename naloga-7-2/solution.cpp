// "s"cepec preberemo iz datoteke
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <math.h>
#include "pgm.h"

#define SIZE			(1024)
#define WORKGROUP_SIZE	(256)
#define MAX_SOURCE_SIZE	16384

#include <time.h>
#include <sys/time.h>
double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
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


int main(void)
{
	char ch;
	int i;
	cl_int ret;

	// int vectorSize = SIZE;
	int width = 0;
	int height = 0;

	const char filename[] = "image.pgm";
	PGMData slika;

	readPGM(filename, &slika);
	int pixelCount = slika.width * slika.height;

	// Branje datoteke
	FILE *fp;
	char *source_str;
	size_t source_size;

	fp = fopen("kernel.cl", "r");
	if (!fp)
	{
		fprintf(stderr, ":-(#\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	source_str[source_size] = '\0';
	fclose(fp);

	// Podatki o platformi
	cl_platform_id	platform_id[10];
	cl_uint			ret_num_platforms;
	char			*buf;
	size_t			buf_len;
	ret = clGetPlatformIDs(10, platform_id, &ret_num_platforms);
	// max. "stevilo platform, kazalec na platforme, dejansko "stevilo platform

	// Podatki o napravi
	cl_device_id	device_id[10];
	cl_uint			ret_num_devices;
	// Delali bomo s platform_id[0] na GPU
	ret = clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_GPU, 10,
		device_id, &ret_num_devices);
	// izbrana platforma, tip naprave, koliko naprav nas zanima
	// kazalec na naprave, dejansko "stevilo naprav

	// Kontekst
	cl_context context = clCreateContext(NULL, 1, &device_id[0], NULL, NULL, &ret);
	// kontekst: vklju"cene platforme - NULL je privzeta, "stevilo naprav,
	// kazalci na naprave, kazalec na call-back funkcijo v primeru napake
	// dodatni parametri funkcije, "stevilka napake

	// Ukazna vrsta
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id[0], 0, &ret);
	// kontekst, naprava, INORDER/OUTOFORDER, napake

	// Delitev dela
	size_t local_item_size = WORKGROUP_SIZE;
	size_t num_groups = ((pixelCount - 1) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;

	// Alokacija pomnilnika na napravi
	cl_mem img_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		pixelCount*sizeof(int), slika.image, &ret);
	cl_mem img_out_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
		pixelCount*sizeof(int), NULL, &ret);

	// Priprava programa
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str,
		NULL, &ret);
	// kontekst, "stevilo kazalcev na kodo, kazalci na kodo,
	// stringi so NULL terminated, napaka

	// Prevajanje
	ret = clBuildProgram(program, 1, &device_id[0], NULL, NULL, NULL);
	// program, "stevilo naprav, lista naprav, opcije pri prevajanju,
	// kazalec na funkcijo, uporabni"ski argumenti
	printf("kernel build %d \n", ret);

	// Log
	size_t build_log_len;
	char *build_log;
	ret = clGetProgramBuildInfo(program, device_id[0], CL_PROGRAM_BUILD_LOG,
		0, NULL, &build_log_len);
	// program, "naprava, tip izpisa,
	// maksimalna dol"zina niza, kazalec na niz, dejanska dol"zina niza
	build_log = (char *)malloc(sizeof(char)*(build_log_len + 1));
	ret = clGetProgramBuildInfo(program, device_id[0], CL_PROGRAM_BUILD_LOG,
		build_log_len, build_log, NULL);
	printf("%s\n", build_log);
	free(build_log);

	// "s"cepec: priprava objekta
	cl_kernel kernel = clCreateKernel(program, "sobel_filter", &ret);
	// program, ime "s"cepca, napaka
	printf("kernel prepare %d \n", ret);

	size_t buf_size_t;
	clGetKernelWorkGroupInfo(kernel, device_id[0], CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(buf_size_t), &buf_size_t, NULL);
	printf("veckratnik niti = %d \n", buf_size_t);

	// "s"cepec: argumenti
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&img_mem_obj);
	ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&img_out_mem_obj);
	ret |= clSetKernelArg(kernel, 2, sizeof(cl_int), (void *)&slika.width);
	ret |= clSetKernelArg(kernel, 3, sizeof(cl_int), (void *)&slika.height);
	// "s"cepec, "stevilka argumenta, velikost podatkov, kazalec na podatke

	double start = get_wall_time();

	// "s"cepec: zagon
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
		&global_item_size, &local_item_size, 0, NULL, NULL);
	// vrsta, "s"cepec, dimenzionalnost, mora biti NULL,
	// kazalec na "stevilo vseh niti, kazalec na lokalno "stevilo niti,
	// dogodki, ki se morajo zgoditi pred klicem

	printf("kernel results %d \n", ret);

	// Kopiranje rezultatov
	ret = clEnqueueReadBuffer(command_queue, img_out_mem_obj, CL_TRUE, 0,
		pixelCount*sizeof(int), slika.image, 0, NULL, NULL);
	// branje v pomnilnik iz naparave, 0 = offset
	// zadnji trije - dogodki, ki se morajo zgoditi prej

	double gpu_time = get_wall_time() - start;
	printf("Time GPU: %.15fs\n", gpu_time);

	// Prikaz rezultatov
	writePGM("sobel.pgm", &slika);

	// CPU
	PGMData slikaCPU;
	readPGM(filename, &slika);
	slikaCPU.height=slika.height;
	slikaCPU.width=slika.width;
	slikaCPU.max_gray=slika.max_gray;
	slikaCPU.image=(int *)malloc(slikaCPU.height*slikaCPU.width*sizeof(int));

	start = get_wall_time();
	sobelCPU(&slika, &slikaCPU);

	double cpu_time = get_wall_time() - start;
	printf("Time CPU: %.15fs\n", cpu_time);

	printf("GPU is %.fx faster\n", cpu_time/gpu_time);

	printf("image size: %d x %d\n", slika.width, slika.height);

	writePGM("sobelCPU.pgm", &slikaCPU);

	// "ci"s"cenje
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(img_mem_obj);
	ret = clReleaseMemObject(img_out_mem_obj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);

	return 0;
}

// kernel

