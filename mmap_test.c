#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

int main() {
    void* start_heap = sbrk(0);
    printf("Top of heap= %p\n",start_heap);
    // Alocăm memorie folosind mmap în apropierea vârfului heap-ului
    int* a = (int*)mmap(start_heap, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (a == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Utilizăm memoria alocată cu mmap
    *a=29;
    printf("Val lui a= %i\n",*a);
    start_heap=sbrk(0);
    printf("Top of heap= %p\n",start_heap);
    // Eliberăm memoria utilizând munmap
    if (munmap(a, 4096) == -1) {
        perror("munmap");
        return 1;
    }

    start_heap=sbrk(0);

    int* b=(int*)mmap(start_heap, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    *b=28;
    //printf("Val lui b= %i\n",*b);

    if (munmap(b, 4096) == -1) {
        perror("munmap");
        return 1;
    }

    start_heap=sbrk(0);

    return 0;
}