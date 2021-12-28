#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
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

sem_t min_mutex;
sem_t min_full;
// sem_t min_empty;
sem_t max_mutex;
sem_t max_full;
// sem_t max_empty;

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

    // sem_wait(&min_empty);
    sem_wait(&min_mutex);
    int *min_buf = args->min_buf;
    int *min_iptr = args->min_iptr;
    int index = *min_iptr;
    min_buf[index] = min;
    printf("PUT %10d in min_buf[%d]\n", min_buf[index], index);
    *min_iptr = index + 1;
    sem_post(&min_mutex);
    sem_post(&min_full);

    // sem_wait(&max_empty);
    sem_wait(&max_mutex);
    int *max_buf = args->max_buf;
    int *max_iptr = args->max_iptr;
    index = *max_iptr;
    max_buf[index] = max;
    printf("PUT %10d in max_buf[%d]\n", max_buf[index], index);
    *max_iptr = index + 1;
    sem_post(&max_mutex);
    sem_post(&max_full);

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

    sleep(1);

    switch(i) {
        case 0:
            for(int j=0; j<4; ++j) {
                sem_wait(&min_full);
                sem_wait(&min_mutex);
                int index = *min_iptr - 1;
                printf("MIN PICK [%d] -> %d\n", index, min_buf[index]);
                if (min_buf[index] < *minimum) {
                    *minimum = min_buf[index];
                    printf("MIN Update %d\n", *minimum);
                }
                *min_iptr = index;
                sem_post(&min_mutex);
                // sem_post(&min_empty);
            }
            break;
        case 1:
            for(int j=0; j<4; ++j) {
                sem_wait(&max_full);
                sem_wait(&max_mutex);
                int index = *max_iptr - 1;
                printf("MAX PICK [%d] -> %-10d\n", index, max_buf[index]);
                if (max_buf[index] > *maximum) {
                    *maximum = max_buf[index];
                    printf("MAX Update %d\n", *maximum);
                }
                *max_iptr = index;
                sem_post(&max_mutex);
                // sem_post(&max_empty);
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

    sem_init(&min_mutex, 1 ,1);
    sem_init(&min_full, 1 ,0);
    // sem_init(&min_empty, 0 ,4);
    sem_init(&max_mutex, 1 ,1);
    sem_init(&max_full, 1 ,0);
    // sem_init(&max_empty, 0 ,4);

    for(int i=0; i<4; ++i) {
        struct p_args *pargs = malloc(sizeof (struct p_args) );
        pargs->big_buf = big_buf;
        pargs->zone = i;
        pargs->min_buf = min_buf;
        pargs->min_iptr = &min_index;
        pargs->max_buf = max_buf;
        pargs->max_iptr = &max_index;
        assert( pthread_create (&p[i], NULL, producer, pargs) == 0);
    }
    
    // for(int i=0; i<2; ++i) {
    //     struct c_args *cargs = malloc(sizeof (struct c_args) );
    //     cargs->i = i;
    //     cargs->ptr_min = &minimum;
    //     cargs->ptr_max = &maximum;
    //     cargs->min_buf = min_buf;
    //     cargs->min_iptr = &min_index;
    //     cargs->max_buf = max_buf;
    //     cargs->max_iptr = &max_index;
    //     assert( pthread_create (&c[i], NULL, consumer, cargs) == 0);
    // }
    
    for(int i=0; i<4; ++i)
       pthread_join (p[i], NULL);

    // for(int i=0; i<2; ++i)
    //    pthread_join (c[i], NULL);

    printf("Success! maximum = %-10d and minimum = %-10d\n", maximum, minimum);
    sem_destroy(&min_mutex);
    sem_destroy(&min_full);
    // sem_destroy(&min_empty);
    sem_destroy(&max_mutex);
    sem_destroy(&max_full);
    // sem_destroy(&max_empty);

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