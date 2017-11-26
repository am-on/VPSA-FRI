// "s"cepec preberemo iz datoteke
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "pgm.h"

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

void mandelbrotCPU(PGMData *I){
	float x0,y0,x,y,xtemp;
	int i,j;
	int color;
	int iter;
	int max_iteration=1000;   //stevilo iteracij

	//za vsak piksel v sliki
	for(i=0; i<I->height; i++)
		for(j=0; j<I->width; j++)
		{
			x0 = (float)j/I->width*(float)3.5-(float)2.5; //zacetna vrednost
			y0 = (float)i/I->height*(float)2.0-(float)1.0;
			x = 0;
			y = 0;
			iter = 0;
			while((x*x+y*y <= 4) && (iter < max_iteration))
			{ //ponavljamo, dokler ne izpolnemo enega izmed pogojev
				xtemp = x*x-y*y+x0;
				y = 2*x*y+y0;
				x = xtemp;
				iter++;
			}
			color = (int)(iter/(float)max_iteration*(float)I->max_gray); //pobarvamo piksel z ustrezno barvo
			I->image[i*I->width+j] = I->max_gray-color;
		}
}

int main(void)
{
	char ch;
	int i;
	cl_int ret;

	PGMData slika;
	slika.width = 1920;
	slika.height = 1080;
	slika.max_gray = 255;

	int pixelNum = slika.width * slika.height;

	slika.image = (int *)malloc(pixelNum*sizeof(int));


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
	size_t num_groups = ((pixelNum - 1) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;

	// Alokacija pomnilnika na napravi
	cl_mem img_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
		pixelNum*sizeof(int), NULL, &ret);

	// Priprava programa
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str,
		NULL, &ret);
	// kontekst, "stevilo kazalcev na kodo, kazalci na kodo,
	// stringi so NULL terminated, napaka

	// Prevajanje
	ret = clBuildProgram(program, 1, &device_id[0], NULL, NULL, NULL);
	// program, "stevilo naprav, lista naprav, opcije pri prevajanju,
	// kazalec na funkcijo, uporabni"ski argumenti

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
	cl_kernel kernel = clCreateKernel(program, "mandelbrot", &ret);
	// program, ime "s"cepca, napaka

	size_t buf_size_t;
	clGetKernelWorkGroupInfo(kernel, device_id[0], CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(buf_size_t), &buf_size_t, NULL);
	printf("veckratnik niti = %d \n", buf_size_t);


	// "s"cepec: argumenti
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&img_mem_obj);
	ret |= clSetKernelArg(kernel, 1, sizeof(cl_int), (void *)&slika.width);
	ret |= clSetKernelArg(kernel, 2, sizeof(cl_int), (void *)&slika.height);
	ret |= clSetKernelArg(kernel, 3, sizeof(cl_int), (void *)&slika.max_gray);
	// "s"cepec, "stevilka argumenta, velikost podatkov, kazalec na podatke

	double start = get_wall_time();

	// "s"cepec: zagon
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
		&global_item_size, &local_item_size, 0, NULL, NULL);
	// vrsta, "s"cepec, dimenzionalnost, mora biti NULL,
	// kazalec na "stevilo vseh niti, kazalec na lokalno "stevilo niti,
	// dogodki, ki se morajo zgoditi pred klicem

	// Kopiranje rezultatov
	ret = clEnqueueReadBuffer(command_queue, img_mem_obj, CL_TRUE, 0,
		pixelNum*sizeof(int), slika.image, 0, NULL, NULL);
	// branje v pomnilnik iz naparave, 0 = offset
	// zadnji trije - dogodki, ki se morajo zgoditi prej

	double gpu_time = get_wall_time() - start;
	printf("Time GPU: %.15fs\n", gpu_time);

	start = get_wall_time();
	mandelbrotCPU(&slika);
	double cpu_time = get_wall_time() - start;
	printf("Time CPU: %.15fs\n", cpu_time);

	printf("GPU is %.fx faster\n", cpu_time/gpu_time);

	printf("image size: %d x %d\n", slika.width, slika.height);

	// Prikaz rezultatov
	writePGM("mandelbort.pgm", &slika);

	// "ci"s"cenje
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(img_mem_obj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);

	return 0;
}

// kernel

