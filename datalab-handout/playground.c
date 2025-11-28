#include <stdio.h>

int main() {
    float f = 1.175494351e-38;
    int i = (int) f;
    printf("INT_MAX = %d, INT_MIN = %d\n", 0x7FFFFFFF, 0x80000000);
    printf("float = %f, int = %d\n", f, i);
    int n = -1082130432;
    printf("unsigned = %u\n", n);
    return 0;
}
