#include <syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE sizeof(Block)

typedef struct block Block ;

typedef struct block
{
    struct block* next;
    struct block* prev;
    size_t payload;
    int free;
};

//data+4 ==payload begining
void* heap_start=NULL;
void* heap_break=NULL;
Block* start_block=NULL;
Block* last_block=NULL;
size_t total_allocated_payload=0;
int initialized=0;

void print_block(Block* b)
{
    printf("Block address: %p\n",b);
    printf("Next address:  %p\n",b->next);
    printf("Usable address: %p\n",(char*)b+BLOCK_SIZE);
    printf("Free:  %d\n",b->free);
    printf("Payload: %ld\n",b->payload);
    printf("------\n");
}

void list_blocks()
{
    Block* p=start_block;
    printf("BLOCK LIST: \n------\n");
    while(p)
    {
        print_block(p);
        p=p->next;
    }
    printf("END LIST\n\n");

}


void init()
{
    if(!initialized)
    {
        sbrk(0);
        sbrk(0);
        initialized=1;
    }
}



Block* find_block(size_t size)
{
    Block* b=start_block;

    while(b)
    {
        if(b->free==1&&b->payload>=size)
        {
            return b;
        }

        b=b->next;
    }

    return NULL;
}

Block* split_block(Block* b,size_t size)
{
    if(b->free==1 && size<b->payload)
    {
        size_t new_block_size=size+BLOCK_SIZE;

        Block *new=(Block*)((char*)b+new_block_size);
        new->payload=(size_t)((b->payload)-size);
        new->free=1;
        new->next=b->next;

        if(b->next)
        {
            b->next->prev=new;
        }


        new->prev=b;
        b->next=new;
        b->payload=size;

        return b;
    }

    return b;


}

void myfree(void *ptr)
{
    Block *b=(Block*)((char*)(ptr)-32);
    b->free=1;
}

void* extend_heap(size_t size)
{
    //init();

    total_allocated_payload+=size;
    size_t new_block_size=BLOCK_SIZE+size;

    void *ptr=sbrk(0);

    if(ptr==(void*)-1)
    {
        return NULL;
    }
    sbrk(new_block_size);
    int status=brk((void*)((char*)ptr+new_block_size));

    void* p=sbrk(0);

    printf("New break:  %p\n",p);

    return ptr;
}



void *mymalloc(size_t size)
{
    init();

    Block* b;
    void* ptr;

    if(start_block)
    {
        b=find_block(size);

        if(!b)
        {
            b=extend_heap(size);
            b->free=0;
            b->payload=size;
            b->next=NULL;
            b->prev=last_block;
            last_block->next=b;
            last_block=b;

            
        }
        else
        {
            if(b->payload>size)
            {
                split_block(b,size);
                b->free=0;
                //b->prev=last_block;
                //last_block->next=b;
               // last_block=b->next;
            }           

        }

        return (char*)(b+BLOCK_SIZE);
    }
    else
    {
        ptr=extend_heap(size);
        b=(Block*)ptr;
        b->next=NULL;
        b->prev=NULL;
        b->free=0;
        b->payload=size;

        start_block=b;
        last_block=b;

        return (char*)(b)+BLOCK_SIZE;
    }

}


int main()
{
    int* b=(int*)mymalloc(2*sizeof(int));
    int* b=(int*)mymalloc(1*sizeof(int));
    int *c=(int*)mymalloc(1*sizeof(int));
    int *d=(int*)mymalloc(1*sizeof(int));
    // printf("%p\n",sbrk(0));
    // printf("%p\n",sbrk(0));
    // printf("%p\n",sbrk(0));
    /*
    printf(" START OF HEAP = %p\n",heap_start);
    printf("BREAK = %p\n\n",heap_break);
    */

    /*
    int* a=(int*)mymalloc(1*sizeof(int));
    
    
    for(int i=0;i<3;i++)
    {
        a[i]=i;
        //printf("Value %d at address %p\n",a[i],&a[i]);

    }
  

    
    
    */
    
    //printf("BREAK = %p\n\n",sbrk(0));

    int* x=(int*)sbrk(0);

    *x=10;
    list_blocks();

    printf("\n");
   return 0;
}
