#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

struct thread_args {
    int *big_buf;
    int zone;
};

struct results {
    int min;
    int max;
};

void *parallel_thread(void *_args) {
    struct thread_args *args = (struct thread_args *) _args;

    struct results *rels = calloc (sizeof (struct results), 1);
    int * big_buf = args->big_buf;
    int zone = args->zone;

    int min = big_buf[256*zone];
    int max = big_buf[256*zone];
    for(int i=256*zone+1; i<256*(zone+1); ++i){
        if (big_buf[i] < min)
            min = big_buf[i];
        else if (big_buf[i] > max)
            max = big_buf[i];
    }
    rels->min = min;
    rels->max = max;
    free(args);
    pthread_exit(rels);
}

int main(int argc, char ** argv) {
    int minimum = 2147483647;
    int maximum = 0;
    
    int big_buf[1024+1] = {0};
    for(int i=0; i<1024; ++i){
        big_buf[i] = rand();
    }

    pthread_t p[4];

    for(int i=0; i<4; ++i) {
        struct thread_args *args = malloc(sizeof (struct thread_args) );
        args->big_buf = big_buf;
        args->zone = i;
        assert( pthread_create (&p[i], NULL, parallel_thread, args) == 0);
    }

    struct results *rels[4];
    
    for(int i=0; i<4; ++i)
       pthread_join (p[i], (void **) &rels[i]);

    for(int i=0; i<4; ++i) {
        if (rels[i]->min < minimum)
            minimum = rels[i]->min;
        if (rels[i]->max > maximum)
            maximum = rels[i]->max;
    }

    for(int i=0; i<4; ++i)
       free(rels[i]);

    printf("Success! maximum = %-10d and minimum = %-10d\n", maximum, minimum);
    pthread_exit (NULL);
}