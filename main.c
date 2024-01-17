#include"mymalloc.h"
#include<stdio.h>

void teste_mymalloc(){
	int *a=(int*)mymalloc(sizeof(int));
	*a=5;
	printf("%i\n",*a);

	myfree(a);
	printf("S-a exec myfree()\n");

	char* str=(char*)mymalloc(sizeof(char)*6);
	str[0]='s';
	str[1]='a';
	str[2]='l';
	str[3]='u';
	str[4]='t';
	str[5]='\n';

	printf("str meu: %s\n\n",str);
	myfree(str);

	char* huge=(char*)mymalloc(sizeof(char)*1000000);
	strcpy(huge,"hello there");
	printf("%s\n",huge);

	myfree(huge);
}

void teste_merge(){
	int *a=(int*)mymalloc(sizeof(int));

	int *b=(int*)mymalloc(2*sizeof(int));

	char* c=(char*)mymalloc(sizeof(char)*10);
	list_blocks();
	myfree(a);

	myfree(c);
	list_blocks();

	myfree(b);
}

void teste_myrealloc(){
	int* a=(int*)mymalloc(sizeof(int));
	*a=8;
	printf("a= %i\n",*a);

	int* b = a;
	printf("b= %i\n",*b);
	b=(int*)myrealloc(NULL,sizeof(int)*3);

	char* c=(char*)mymalloc(sizeof(char)*4096);
	myfree(c);
	
	list_blocks();

	a=(int*)myrealloc(a, sizeof(int)*2);
	a[1]=10;
	for(int i=0;i<2;i++)
		printf("a[%i]= %i\n",i,a[i]);
	printf("\n");

	myfree(a);
  	list_blocks();
	myfree(b);
}	


int main(){
	//teste_mymalloc();
	teste_merge();
	teste_myrealloc();


	printf("\n\nPRINT FINAL:\n");
	list_blocks();

	return 0;
}