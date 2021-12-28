#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

struct p_args {
    int *big_buf;
    int zone;
    int *min_buf;
    int *min_iptr;
    int *max_buf;
    int *max_iptr;
};

struct c_args {
    int i;
    int *ptr_min;
    int *ptr_max;
    int *min_buf;
    int *min_iptr;
    int *max_buf;
    int *max_iptr;
};

struct results {
    int min;
    int max;
};

void *producer(void *_args) {
    struct p_args *args = (struct p_args *) _args;
    int * big_buf = args->big_buf;
    int zone = args->zone;

    int min = big_buf[256*zone];
    int max = big_buf[256*zone];
    for(int i=256*zone+1; i<256*(zone+1); ++i){
        if (big_buf[i] < min)
            min = big_buf[i];
        if (big_buf[i] > max)
            max = big_buf[i];
    }

    sleep(1);

    int *min_buf = args->min_buf;
    int *min_iptr = args->min_iptr;
    min_buf[*min_iptr] = min;
    *min_iptr = *min_iptr + 1;

    int *max_buf = args->max_buf;
    int *max_iptr = args->max_iptr;
    max_buf[*max_iptr] = max;
    *max_iptr = *max_iptr + 1;

    free(args);
    pthread_exit(NULL);
}

void *consumer(void *_args) {
    struct c_args *args = (struct c_args *) _args;
    int i = args->i;
    int *minimum = args->ptr_min;
    int *maximum = args->ptr_max;
    int *min_buf = args->min_buf;
    int *min_iptr = args->min_iptr;
    int *max_buf = args->max_buf;
    int *max_iptr = args->max_iptr;

    sleep(2);

    printf("%d\n", i);
    switch(i) {
        case 0:
            for(int j=0; j<4; ++j) {
                *min_iptr = *min_iptr - 1;
                // if (min_buf[*min_iptr] < *minimum)
                    // *minimum = min_buf[*min_iptr];
                printf("%d[%d] -> %d\n", i, *min_iptr, min_buf[*min_iptr]);
            }
            break;
        case 1:
            for(int j=0; j<4; ++j) {
                *max_iptr = *max_iptr - 1;
                // if (max_buf[*max_iptr] > *maximum)
                //     *maximum = max_buf[*max_iptr];
                printf("%d[%d] -> %d\n", i, *max_iptr, max_buf[*max_iptr]);
            }
            break;
    }

    free(args);
    pthread_exit(NULL);
}

int main(int argc, char ** argv) {
    int minimum = 2147483647;
    int maximum = 0;
    
    int big_buf[1024+1] = {0};
    for(int i=0; i<1024; ++i){
        big_buf[i] = rand();
    }

    pthread_t p[4];
    pthread_t c[2];

    int min_buf[4];
    int min_index = 0;
    int max_buf[4];
    int max_index = 0;

    for(int i=0; i<4; ++i) {
        struct p_args *args = malloc(sizeof (struct p_args) );
        args->big_buf = big_buf;
        args->zone = i;
        args->min_buf = min_buf;
        args->max_iptr = &max_index;
        args->max_buf = max_buf;
        args->min_iptr = &min_index;
        assert( pthread_create (&p[i], NULL, producer, args) == 0);
    }
    
    for(int i=0; i<2; ++i) {
        struct c_args *args = malloc(sizeof (struct c_args) );
        args->i = i;
        args->min_iptr = &minimum;
        args->max_iptr = &maximum;
        args->min_buf = min_buf;
        args->max_iptr = &max_index;
        args->max_buf = max_buf;
        args->min_iptr = &min_index;
        assert( pthread_create (&p[i], NULL, consumer, args) == 0);
    }
    
    for(int i=0; i<4; ++i)
       pthread_join (p[i], NULL);

    for(int i=0; i<2; ++i)
       pthread_join (c[i], NULL);

    printf("Success! maximum = %-10d and minimum = %-10d\n", maximum, minimum);

    int min = big_buf[0];
    int max = big_buf[0];
    for(int i=1; i<1024; ++i){
        if (big_buf[i] < min)
            min = big_buf[i];
        if (big_buf[i] > max)
            max = big_buf[i];
    }
    printf("(CHECK)! maximum = %-10d and minimum = %-10d\n", max, min);
    pthread_exit (NULL);
}