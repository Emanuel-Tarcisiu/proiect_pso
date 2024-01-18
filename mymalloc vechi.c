#include <syscall.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include<stdbool.h>
#include<string.h>

#define S_BLOCK_SIZE sizeof(Block)
#define S_PAGE_SIZE sizeof(Page)
#define BREAK_POINT sbrk(0)
#define PAGE_SIZE sysconf(_SC_PAGESIZE)

typedef struct s_block Block ;

struct s_block
{
    struct s_block* next;
    struct s_block* prev;
    size_t payload;
    int free;
}s_block;

typedef struct s_page Page;

struct s_page
{
    struct s_page* next;
    struct s_page* prev;
    struct s_block* first_page_block;
    struct s_block* last_page_block;
}s_page;

Page* START_PAGE=NULL;
Page* LAST_PAGE=NULL;

Block* start_block=NULL;
Block* last_block=NULL;

Block* free_blocks[16];
int index_free_blocks=0;

void print_block(Block* b)
{
    printf("Block address: %p\n",b);
    printf("Next address:  %p\n",b->next);
    printf("Usable address: %p\n",(char*)b+S_BLOCK_SIZE);
    printf("Free:  %d\n",b->free);
    printf("Payload: %ld\n",b->payload);
    printf("------\n");
}

void list_blocks_in_page(Page* page)
{
  printf("Page start: %p\n",page);

  Block* p=page->first_page_block;

  while(p)
  {
    print_block(p);
    p=p->next;
  }
  printf("\n");
}

