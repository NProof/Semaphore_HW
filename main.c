#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

void *
Tchild (void *args)
{
  /* POTENTIALLY DANGEROUS TIMING */
  int *argptr = (int *) args;
  int arg = *argptr;

  /* Print the local copy of the argument */
  printf ("Argument is %d\n", arg);
  pthread_exit (NULL);
}

int main(int argc, char ** argv) {
    pthread_t child_thread[11];
    int listi[11] = {0};

    for (int i = 1; i <= 10; i++)
        listi[i] = i;

    for (int i = 1; i <= 10; i++)
        assert (pthread_create (&child_thread[i], NULL, Tchild, (void*)&listi[i]) == 0);

    for (int i = 1; i <= 10; i++)
        pthread_join (child_thread[i], NULL);

    return 0;
}