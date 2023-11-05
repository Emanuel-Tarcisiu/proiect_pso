#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

int main() {
    void* start_heap = sbrk(0);
    printf("Top of heap= %p\n",start_heap);
    
    int* a = (int*)mmap(start_heap, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_FIXED, -1, 0);
    if (a == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    
    *a=29;
    printf("Val lui a= %i\n",*a);
    start_heap=sbrk(0);
    printf("Top of heap= %p\n",start_heap);
    
    if (munmap(a, 4096) == -1) {
        perror("munmap");
        return 1;
    }

    start_heap=sbrk(0);

    int* b=(int*)mmap(start_heap, 4*sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    b[0]=1;
    b[1]=2;
    b[2]=3;
    b[3]=4;
    b[4]=5;
    //printf("Val lui b= %i\n",*b);

    start_heap=sbrk(0);
    void* end_heap=sbrk(0);

    if (munmap(b, 4096) == -1) {
        perror("munmap");
        return 1;
    }

    

    return 0;
}