void list_blocks()
{
    Block* p=start_block;
    printf("HEAP: \n------\n");
    printf("HEAP BREAK POINT: %p\n",BREAK_POINT);
    int i=1;
    while(p)
    {
        printf("%d\n",i);
        print_block(p);
        p=p->next;
        i++;
    }
    

    printf("HEAP BREAK POINT: %p\n",BREAK_POINT);
    printf("===================================\n");

    Page* page=START_PAGE;
    i=1;
    printf("PAGES\n");
    printf("------\n");

    while(page)
    {
       printf("%d\n",i);
       list_blocks_in_page(page);

       page=page->next;
       i++;
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

void try_merge_blocks(Block* a,Block* b)
{
    if(a!=NULL && b!=NULL)
    {
        if(a->free==1 && b->free==1)
        {
            a->next=b->next;
            b->next->prev=a;
            a->payload=a->payload+S_BLOCK_SIZE+b->payload;
        }
    }

}

void merge_free_blocks()
{
    Block* p=start_block;

    while(p!=NULL)
    {
        try_merge_blocks(p,p->next);
        p=p->next;
    }
}

Block* split_block(Block* b,size_t size)
{
    if(b->free==1 && size<b->payload)
    {
        size_t new_block_size=size+S_BLOCK_SIZE;

        Block* new=(Block*)((char*)b+new_block_size);
        new->payload=(size_t)((b->payload)-size);
        new->free=1;
        new->next=b->next;

        if(b->next!=NULL)
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



void remove_node_free_blocks(int index){
  for(int i=index;i<index_free_blocks-1;i++){
    free_blocks[i]=free_blocks[i+1];
  }

  index_free_blocks--;
}



Block* merge_blocks_Next(Block* curent_node){
  Block* other_node=curent_node->next;

  if(curent_node!=NULL && other_node!=NULL)
    {
        if(curent_node->free==1 && other_node->free==1)
        {
            curent_node->next=other_node->next;
            if(other_node->next != NULL)
              other_node->next->prev=curent_node;

            curent_node->payload=curent_node->payload + S_BLOCK_SIZE + other_node->payload;

            
            for(int i=0;i<index_free_blocks;i++){
              if (free_blocks[i] == other_node){
                free_blocks[i]=curent_node;

                return curent_node;
              }
            }
        
    }
    else
      return NULL;
  }
  else
    return NULL;
}



Block* merge_blocks_Prev(Block* curent_node){
  Block* other_node=curent_node->prev;

   if(curent_node!=NULL && other_node!=NULL){
    if(curent_node->free==1 && other_node->free==1)
        {
          other_node->next=curent_node->next;
          if(curent_node->next != NULL)
            curent_node->next->prev=other_node;

          other_node->payload=other_node->payload + S_BLOCK_SIZE + curent_node->payload;

          return other_node;
        }
        else
          return NULL;
   }
   else
    return NULL;

    return curent_node;
}



void print_free_B(){
  if(index_free_blocks == 0){
    printf("NU sunt blocuri libere!\n\n");
    return;
  }

  printf("Inceput print block-uri libere:\n");

  for(int i=0;i<index_free_blocks;i++){
    if(free_blocks[i]->free != 1){
      perror("NU s-a eliberat un block!\n");
      return;
    }

    printf("Block[%i] are payload= %i\n",i, (int)free_blocks[i]->payload);
  }

  printf("\n");
}


void remove_node_free_list(int index){
  for(int i=index;i<index_free_blocks-1;i++){
    free_blocks[i]=free_blocks[i+1];
  }

  index_free_blocks--;
}




void clear_free_list(){
  for(int i=0;i<index_free_blocks-1;i++){
    for(int j=0;j<index_free_blocks;j++){
      if(free_blocks[i] == free_blocks[j])
        remove_node_free_list(j);
    }
  }
}



void myfree(void *ptr)
{
    Block* b=(Block*)((char*)(ptr)-32);
    b->free=1;

    int count=0;
    Block* p1=merge_blocks_Prev(b);
    
    if(p1 != b && p1 != NULL){
      b=p1;
      count++;
    }

    Block* p2=merge_blocks_Next(b);
    if(p2 != b && p2 != NULL){
      count++;
    }

    if(count == 0){
      free_blocks[index_free_blocks++]=b;
    }
    else{
      clear_free_list();
    }
    
}



Block* find_block_in_page(Page *page,size_t size)
{
  Block *p=page->first_page_block;

  while(p)
  {
    if(p->free==1 && p->payload>=size)
    {
      return p;
    }

    p=p->next;
  }

  return NULL;
}

Block* find_block_in_pages(size_t size,Page** current_page)
{
  Page* p=START_PAGE;

  while(p)
  {
    Block* b=p->first_page_block;

    while(b)
    {
      if(b->free==1 && b->payload >= size)
      {
        (*current_page)=p;
        return b;
      }

      b=b->next;
    }

    p=p->next;
  }

  return NULL;
}

size_t get_page_payload(Page* page)
{
  Block* b=page->first_page_block;
  size_t payload=0;

  while(b)
  {
    payload+=b->payload+S_BLOCK_SIZE;

    b=b->next;
  }

  return payload;
}

Page *alloc_new_page()
{
  void *ptr = mmap(NULL,PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (ptr == MAP_FAILED) {
        perror("alloc_new_page::mmap");
        exit(EXIT_FAILURE);
    }

  Page* new_page=(Page*)(ptr);

  if(START_PAGE==NULL)
  {
    new_page->prev=NULL;
    new_page->next=NULL;
    new_page->first_page_block=NULL;
    START_PAGE=new_page;
    LAST_PAGE=new_page;
  }
  else
  {
    new_page->prev=LAST_PAGE;
    new_page->next=NULL;
    new_page->first_page_block=NULL;

    LAST_PAGE->next=new_page;
    LAST_PAGE=new_page;
  }

  return new_page;
}


Block* alloc_block_in_page(Page* page,size_t size)
{
    
    if(get_page_payload(page)+size>PAGE_SIZE)
    {
      return NULL;
    }

    if(page->first_page_block==NULL)
    {
      
      Block* new_block=(Block*)((char*)page+S_PAGE_SIZE);
      page->first_page_block=new_block;
      page->last_page_block=new_block;

      new_block->prev=NULL;
      new_block->next=NULL;
      new_block->payload=size;
      new_block->free=0;

      return new_block;
    }
    else
    {
      Block* new_block=(Block*)((char*)(page->last_page_block)+S_BLOCK_SIZE+page->last_page_block->payload);
  
      page->last_page_block->next=new_block;
      new_block->next=NULL;
      new_block->prev=page->last_page_block;

      new_block->payload=size;
      new_block->free=0;

      page->last_page_block=new_block;

      return new_block;
    }

  //return new_block;
}

Block* alloc_block_in_pages(size_t size,Page** current_page)
{
  Page* page=START_PAGE;
  Block* block=NULL;

  while(page)
  {
    block=alloc_block_in_page(page,size);
    //block->free=0;
  //  block->payload=size;
    if(block!=NULL)
    {
      (*current_page)=page;
      return block;
    }

    page=page->next;
  }

  (*current_page)=NULL;

  return NULL;
}

Block *extend_heap(size_t size)
{
  Block* ptr=(Block*)sbrk(S_BLOCK_SIZE + size);

  return ptr;
}

Block* start_allocating_pages(size_t size)
{
  if(START_PAGE==NULL)
  {
    alloc_new_page();
  }

  Page* current_page=NULL;
  Block* found_block=find_block_in_pages(size,&current_page);

  if(found_block)
  {
    return found_block;
  }
  else
  {
    Block* new_block = alloc_block_in_pages(size,&current_page);

    if(new_block==NULL)
    {
      Page* new_page=alloc_new_page();
      new_block=alloc_block_in_page(new_page,size);
      return new_block;
    }

    return new_block;
  }
  

}

void* mymalloc(size_t size)
{
  //Block ptr=(Block)sbrk(BLOCK_SIZE + size);
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
    ptr=(void*)-1;
    if(ptr==(void*)-1)
    {
      //printf("error");
      ptr=start_allocating_pages(size);
      ptr=(Block*)(ptr);
    }
    else
    {
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
    }
    return (Block*)(ptr+1);
  }
}

void *mycalloc(size_t num, size_t size)
{
  size_t payload=size*num;

  char* ptr=(char*)mymalloc(payload);
  //char ptr=(char)p;

  for(int i=0;i<payload;i++)
  {
      //((char*)(ptr+i))=0;
	  ((char*)ptr)[i]=0;
  }
}



int find_best_block(size_t size){
  int best=-1;
  int rest_b=_INT_MAX_;

  for(int i=0;i<index_free_blocks;i++){
    if(free_blocks[i]->payload == size)
      return i;
    
    if(free_blocks[i]->payload - size <= rest_b){
      best=i;
      rest_b=free_blocks[i]->payload-size;
    }
  }

  return best;
}



Block* split_free_block(int index, size_t new_size){
  if(index == -1){
    return NULL;
  }

  Block* p=free_blocks[index] + new_size;

  free_blocks[index]->next = p;
  p->prev = free_blocks[index];
  p->payload = free_blocks[index]->payload - new_size;
  free_blocks[index]->payload = new_size;
  free_blocks[index]->free = 0;

  Block* ret_value = free_blocks[index];
  free_blocks[index] = p;

  return ret_value;
}


void *realloc(void *ptr, size_t size){
  if(index_free_blocks > 0){
    Block* new_realocated=split_free_block(find_best_block(size),size);
    if(new_realocated == NULL){
      perror("Eroare la realocare\n");
      return NULL;}

    new_realocated = memcpy(new_realocated,ptr,size);
    if(new_realocated == NULL){
      perror("Eroare la copierea mem dupa realocare!\n");
      return NULL;
    }   

    return new_realocated;
  }

  return ptr;
}



int main()
{
  print_free_B();

  int* a=(int*)mymalloc(sizeof(int));
  if(a == NULL){
    perror("Eroare la alocare!\n");
    exit(EXIT_FAILURE);
  }

  *a=23;
  printf("a=%i\n",*a);

  float* b=(float*)mymalloc(sizeof(float));
  if(b == NULL){
    perror("Eroare la alocare!\n");
    exit(EXIT_FAILURE);
  }

  *b=6;
  printf("b=%f\n",*b);

  char* c=(char*)mymalloc(6*sizeof(char));
  if(c == NULL){
    perror("Eroare la alocare!\n");
    exit(EXIT_FAILURE);
  }

  c[0]='s';
  c[1]='a';
  c[2]='l';
  c[3]='u';
  c[4]='t';
  c[5]='\0';

  printf("c=%s\n\n",c);

  print_free_B();
  myfree(a);

  print_free_B();
  myfree(c);

  print_free_B();
  myfree(b);

 

  print_free_B();
  

  return 0;

}