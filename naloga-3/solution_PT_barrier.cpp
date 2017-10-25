#include<stdio.h>
#include <stdlib.h>
#include<pthread.h>

// time
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

#define M 8388608 // vector length
#define N 4 // number of threads

double vectorA[M];
double vectorB[M];
double result;

pthread_barrier_t barrier;

struct threadArgs {
    int id;
};

// threads and their args
pthread_t thread[N];
struct threadArgs tArgs[N];

double scalarStrip(double *a, double *b, int start, int end) {
    double sum = 0;

    for(int i=start; i<end; i++) {
        sum += a[i] * b[i];
    }

    return sum;
}

void *scalar_thread (void* arg) {
    struct threadArgs *data;
    data = (struct threadArgs *) arg;

    int len = M/N;
    int start = len * data->id;
    int end = len + start;
    double sum = scalarStrip(vectorA, vectorB, start, end);
    vectorA[start] = sum;

    int workingHalf = M / 2;
    int activeThreads = N / 2;

    for (int i=activeThreads; i>0; i=i/2) {
        pthread_barrier_wait(&barrier);
        if(data->id < i) {
            vectorA[start] = vectorA[start] + vectorA[start + workingHalf];
            workingHalf = workingHalf / 2;
        }
    }
}

void fillVector(double *vector, int len) {
    // Fill vector with random values.
    for (int i=0; i<len; i++) {
        vector[i] = ((double)rand()) / 100000000;
        //vector[i] = 1;
        // printf("%.5f ", vector[i]);
    }
    // printf("\n");
}


int main(){
    srand(time(NULL));

    // fill vectors
    fillVector(vectorA, M);
    fillVector(vectorB, M);

    double wall0 = get_wall_time();
    double serialScalar = scalarStrip(vectorA, vectorB, 0, M);
    double wall1 = get_wall_time();

    printf("serial time: %.15f \n", wall1 - wall0);


    // init barier
    pthread_barrier_init(&barrier, NULL, N);

    double wallT0 = get_wall_time();
    // init args and threads
    for(int i=0; i<N; i++) {
        tArgs[i].id = i;
        pthread_create(&thread[i], NULL, scalar_thread, (void *) &tArgs[i]);
    }

    // join threads
    for(int i=0; i<N; i++) {
        pthread_join(thread[i], NULL);
    }
    double wallT1 = get_wall_time();

    printf("thread time: %.15f \n", wallT1 - wallT0);

    printf(
        "serial = %.f, threads = %.f, diff = %.f \n",
        serialScalar,
        vectorA[0],
        serialScalar - vectorA[0]
    );

    return 0;
}