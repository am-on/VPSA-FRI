#define _CRT_SECURE_NO_WARNINGS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <math.h>
#include "pgm.h"

#define SIZE			(1024)
#define WORKGROUP_SIZE	(1024)
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

PGMData* sobel(
			cl_device_id *device_id,
			cl_context *context,
			cl_command_queue *command_queue,
			cl_program *program,
			PGMData *image,
			PGMData *sobel
) {
	cl_int ret;

	sobel->height = image->height;
	sobel->width = image->width;
	sobel->max_gray = image->max_gray;
	sobel->image = (int *)malloc(image->height*image->width*sizeof(int));

	int pixelCount = image->width * image->height;

	// Delitev dela
	size_t local_item_size = WORKGROUP_SIZE;
	size_t num_groups = ((pixelCount - 1) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;

	// Alokacija pomnilnika na napravi
	cl_mem img_mem_obj = clCreateBuffer(*context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		pixelCount*sizeof(int), image->image, &ret);
	cl_mem img_out_mem_obj = clCreateBuffer(*context, CL_MEM_WRITE_ONLY,
		pixelCount*sizeof(int), NULL, &ret);


	// "s"cepec: priprava objekta
	cl_kernel kernel = clCreateKernel(*program, "sobel_filter", &ret);
	// program, ime "s"cepca, napaka
	if (ret != 0)
		printf("kernel prepare %d \n", ret);

	size_t buf_size_t;
	clGetKernelWorkGroupInfo(kernel, device_id[0], CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(buf_size_t), &buf_size_t, NULL);

	// "s"cepec: argumenti
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&img_mem_obj);
	ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&img_out_mem_obj);
	ret |= clSetKernelArg(kernel, 2, sizeof(cl_int), (void *)&image->width);
	ret |= clSetKernelArg(kernel, 3, sizeof(cl_int), (void *)&image->height);
	// "s"cepec, "stevilka argumenta, velikost podatkov, kazalec na podatke

	// "s"cepec: zagon
	ret = clEnqueueNDRangeKernel(*command_queue, kernel, 1, NULL,
		&global_item_size, &local_item_size, 0, NULL, NULL);
	// vrsta, "s"cepec, dimenzionalnost, mora biti NULL,
	// kazalec na "stevilo vseh niti, kazalec na lokalno "stevilo niti,
	// dogodki, ki se morajo zgoditi pred klicem

	if (ret != 0)
		printf("kernel results %d \n", ret);

	// Kopiranje rezultatov
	ret = clEnqueueReadBuffer(*command_queue, img_out_mem_obj, CL_TRUE, 0,
		pixelCount*sizeof(int), sobel->image, 0, NULL, NULL);
	// branje v pomnilnik iz naparave, 0 = offset
	// zadnji trije - dogodki, ki se morajo zgoditi prej


	ret = clReleaseKernel(kernel);
	ret = clReleaseMemObject(img_mem_obj);
	ret = clReleaseMemObject(img_out_mem_obj);

	return sobel;
}

void findSeam(
			cl_device_id *device_id,
			cl_context *context,
			cl_command_queue *command_queue,
			cl_program *program,
			PGMData *data,
			int *seam
) {
	cl_int ret;
	PGMData slika = *data;

	int pixelCount = slika.width * slika.height;

	// Delitev dela
	size_t local_item_size = WORKGROUP_SIZE;
	size_t num_groups = 1; //((pixelCount - 1) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;

	// Alokacija pomnilnika na napravi
	cl_mem img_mem_obj = clCreateBuffer(*context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		pixelCount*sizeof(int), slika.image, &ret);
	cl_mem seam_mem_obj = clCreateBuffer(*context, CL_MEM_WRITE_ONLY,
		slika.height*sizeof(int), NULL, &ret);
	cl_mem img_energy_mem_obj = clCreateBuffer(*context, CL_MEM_READ_WRITE,
		pixelCount*sizeof(int), NULL, &ret);
	cl_mem sort_mem_obj = clCreateBuffer(*context, CL_MEM_READ_WRITE,
		slika.width*sizeof(int), NULL, &ret);


	// "s"cepec: priprava objekta
	cl_kernel kernel = clCreateKernel(*program, "findSeam", &ret);
	// program, ime "s"cepca, napaka
	if (ret != 0)
		printf("kernel prepare %d \n", ret);

	size_t buf_size_t;
	clGetKernelWorkGroupInfo(kernel, device_id[0], CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(buf_size_t), &buf_size_t, NULL);

	// "s"cepec: argumenti
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&img_mem_obj);
	ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&seam_mem_obj);
	ret |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&img_energy_mem_obj);
	ret |= clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&sort_mem_obj);
	ret |= clSetKernelArg(kernel, 4, sizeof(cl_int), (void *)&slika.width);
	ret |= clSetKernelArg(kernel, 5, sizeof(cl_int), (void *)&slika.height);
	// "s"cepec, "stevilka argumenta, velikost podatkov, kazalec na podatke
	if (ret != 0)
		printf("kernel args %d \n", ret);

	// "s"cepec: zagon
	ret = clEnqueueNDRangeKernel(*command_queue, kernel, 1, NULL,
		&global_item_size, &local_item_size, 0, NULL, NULL);
	// vrsta, "s"cepec, dimenzionalnost, mora biti NULL,
	// kazalec na "stevilo vseh niti, kazalec na lokalno "stevilo niti,
	// dogodki, ki se morajo zgoditi pred klicem

	if (ret != 0)
		printf("kernel results %d \n", ret);

	// Kopiranje rezultatov
	ret = clEnqueueReadBuffer(*command_queue, seam_mem_obj, CL_TRUE, 0,
		slika.height*sizeof(int), seam, 0, NULL, NULL);
	// branje v pomnilnik iz naparave, 0 = offset
	// zadnji trije - dogodki, ki se morajo zgoditi prej

	ret = clReleaseKernel(kernel);
	ret = clReleaseMemObject(img_mem_obj);
	ret = clReleaseMemObject(seam_mem_obj);
	ret = clReleaseMemObject(img_energy_mem_obj);
	ret = clReleaseMemObject(sort_mem_obj);
}

