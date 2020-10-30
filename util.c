#include <stdio.h>
#include <stdlib.h>

void* Malloc (size_t size) {
    void * tmp;
    if ( ( tmp = malloc(size) ) == NULL) {
        perror("Malloc");
        exit(EXIT_FAILURE); 
    }
    else
      return tmp;
}