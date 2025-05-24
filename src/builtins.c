#include <stdio.h>

double print(double x) {
    printf("%f\n", x);

    return x;
}

double prints(char* ptr) {
    return (double) puts(ptr);
}

double max(double a, double b) {
    if (a > b) {
        return a;
    }
    return b;
}

double min(double a, double b) {
    if (a < b) {
        return a;
    }
    else {
        return b;
    }
}
