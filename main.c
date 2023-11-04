// #include<stdio.h>
// #include"unistd.h"
// #include<sys/mman.h>

// int main(int argc, char* argv[]){
//     printf("Top of heap: %p\n",sbrk(0));

//     int* addr=(int*)mmap(sbrk(0), sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE, 0, 0);
//     addr=12;
//     printf("nr dupa alocare: %i\n",addr);


//     printf("Top of heap(dupa mmap): %p\n",sbrk(0));
//     munmap(addr,sizeof(int));

//     printf("Top of heap(dupa munmap): %p\n",sbrk(0));
//     return 0;
// }

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

int main(int argc, char* argv[]) {
    printf("Top of heap: %p\n", sbrk(0));

    int* a= (int*)mmap(sbrk(0), sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    //int *addr=(int*)sbrk(sizeof(int));
    *a = 12;
    printf("nr dupa alocare: %i\n", *a);
    printf("Adr a= %p\n",a);

    printf("Top of heap (după mmap): %p\n", sbrk(0));
    int err=11;
    err=munmap(a, sizeof(int));
    
    sbrk(-1*sizeof(int));
    printf("Adr a= %p\n",a);
    // int a=*addr;
    // printf("nr dupa dealocare: %i\n", a);

    printf("Top of heap (după munmap): %p\n", sbrk(0));
    return 0;
}
