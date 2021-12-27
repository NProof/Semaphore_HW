#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv) {
    int minimum = 2147483647;
    int maximum = 0;
    int big_buf[1024+1] = {0};
    for(int i=0; i<1024; ++i){
        big_buf[i] = rand();
        // printf("%d, ", big_buf[i]);
    }
    minimum = big_buf[0];
    maximum = big_buf[0];
    for(int i=1; i<1024; ++i){
        if (big_buf[i] < minimum)
            minimum = big_buf[i];
        else if (big_buf[i] > maximum)
            maximum = big_buf[i];
    }
    printf("Success! maximum = %-10d and minimum = %-10d\n", maximum, minimum);
    return 0;
}