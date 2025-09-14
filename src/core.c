#include "core.h"

int mod(int a, int b)
{
    int r = a % b;
    return r > 0 ? r : r + b;
}

// Generates a random number from -1 to 1, with a bias towards 0
// Not an actual normal distribution, but quick and cheap to calculate
double rand_normal()
{
    double a = (double)rand() / RAND_MAX;
    double b = (double)rand() / RAND_MAX;
    return a + b - 1;
}