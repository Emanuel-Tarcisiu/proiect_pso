#include"mymalloc.h"

pthread_mutex_t mutex_mymalloc = PTHREAD_MUTEX_INITIALIZER;


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

Block* first_free_block = NULL;
Block* last_free_block = NULL;


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

  printf("\ndimensiunea unui Block= %i\n\n",(int)sizeof(Block));
}



Block* find_best_free_block(size_t size){
  int rest_b=__INT_MAX__;
  Block* best_block=NULL;

  for(Block* i=first_free_block;i != NULL;i=i->free_next){
    if(i->payload == size)
      return i;

    if(i->payload >= (2*S_BLOCK_SIZE) + size)
      if(rest_b >= (int)((i->payload - 2*S_BLOCK_SIZE) - size)){
        best_block=i;
        rest_b = (int)((i->payload - S_BLOCK_SIZE) - size);
      }
    else
      continue;
  }

  return best_block;
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



Block* split_free_block(Block* b, size_t new_size){
if(b == NULL){
  return NULL;
}

if((b->payload) == new_size){
  b->free = 0;
  remove_node_freeList(b);
  
  return b+1;
}


//update lista blocuri
Block* new = (Block*)((char*)b + new_size + S_BLOCK_SIZE);
new->payload = (size_t)((b->payload) - new_size - S_BLOCK_SIZE);
new->next = b->next;
new->prev = b;


if(last_block == b)
  last_block = new;


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



void remove_free_block(Block* wanted)
{
  Block* b=first_free_block;
  if(wanted == last_free_block && wanted == first_free_block)
  {
    last_free_block=NULL;
    first_free_block=NULL;;
    return;
  }

  if(wanted==last_free_block)
  {
      
      last_free_block=last_free_block->free_prev;


      last_free_block->free_next=NULL;
      return;
  }

  if(wanted==first_free_block)
  {
    first_free_block=first_free_block->free_next;
    first_free_block->free_prev=NULL;
    return;
  }

    while(b)
    {
        if(b==wanted)
        {
            b->free_prev->free_next=b->free_next;

            if(b->free_next!=NULL)
            {
              b->free_next->free_prev=b->free_prev;
            }
            return;
        }

        b=b->free_next;
    }
}


void add_free_block(Block* new_block)
{
    if(first_free_block==NULL)
    {
        first_free_block=new_block;
        new_block->free_next=NULL;
        new_block->free_prev=NULL;
        last_free_block=new_block;
    }
    else
    {
      last_free_block->free_next=new_block;
      new_block->free_prev=last_free_block;
      new_block->free_next=NULL;
      last_free_block=new_block;
    }
}



Block* split_block(Block* b,size_t size)
{
    if(size<=S_BLOCK_SIZE || b->payload-size<=S_BLOCK_SIZE)
    {
      return NULL;
    }

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
    else
    {
      return NULL;
    }

}



Block* merge_blocks_Next(Block* curent_node){
  if(curent_node->next == NULL || curent_node->next->free == 0)
    return NULL;
    
  Block* next_node = curent_node->next;

  if(last_block == next_node)
    last_block = curent_node;

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

  if(last_block == curent_node)
    last_block = prev_node;

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



int try_merge_free_blocks(Block* a,Block* b)
{
    if(a==NULL || b== NULL)
    {
      return 0;
    }

    if(a->next!=b || b->prev!=a)
    {
      return 0;
    }

    if(a->free==1 && b->free==1 )
    {
      
        a->next=b->next;

        if(b->next!=NULL)
        {
          b->next->prev=a;
        }

        a->payload=a->payload+S_BLOCK_SIZE+b->payload;    

        return 1;
    }

    return 0;

}


void myfree(void *ptr)
{
    Block* block=(Block*)((char*)(ptr)-S_BLOCK_SIZE);
    block->free=1;

    
    pthread_mutex_lock(&mutex_mymalloc);

    if(last_free_block == NULL){
      first_free_block = block;
      last_free_block = block;
    }
    else{
      int updates=0;
      Block* p=merge_blocks_Prev(block);
      if(p != block && p != NULL){
        updates += 1;
        block=p;}

      p = merge_blocks_Next(block);
      if(p != NULL){
        updates += 2;
        block=p;}

      if(updates == 0){
        last_free_block->free_next = block;
        last_free_block = block;
      }

    }
    
    pthread_mutex_unlock(&mutex_mymalloc);

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



Page* get_page_of_block(Block* block)
{
  Page* s=START_PAGE;

  if(s==NULL)
  {
    return NULL;
  }

  while(s)
  {
    if(s->first_page_block && s->last_page_block)
    {
      if(s->first_page_block <= block && block <= s->last_page_block)
      {
        return s;
      }
      
    }

    s=s->next;
  }

  return NULL;
}



size_t get_page_payload(Page* page)
{
  if(page==NULL)
  {
    return 0;
  }

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
  size_t needed=size+S_PAGE_SIZE+S_BLOCK_SIZE;
  int multiply=needed/PAGE_SIZE+1;

  void *ptr = mmap(NULL,multiply * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (ptr == MAP_FAILED) {
       return NULL;
    }

  Page* new_page=(Page*)(ptr);
  new_page->capacity=multiply*PAGE_SIZE-S_PAGE_SIZE;

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



void add_block_to_page(Page* page,Block* block)
{
    Block* first=page->first_page_block;
    Block* last=page->last_page_block;

    if(first==NULL)
    {
      page->first_page_block=block;
      page->last_page_block=block;
      block->next=NULL;
      block->prev=NULL;
    }
    else
    {
      page->last_page_block->next=block;
      block->prev=last;
      block->next=NULL;
      page->last_page_block=block;
    }
    
    
}



void init_page(Page* page)
{
    if(page==NULL)
    {
      perror("init_page:page is NULL");
      return;
    }

    Block* new_block=(Block*)((char*)page+S_PAGE_SIZE);
    new_block->free=1;
    new_block->payload=page->capacity-S_BLOCK_SIZE;

    new_block->next=NULL;
    new_block->prev=NULL;
    new_block->free_next=NULL;
    new_block->free_prev=NULL;

    add_free_block(new_block);
    add_block_to_page(page,new_block);
}



void remove_block_from_page(Page* page,Block* block)
{
    Block* b=page->first_page_block;

    if(block==page->first_page_block && block==page->last_page_block)
    {
      page->last_page_block=NULL;
      page->first_page_block=NULL;
      return;
    }

    if(block==page->last_page_block)
    {
      page->last_page_block=page->last_page_block->prev;
      page->last_page_block->next=NULL;

      return;
    }

    while(b)
    {
      if(b==block)
      {
        b->prev->next=b->next;

        if(b->next)
        {
          b->next->prev=b->prev;
        }

        return;
      }

      b=b->next;
    }
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
  ptr->free=0;

  return ptr;
}



Block* start_allocating_pages(size_t size)
{
  Page* current_page=NULL;
  Block* block=NULL;

  if(START_PAGE==NULL)
  {
    current_page = alloc_new_page(size);

    if(current_page==NULL)
    {
      return NULL;
    }

    init_page(current_page);
  }

  block=find_best_free_block(size);

  if(block)
  {
    
    current_page=get_page_of_block(block);

    remove_free_block(block);

    if(split_block(block,size))
    {
      add_free_block(block->next);
      add_block_to_page(current_page,block->next);
    }

    block->free=0;
    return block;
  }
  else
  {
    current_page = alloc_new_page(size);


    if(current_page==NULL)
    {
      return NULL;
    }

    init_page(current_page);

    block=find_best_free_block(size);
    
    current_page=get_page_of_block(block);

    remove_free_block(block);

    if(split_block(block,size))
    {
      add_free_block(block->next);
      add_block_to_page(current_page,block->next);
    }

    block->free=0;
    return block;
  }
}



void add_block(Block* block)
{
  if(start_block==NULL)
  {
    block->next=NULL;
    block->prev=NULL;

    start_block=block;
    last_block=block;
  }
  else
  {
    last_block->next=block;
    block->prev=last_block;
    block->next=NULL;
    last_block=block;
  }
}



void* mymalloc(size_t size)
{
  Block* ptr = find_best_free_block(size);

  if(ptr)
  {
    pthread_mutex_lock(&mutex_mymalloc);
    ptr = split_free_block(ptr,size);
    pthread_mutex_unlock(&mutex_mymalloc);

        return (Block*)(ptr+1);
  }
  else
  {
    pthread_mutex_lock(&mutex_mymalloc);
    ptr=extend_heap(size);

    if(ptr==(void*)-1)
    {
      ptr=start_allocating_pages(size);

      if(ptr==NULL)
      {
         pthread_mutex_unlock(&mutex_mymalloc);
        return NULL;
      }

      ptr=(Block*)(ptr);

      add_block(ptr);
      pthread_mutex_unlock(&mutex_mymalloc);
	
      ptr->payload=size;
      ptr->free=0;

      return (Block*)(ptr+1);
    }
    else
    {
      add_block(ptr);
      pthread_mutex_unlock(&mutex_mymalloc);

      ptr->payload=size;
      return (Block*)(ptr+1);
    }
    
  }
}



void *mycalloc(size_t num, size_t size)
{
  size_t payload=size*num;
  void* ptr=mymalloc(payload);
  memset((unsigned char*)ptr , 0, payload);

  return ptr;
}



void *myrealloc(void *ptr, size_t size){
  if(ptr == NULL){
    return mymalloc(size);
  }

  Block* aux=(Block*)((char*)ptr - S_BLOCK_SIZE);

  if(aux->next->free == 1 && aux->next->payload > 2*S_BLOCK_SIZE){
    pthread_mutex_lock(&mutex_mymalloc);
    aux->free = 1;
    Block* new_realocated = merge_blocks_Prev(split_free_block(aux->next, size - aux->payload));
    pthread_mutex_unlock(&mutex_mymalloc);
    if(new_realocated == NULL){
      perror("Eroare la realocare\n");
      return NULL;
      }

    new_realocated->free = 0;

    return (char*)new_realocated + S_BLOCK_SIZE;
  }
  
  unsigned char buffer[aux->payload];
  memcpy(buffer, (char*)aux + S_BLOCK_SIZE, aux->payload);

  if(first_free_block != NULL){
    pthread_mutex_lock(&mutex_mymalloc);
    Block* new_realocated=split_free_block(find_best_free_block(size), size);
    pthread_mutex_unlock(&mutex_mymalloc);
    if(new_realocated == NULL){
      goto no_free_gud_block;
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
  no_free_gud_block:
    Block* new_realocated = (Block*)mymalloc(size);
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
