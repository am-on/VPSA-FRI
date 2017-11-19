#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define LEN 67108864 // vector length

double vectorX[LEN];
double vectorY[LEN];
double vectorK[LEN];

void fillVector(double *vector, int len) {
    // Fill vector with random values.
    for (int i=0; i<len; i++) {
        vector[i] = ((double)rand()) / 100000000;
        vector[i] = i * i;
    }
}

int main() {

    // fillVector(vectorY, LEN);

    for (int i=0; i<LEN; i++) {
        vectorX[i] = i + 1;
        vectorY[i] = (i+1)*(i+1);
        if(i % 2 == 0) {
            vectorY[i] = 1;
        }
    }

    double sightHeight = 0;
    long inSight = 1;

    double start = omp_get_wtime();
    omp_set_num_threads(8);
    #pragma omp parallel reduction(+:sightHeight, inSight)
    {
        int threadWork = LEN / omp_get_num_threads();
        int Tn = omp_get_thread_num();

        int start = Tn * threadWork;
        int end = start + threadWork;

        inSight = 0;
        sightHeight = 0;

        vectorK[start] = vectorY[start] / vectorX[start];

        // get local k's
        for(int i=start+1; i<end; i++) {
            double k = vectorY[i] / vectorX[i];
            vectorK[i] = fmax(vectorK[i-1], vectorY[i] / vectorX[i]);
        }

        #pragma omp barrier

        // get max k from previous k's
        int max = -1;
        if (Tn != 0) {
            int check_start = threadWork-1;
            max = vectorK[check_start];
            check_start += threadWork;
            for(int i=check_start; i<start; i+=threadWork) {
                max = fmax(max, vectorK[i]);
                vectorK[i] = max;
            }
        } else {
            start++;
        }

        // fix values, calculate visible and height
        for(int i=start; i<end; i++) {
            if(max >= vectorK[i]) {
                vectorK[i] = max;
            }
            if(vectorK[i] > vectorK[i-1]) {
                sightHeight += vectorY[i] - (vectorK[i-1] * vectorX[i]);
                inSight++;
            }
        }
    }

    // for (int i=0; i<LEN; i++) {
    //     printf("x: %.f, y: %.f, k: %.1f\n",
    //         vectorX[i], vectorY[i], vectorK[i]);
    // }

     printf("inSight: %d, sight height: %.15f\n", inSight, sightHeight);
     printf("Time: \t %f \n", omp_get_wtime()-start);

    return 0;
}