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

    int free;
	  struct s_block* free_next;
    struct s_block* free_prev;
    
	  size_t payload;
};

Block* first_free_block = NULL;
Block* last_free_block = NULL;

typedef struct s_page Page;

struct s_page
{
    struct s_page* next;
    struct s_page* prev;
    struct s_block* first_page_block;
    struct s_block* last_page_block;
    size_t capacity;
};

Page* START_PAGE=NULL;
Page* LAST_PAGE=NULL;

Block* start_block=NULL;
Block* last_block=NULL;



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
    Block* b=first_free_block;

    while(b)
    {
        if(b->free==1&&b->payload>=size)
        {
            return b;
        }

        b=b->free_next;
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
    if(b->free == 1 && size < b->payload)
    {
        size_t new_block_size=size+S_BLOCK_SIZE;

        Block* new=(Block*)((char*)b+new_block_size);
        new->payload=(size_t)(b->payload - size);
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



Block* merge_blocks_Next(Block* curent_node){
  if(curent_node->next == NULL || curent_node->next->free == 0)
    return NULL;
    
  Block* next_node = curent_node->next;

//update lista free
  curent_node->free_next = next_node->free_next;
  if(next_node->free_next != NULL)
    next_node->free_next->free_prev = curent_node;
    
  if(first_free_block == next_node)
    first_free_block = curent_node;

  if(last_free_block == next_node)
    last_free_block = curent_node;

//update lista blockuri
  curent_node->payload += next_node->payload + S_BLOCK_SIZE;
  if(next_node->next != NULL)
    next_node->next->prev = curent_node;
  curent_node->next = next_node->next;


  return curent_node;
}



Block* merge_blocks_Prev(Block* curent_node){
  if(curent_node->prev == NULL || curent_node->prev->free == 0)
    return NULL;

  Block* prev_node = curent_node->prev;

//update lista free
  prev_node->free_next = curent_node->free_next;
  if(curent_node->free_next != NULL)
    curent_node->free_next->free_prev = prev_node;

//update lista blocuri
  prev_node->payload += curent_node->payload + S_BLOCK_SIZE;
  prev_node->next = curent_node->next;
  if(curent_node->next != NULL)
    curent_node->next->prev = prev_node;

  return prev_node;
}



void print_free_B(){
  if(first_free_block == NULL){
    printf("NU sunt blocuri libere!\n\n");
    return;
  }

  printf("Inceput print block-uri libere:\n");

  for(Block* i=first_free_block;i != NULL;i = i->free_next){
    if(i->free != 1){
      perror("NU s-a eliberat un block!\n");
      return;
    }

    printf("Block-ul la %p are payload= %i\n",i, (int)i->payload);
  }

  printf("\n");
}


void remove_node_freeList(Block* current_bloc){
  if(current_bloc == first_free_block)
    first_free_block = current_bloc->free_next;
  

  if(current_bloc == last_free_block)
    last_free_block = current_bloc->free_prev;
  
  
  current_bloc->free_prev->free_next = current_bloc->free_next;
  current_bloc->free_next->free_prev = current_bloc->free_prev;

  current_bloc->free_next = NULL;
  current_bloc->free_prev = NULL;
  current_bloc->free = 0;
}




void clear_free_list(){
  for(Block* i=first_free_block;i != NULL;i=i->free_next){
    for(Block* j=i->free_next;j != NULL;j=j->free_next){
      if(i == j)
        remove_node_freeList(j);
    }
  }
}



void myfree(void *ptr)
{
    Block* b=(Block*)((char*)(ptr)-S_BLOCK_SIZE);
    b->free=1;

    if(last_free_block == NULL){
      first_free_block = b;
      last_free_block = b;
    }
    else{
      int updates=0;
       Block* p=merge_blocks_Prev(b);
      if(p != b && p != NULL){
        updates += 1;
        b=p;}

      p = merge_blocks_Next(b);
      if(p != NULL){
        updates += 2;
        b=p;}

      if(updates == 0){
        last_free_block->free_next = b;
        last_free_block = b;
      }
    }

    clear_free_list();
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

Page *alloc_new_page(size_t size)
{
  int multiply=size/PAGE_SIZE+1;

  void *ptr = mmap(NULL,multiply * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (ptr == MAP_FAILED) {
        //perror("alloc_new_page::mmap");
       // exit(EXIT_FAILURE);

       return NULL;
    }

  Page* new_page=(Page*)(ptr);
  new_page->capacity=multiply*PAGE_SIZE;

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
    
    if(get_page_payload(page)+size>page->capacity)
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
    alloc_new_page(size);
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
      Page* new_page=alloc_new_page(size);
      new_block=alloc_block_in_page(new_page,size);
      return new_block;
    }

    return new_block;
  }
  

}

void* mymalloc(size_t size)
{
  Block *ptr=find_block(size);

  if(ptr)
  {
      if(ptr->payload>size)
      {
        split_block(ptr,size);
        ptr->free=0;
        
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

		ptr->free_next = NULL;
		ptr->free_prev = NULL;

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
			
			ptr->free_next = NULL;
			ptr->free_prev = NULL;
            
            last_block=ptr;
      }
    }
    return (Block*)(ptr+1);
  }
}

void *mycalloc(size_t num, size_t size)
{
  size_t payload=size*num;

  void* ptr=mymalloc(payload);

  memset((unsigned char*)ptr + S_BLOCK_SIZE, 0, payload);

  return ptr + S_BLOCK_SIZE;
}



Block* find_best_free_block(size_t size){
  int rest_b=__INT_MAX__;
  Block* best_block=NULL;

  for(Block* i=first_free_block;i != NULL;i=i->free_next){
    if(i->payload == size)
      return i;

    if(rest_b >= (int)(i->payload - size)){
      best_block=i;
      rest_b = (int)(i->payload - size);
    }
  }

  return best_block;
}



Block* split_free_block(Block* b, size_t new_size){
if(b == NULL){
  return NULL;
}

if(b->payload == new_size){
  b->free = 0;
  remove_node_freeList(b);
  
  return b;
}

size_t new_block_size = new_size + S_BLOCK_SIZE;


//update lista blocuri
Block* new = (Block*)((char*)b + new_block_size);
new->payload = (size_t)((b->payload) - new_size);
new->next = b->next;
new->prev = b;

if(b->next != NULL)
{
    b->next->prev = new;
}
b->next = new;
b->payload = new_size;
b->free = 0;

//update lista free
new->free_next = b->free_next;
new->free_prev = b->free_prev;
new->free = 1;

if(b->free_next != NULL)
  b->free_next->free_prev = new;

if(b->free_prev != NULL)
  b->free_prev->free_next = new;


if(b == first_free_block)
  first_free_block = new;

if(b == last_free_block)
  last_free_block = new;

return b;
}


void *myrealloc(void *ptr, size_t size){
  if(ptr == NULL){
    // ptr=mymalloc(size);
    // return (char*)ptr + S_BLOCK_SIZE;
    return mymalloc(size);
  }

  Block* aux=(Block*)((char*)ptr - S_BLOCK_SIZE);

  if(aux->next->free == 1){
    aux->free = 1;
    merge_blocks_Prev(split_free_block(aux->next, size - aux->payload));

    return (char*)aux + S_BLOCK_SIZE;
  }

  
  unsigned char buffer[aux->payload];
  memcpy(buffer, (char*)aux + S_BLOCK_SIZE, aux->payload);

  if(first_free_block != NULL){
    Block* new_realocated=split_free_block(find_best_free_block(size), size);
    if(new_realocated == NULL){
      perror("Eroare la realocare\n");
      return NULL;
      }

    memcpy((char*)new_realocated + S_BLOCK_SIZE, buffer, aux->payload);
    if(new_realocated == NULL){
      perror("Eroare la copierea mem dupa realocare!\n");
      return NULL;
    }   

    myfree(ptr);

    return (char*)new_realocated + S_BLOCK_SIZE;
  }
  else{
    Block* new_realocated=(Block*)mymalloc(size);
    if(new_realocated == NULL){
      perror("Eroare de alocare-realocare\n");
      return NULL;
    }

    memcpy((char*)new_realocated + S_BLOCK_SIZE, buffer, aux->payload);
    if(new_realocated == NULL){
      perror("Eroare la copierea mem dupa realocare!\n");
      return NULL;
    }

    myfree(ptr);

    return (char*)new_realocated + S_BLOCK_SIZE;
  }
}



void teste_mycalloc(){
  print_free_B();

  int* a=(int*)mycalloc(5,sizeof(int));

  for(int i=0;i<5;i++)
  printf("a[%i]= %i\t",i,a[i]);

  myfree(a);
  print_free_B();
}



void teste_myfree(){
  print_free_B();

  int* a=(int*)mymalloc(sizeof(int));
  if(a == NULL){
    perror("Eroare la alocare!\n");
    exit(EXIT_FAILURE);
  }

  *a=23;
  printf("a=%i\n",*a);

  int* b=(int*)mymalloc(2*sizeof(int));
  if(b == NULL){
    perror("Eroare la alocare!\n");
    exit(EXIT_FAILURE);
  }

  b[0]=6;
  b[1]=7;
  printf("b[0]=%i\tb[1]=%i\n", b[0],b[1]);

  char* c=(char*)mymalloc(2*sizeof(int)+2);
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

  list_blocks();

  myfree(b);
  print_free_B();

  myfree(a);
  print_free_B();

  myfree(c);
  print_free_B();

  list_blocks();
}



void teste_myrealloc(){
  int* a=(int*)mymalloc(sizeof(int));
  if(a == NULL){
    perror("Eroare la alocare!\n");
    exit(EXIT_FAILURE);
  }

  *a=23;
  printf("a=%i\n",*a);

  int* b=(int*)mymalloc(2*sizeof(int));
  if(b == NULL){
    perror("Eroare la alocare!\n");
    exit(EXIT_FAILURE);
  }

  char* c=(char*)mymalloc(2*sizeof(int)+2);
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


  b[0]=6;
  b[1]=7;
  printf("b[0]=%i\tb[1]=%i\n", b[0],b[1]);

  printf("c=%s\n\n",c);

  list_blocks();
  print_free_B();

  myfree(b);
  print_free_B();
  myfree(c);
  print_free_B();

  a = myrealloc(a,2*sizeof(int));
  a[1] = 45;
  printf("a[0]=%i\ta[1]=%i\n", a[0],a[1]);
  print_free_B();

  char* d = myrealloc(NULL,3*sizeof(char));
  if(d == NULL){
    perror("Eroare la realoc cu null!\n");
    exit(EXIT_FAILURE);
  }

  d[0]='d';
  d[1]='a';
  d[2]='\0';

  printf("d= %s\n",d);
  print_free_B();

  // list_blocks();

}



int main()
{
teste_myrealloc();

return 0;
}