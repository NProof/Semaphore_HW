#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

struct thread_args {
    int *big_buf;
};

struct results {
    int min;
    int max;
};

void *parallel_thread(void *_args) {
    struct thread_args *args = (struct thread_args *) _args;

    struct results *rels = calloc (sizeof (struct results), 1);
    int * big_buf = args->big_buf;
    int min = big_buf[0];
    int max = big_buf[0];
    for(int i=1; i<1024; ++i){
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

    pthread_t p1;

    struct thread_args *args = malloc(sizeof (struct thread_args) );
    args->big_buf = big_buf;

    assert( pthread_create (&p1, NULL, parallel_thread, args) == 0);

    struct results *rels[1];
    
    pthread_join (p1, (void **) &rels[0]);

    minimum = rels[0]->min;
    maximum = rels[0]->max;

    free(rels[0]);

    printf("Success! maximum = %-10d and minimum = %-10d\n", maximum, minimum);
    pthread_exit (NULL);
}