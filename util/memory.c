#include "memory.h"

#include <stdlib.h>
#include <errno.h>

#include <stdlib.h>
#include <errno.h>

void* sc_allocarray(size_t nmemb, size_t size) {
    if (nmemb > 0 && SIZE_MAX / nmemb < size) {
        errno = ENOMEM;
        return NULL;
    }

    // Allocate memory for the array
    return malloc(nmemb * size);
}
