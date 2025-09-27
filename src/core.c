#include "core.h"

int mod(int a, int b)
{
    int r = a % b;
    return r >= 0 ? r : r + b;
}

int max2(int a, int b)
{
    return (a > b) ? a : b;
}

int max3(int a, int b, int c)
{
    int x = max2(a, b);
    return max2(x, c);
}