PGMData* removeSeam(
			cl_device_id *device_id,
			cl_context *context,
			cl_command_queue *command_queue,
			cl_program *program,
			PGMData *image,
			PGMData *small_image,
			int *seam
) {
	cl_int ret;

	small_image->height = image->height;
	small_image->width = image->width - 1;
	small_image->max_gray = image->max_gray;
	small_image->image = (int *)malloc(small_image->height*small_image->width*sizeof(int));

	int pixelCount = image->width * image->height;
	int pixelCountSmall = small_image->width * small_image->height;

	// Delitev dela
	size_t local_item_size = WORKGROUP_SIZE;
	size_t num_groups = ((pixelCountSmall - 1) / local_item_size + 1);
	size_t global_item_size = num_groups*local_item_size;

	// Alokacija pomnilnika na napravi
	cl_mem img_mem_obj = clCreateBuffer(*context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		pixelCount*sizeof(int), image->image, &ret);
	cl_mem seam_mem_obj = clCreateBuffer(*context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		small_image->height*sizeof(int), seam, &ret);
	cl_mem img_out_mem_obj = clCreateBuffer(*context, CL_MEM_WRITE_ONLY,
		pixelCountSmall*sizeof(int), NULL, &ret);

	// "s"cepec: priprava objekta
	cl_kernel kernel = clCreateKernel(*program, "removeSeam", &ret);
	// program, ime "s"cepca, napaka
	if (ret != 0)
		printf("kernel prepare %d \n", ret);

	size_t buf_size_t;
	clGetKernelWorkGroupInfo(kernel, device_id[0], CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(buf_size_t), &buf_size_t, NULL);

	// "s"cepec: argumenti
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&img_mem_obj);
	ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&img_out_mem_obj);
	ret |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&seam_mem_obj);
	ret |= clSetKernelArg(kernel, 3, sizeof(cl_int), (void *)&small_image->width);
	ret |= clSetKernelArg(kernel, 4, sizeof(cl_int), (void *)&small_image->height);
	// "s"cepec, "stevilka argumenta, velikost podatkov, kazalec na podatke

	if (ret != 0)
		printf("kernel args %d \n", ret);

	// "s"cepec: zagon
	ret = clEnqueueNDRangeKernel(*command_queue, kernel, 1, NULL,
		&global_item_size, &local_item_size, 0, NULL, NULL);
	// vrsta, "s"cepec, dimenzionalnost, mora biti NULL,
	// kazalec na "stevilo vseh niti, kazalec na lokalno "stevilo niti,
	// dogodki, ki se morajo zgoditi pred klicem

	if (ret != 0)
		printf("kernel results %d \n", ret);

	// Kopiranje rezultatov
	ret = clEnqueueReadBuffer(*command_queue, img_out_mem_obj, CL_TRUE, 0,
		pixelCountSmall*sizeof(int), small_image->image, 0, NULL, NULL);
	// branje v pomnilnik iz naparave, 0 = offset
	// zadnji trije - dogodki, ki se morajo zgoditi prej

	ret = clReleaseKernel(kernel);
	ret = clReleaseMemObject(img_mem_obj);
	ret = clReleaseMemObject(img_out_mem_obj);

	return small_image;
}

int main(void) {
	char ch;
	int i;
	cl_int ret;

	// Branje datoteke
	FILE *fp;
	char *source_str;
	size_t source_size;

	fp = fopen("kernel.cl", "r");
	if (!fp) {
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

	// Priprava programa
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str,
		NULL, &ret);
	// kontekst, "stevilo kazalcev na kodo, kazalci na kodo,
	// stringi so NULL terminated, napaka

	// Prevajanje
	ret = clBuildProgram(program, 1, &device_id[0], NULL, NULL, NULL);
	// program, "stevilo naprav, lista naprav, opcije pri prevajanju,
	// kazalec na funkcijo, uporabni"ski argumenti
	if (ret != 0)
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

	if (ret != 0)
		printf("%s\n", build_log);
	free(build_log);

	// read image
	const char filename[] = "images/tower.pgm";
	PGMData image;
	PGMData small_image;
	readPGM(filename, &image);

	double start = get_wall_time();

	for(int i=0; i<400; i++){
		// sobel filter
		PGMData sobel_img;
		sobel(device_id, &context, &command_queue, &program, &image, &sobel_img);

		// find seam
		int *seam= (int *)malloc(sobel_img.height*sizeof(int));
		findSeam(device_id, &context, &command_queue, &program, &sobel_img, seam);

		free(sobel_img.image);

		// remove seam
		removeSeam(device_id, &context, &command_queue, &program, &image, &small_image, seam);
		free(seam);
		free(image.image);

		image.image = small_image.image;
		image.width = small_image.width;
	}

	double gpu_time = get_wall_time() - start;
	printf("Time GPU: %.15fs\n", gpu_time);

	writePGM("mini.pgm", &small_image);

	// "ci"s"cenje
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseProgram(program);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);

	return 0;
}
