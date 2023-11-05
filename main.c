#include<unistd.h>
#include<stdlib.h>
#include<stdbool.h>
#include<stdio.h>


#define SIZE_HEAP 64

typedef struct data_block
{
    struct data_block* next;
    struct data_block* prev;
    size_t dim_bloc;
    bool liber;

}data_block;

static data_block* heap_liber[SIZE_HEAP];
static data_block* start_node=NULL;

data_block* create_node(size_t dim){
    data_block* new_node=(data_block*)sbrk(sizeof(data_block));

    new_node->dim_bloc=dim;
    new_node->liber=false;
    new_node->next=NULL;
    new_node->prev=NULL;

    return new_node;
}

void insert_new_node(data_block** head, size_t dim){
    //data_block* curent_node=*head;
    data_block* new_node=create_node(dim);

    if(*head != NULL){
        (*head)->next=new_node;
    }

    new_node->prev=*head;

    *head=new_node;
}

void* mymalloc(size_t dim){
    insert_new_node(&start_node,dim);

    return (void*)&start_node;
}

int main(){
    int* a=(int*)mymalloc(sizeof(int)*4);
    for(int i=0;i<4;i++)
    {
        a[i]=i+3;
    }

    for(int i=0;i<4;i++){
        printf("a[%i]= %i\n",i,a[i]);
    }
    
    return 0;
}