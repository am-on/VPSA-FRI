#include <stdio.h>
#include <time.h>
#include <stdlib.h>


double * Random(int n) {
    srand(time(NULL));
    double *randomList = (double *)malloc(n * sizeof(double));

    for(int i=0; i<n; i++) {
        randomList[i] = (double)rand() / RAND_MAX;
    }

    return randomList;
}


double ** Matrix(double *A, int n, int r) {
    int cols = n / r;

    if(n % r != 0) {
        cols++;
    }

    double **matrix = malloc(r * sizeof(double *));

    for(int i=0; i<r; i++) {
        matrix[i] = calloc(cols, sizeof(double));

        for(int j=0; j<cols && (i*j)<n; j++) {
            matrix[i][j] = A[i*cols + j];
        }
    }

    return matrix;
}


double * Max(double *A, int n) {
    double *max = A;

    for(int i=1; i<n; i++) {
        if(A[i]>*max) {
            max = &A[i];
        }
    }

    return max;
}


int main() {
    int n, r;
    clock_t start, stop;
    double time;

    printf("Vnesi n: ");
    scanf("%d", &n);

    printf("Vnesi r: ");
    scanf("%d", &r);

    printf("\n");

    start = clock();
    double *rndList = Random(n);
    stop = clock();

    printf("Izhod 1D: \n");
    for(int i=0; i<n; i++) {
        printf("%.2f ", rndList[i]);
    }

    printf("\n");

    double **matrix = Matrix(rndList, n, r);

    int cols = n / r;
    if(n % r != 0) {
        cols++;
    }

    printf("Izhod 2D: \n");
    for(int i=0; i<r; i++) {
        for(int j=0; j<cols;j++) {
            printf("%.2f ", matrix[i][j]);
        }
        printf("\n");
    }

    printf("\n");

    double *maxValue = Max(rndList, n);

    printf(
        "Najvecja vrednost: %.2f na naslovu: %0X\n",
        *maxValue,
        maxValue
    );

    time = (double)(stop - start) / CLOCKS_PER_SEC;
    printf("Cas izvajanja: %f", time);

    printf("\n");

    for(int i=0; i<r; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(rndList);
}
