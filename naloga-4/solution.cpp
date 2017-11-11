#include <stdio.h>
#include <math.h>
#include <omp.h>

#define START 2
#define STOP 100000

int divider_sum(int n) {
    // Returns sum of dividers if sum is smaller than number, otherwise -1
    int sum = 1;
    int root = sqrt(n);

    for(int i=2; i<root; i++) {
        int quotient = (int)n / i;
        int remainder = n % i;

        if(remainder == 0) {
            // sum quotient and divider, because we only
            // divide to square root of n
            sum += quotient + i;
            if (sum >= n) {
                return -1;
            }
        }
    }

    return sum;
}

int are_amicable(int n, int f) {
    // Check if sum of dividers of n equals to f
    f = f - 1;
    int root = sqrt(n);

    for(int i=2; i<root; i++) {
        int quotient = (int)n / i;
        int remainder = n % i;

        if(remainder == 0) {
            f -= quotient + i;
            if (f < 0) {
                return false;
            }
        }
    }

    return f == 0;
}

int main() {
    int sum = 0;

    #pragma omp parallel for schedule(dynamic,10) reduction(+:sum)
    for (int i=START; i<=STOP; i++) {
        int amicable_candidate = divider_sum(i);

        if(amicable_candidate != -1) {
            if(are_amicable(amicable_candidate, i)) {
                printf("%d %d\n", i, amicable_candidate);
                sum += i + amicable_candidate;
            }
        }
    }

    printf(
        "Vsota prijateljskih števil med %d in %d je %d\n",
        START, STOP, sum
    );

    return 0;
}
