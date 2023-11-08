#include <syscall.h>
#include <sys/types.h>
#include <unistd.h>
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
}s_block;

Block* start_block=NULL;
Block* last_block=NULL;

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
    int i=1;
    while(p)
    {
        printf("%d\n",i);
        print_block(p);
        p=p->next;
        i++;
    }
    printf("BREAK POINT: %p\n\n",sbrk(0));

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

Block *extend_heap(size_t size)
{
  Block *ptr=(Block*)sbrk(BLOCK_SIZE + size);

  if(ptr==(void*)-1)
  {
    printf("error");
  }

  return ptr;
}


void* mymalloc(size_t size)
{
  //Block *ptr=(Block*)sbrk(BLOCK_SIZE + size);
  //Block* ptr=extend_heap(size);

  Block *ptr=find_block(size);

  if(ptr)
  {
    ptr->free=0;
      if(ptr->payload>size)
      {
        split_block(ptr,size);
        
        if(start_block==NULL)
        {
          ptr->prev=NULL;
          ptr->free=0;
          start_block=ptr;

          if(ptr->next)
          {
            last_block=ptr->next;
          }
          else
          {
            ptr->next=NULL;
            last_block=ptr;
          }
        }
        else
        {
          ptr->prev=last_block;
          last_block->next=ptr;
          last_block=ptr;

        }
      }

      return (Block*)(ptr+1);
  }
  else
  {
    ptr=extend_heap(size);
    if(ptr==(void*)-1)
    {
      printf("error");
    }

    if(start_block==NULL)
    {
      ptr->next=NULL;
      ptr->prev=NULL;
      ptr->payload=size;
      ptr->free=0;

      start_block=ptr;
      last_block=ptr;

    }
    else
    {

        last_block->next=ptr;
          ptr->next=NULL;
          ptr->prev=last_block;;
          ptr->payload=size;
          ptr->free=0;
          
          last_block=ptr;
    }
    
    return (Block*)(ptr+1);
  }
}

void *mycalloc(size_t num, size_t size)
{
  size_t payload=size*num;

  char *ptr=(char*)mymalloc(payload);
  //char *ptr=(char*)p;

  for(int i=0;i<payload;i++)
  {
      *(char*)(ptr+i)=0;
  }
}

int main()
{
  /*
  printf("%p\n",sbrk(0));
  printf("%p\n",sbrk(0));
  printf("%p\n",sbrk(256));
  printf("%p\n",sbrk(0));
  */

     int* a=(int*)mymalloc(2*sizeof(int));
    int* b=(int*)mymalloc(1*sizeof(int));
    int *c=(int*)mymalloc(1*sizeof(int));
    int *d=(int*)mymalloc(1*sizeof(int));

    list_blocks();

   printf("\n");
   return 0;

}
