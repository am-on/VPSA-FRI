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

#define M 67108864 // vector length
#define N 2 // number of threads

double vectorA[M];
double vectorB[M];
double result;


struct barrier_data {
    int counter;
    int max;
    bool first;
    bool round_id;
    bool condition;
    pthread_mutex_t first_lock;
    pthread_mutex_t counter_lock;
};

struct barrier_data b_data;

void my_barrier(int id) {
    printf("in barrier %d\n", id);
    // first thread in barrier is assigned to controll barrier
    bool first = false;

    if (b_data.first) {
        pthread_mutex_lock(&b_data.first_lock);
        // if thread is first then reset condition
        // and flip round_id
        if (b_data.first) {
            first = true;
            b_data.first = false;
            b_data.condition = false;
            b_data.round_id = !b_data.round_id;
        }
        pthread_mutex_unlock(&b_data.first_lock);
    }

    // remember b_data.round_id,
    // so thread can exit barrier even if
    // it missed b_data.condition
    bool old_round_id = b_data.round_id;

    pthread_mutex_lock(&b_data.counter_lock);
    b_data.counter += 1;
    pthread_mutex_unlock(&b_data.counter_lock);

    if (first) {
        printf("first %d\n", id);
        while(b_data.counter < b_data.max) {
            // wait until every thread is in barrier
            continue;
        }
        pthread_mutex_lock(&b_data.first_lock);
        // reset barrier data
        b_data.counter = 0;
        b_data.first = true;

        // other threads can now continue out of barrier
        b_data.condition = true;
        pthread_mutex_unlock(&b_data.first_lock);
    } else {
        while(!b_data.condition && b_data.round_id == old_round_id) {
            // wait until condition is set to true, or
            // until an thread starts next barrier round.
            continue;
        }
    }
    printf("out %d\n", id);
}


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

    //my_barrier(data->id);
    for (int i=activeThreads; i>0; i=i/2) {
        my_barrier(data->id);
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
        vector[i] = 1;
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

    // init barier
    b_data.counter = 0;
    b_data.max = N;
    b_data.round_id = true;
    b_data.first = true;
    b_data.condition = false;
    b_data.first_lock = PTHREAD_MUTEX_INITIALIZER;
    b_data.counter_lock = PTHREAD_MUTEX_INITIALIZER;

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

    printf("\n");

    printf(
        "serial  = %.f\nthreads = %.f\ndiff    = %.f \n",
        serialScalar,
        vectorA[0],
        serialScalar - vectorA[0]
    );

    printf("\n");

    printf("serial time: %.15fs \n", wall1 - wall0);
    printf("thread time: %.15fs \n", wallT1 - wallT0);
    printf("----------------------------\n");
    printf("threads are %.15fs faster\n", (wall1 - wall0) - (wallT1 - wallT0));

    pthread_mutex_destroy(&b_data.first_lock);
    pthread_mutex_destroy(&b_data.counter_lock);

    return 0;
}