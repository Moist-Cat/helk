#include <stdio.h>

double print(double x) {
    printf("%f\n", x);

    return x;
}

double prints(char* ptr) {
    return (double) puts(ptr);
}
