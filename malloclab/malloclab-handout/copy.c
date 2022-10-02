/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

#define WORD 4
#define DORD 8
#define CHUNKSIZE (1<<12)
#define LIST_NUM 16


#define PACK(x,y) ((x)|(y))
#define GET(p) (*(unsigned int*)(p))
#define PUT(p,val) (*(unsigned int*)(p)=(val))

#define HEAD(p) (((char*)(p)-WORD))
#define GETSIZE(head) ((GET(head))&~(0x7))
#define TAIL(p) ((char*)(p)+(GETSIZE((char*)p-WORD))-DORD)
#define ALLOCED(head) ((GET(head))&0x1)

#define NEXT(p) ((char*)(p)+(GETSIZE((char*)p-WORD)))
#define PREV(p) ((char*)(p)-(GETSIZE(((char*)(p)-DORD))))

#define PRE_PTR(p) ((char*)p)
#define NEXT_PTR(p) ((char*)((char*)p+WORD))

int* list;
int* heap_start;


/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

void* find_fit(int size);
void add_list(char* elem,int size);
int get_list_index(int size);
// void place(void* p,int size);
void* join(char* elem);
void delete_list(char*elem);
void* cut_two(char*elem,int need_size);
void* mem_extend(int size);

int get_list_index(int size){
    int result = 0;  
  if(size&0xffff0000) {result += 16; size >>= 16; }  
  if(size&0x0000ff00) {result += 8; size >>= 8; }  
  if(size&0x000000f0) {result += 4; size >>= 4; }  
  if(size&0x0000000c) {result += 2; size >>= 2; }  
  if(size&0x00000002) {result += 1; size >>= 1; }  
  if(result>=(1<<(LIST_NUM+1))) {return LIST_NUM-1;}
  return result-2; 
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if((list=mem_sbrk(WORD*LIST_NUM))==(void*)-1){
        return -1;
    }
    for(int i=0;i<LIST_NUM;i++){
        PUT(list+i,NULL);
    }
    if((heap_start=mem_sbrk(WORD*4))==(void*)-1){
        return -1;
    }
    PUT(heap_start,0);
    PUT(heap_start+1,PACK(DORD,1));
    PUT(heap_start+2,PACK(DORD,1));
    PUT(heap_start+3,PACK(0,1));
    heap_start+=2;
    // printf("%x\n",heap_start);
    if(mem_extend(CHUNKSIZE/WORD)==NULL){
        printf("mm init error\n");
        return -1;
    }
    // printf("end of init\n");
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)                //注意，分配块和空闲块的最小大小不一样，所以大小检测标准不一样
{
    // printf("start of malloc\n");
    size+=8;
    if(size<16){
        size=16;
    }else{
        size=((size+DORD-1)/DORD)<<3;
    }
    return find_fit(size);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    // printf("start of free\n");
    if(!ptr){
        printf("free error\n");
        return;
    }
    // printf("%x\n",GET(ptr));
    int size=GETSIZE(HEAD(ptr));
    PUT(HEAD(ptr),PACK(size,0));
    PUT(TAIL(ptr),PACK(size,0));
    ptr=join(ptr);
    if(!ptr){
        printf("free error\n");
        return;
    }
    add_list(ptr,GETSIZE(HEAD(ptr))/WORD);
    return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    char* p=mm_malloc(size);
    int other_size=GETSIZE(HEAD(ptr))-DORD;
    size=size<other_size?size:other_size;
    for(int i=0;i<size;i++){
        p[i]=((char*)ptr)[i];
    }
    mm_free(ptr);
    return p;
}

void* mem_extend(int size){         //这里的size单位是字
    if(size<1)
        return NULL;
    char* ptr;
    if(size&1){
        size++;
    }
    if(size<4) size=4;
    int tmp=size;
    size*=WORD;
    // printf("extend of start\n");
    if((ptr=mem_sbrk(size))==(void*)-1){
        printf("mem extend error!\n");
        return NULL;
    }
    // printf("mid of extend\n");
    PUT(HEAD(ptr),PACK(size,0));
    PUT(TAIL(ptr),PACK(size,0));
    // printf("%x\n",HEAD(ptr));
    // printf("%x\n",TAIL(ptr));
    // printf("extend1\n");
    PUT(PRE_PTR(ptr),NULL);
    PUT(NEXT_PTR(ptr),NULL);
    // printf("extend2\n");
    PUT(HEAD(NEXT(ptr)),PACK(0,1));
    // printf("after put\n");
    ptr=join(ptr);
    // printf("after join\n");
    add_list(ptr,tmp);
    // printf("end of extend\n");
    return ptr;
}

void* find_fit(int size){   //单位为字
    // printf("start of fit\n");
    int index=get_list_index(size);
    size*=WORD;
    for(int i=index;i<LIST_NUM;i++){
        // printf("%d\n",i);
        unsigned int* tmp=GET((unsigned int*)list+i);
        // printf("before while\n");
        // printf("%x\n",tmp);
        while(tmp){
            if(GETSIZE(HEAD(tmp))>=size){
                // printf("%x\n",GETSIZE(HEAD(tmp)));
                // printf("before delete\n");
                delete_list(tmp);
                if(!cut_two(tmp,size)){
                    // printf("pos3 of fit\n");
                    PUT(HEAD(tmp),PACK(size,1));
                    PUT(TAIL(tmp),PACK(size,1));
                }
                // printf("end of fit\n");
                return tmp;
            }
            tmp=GET(NEXT_PTR(tmp));
        }
    }
    // printf("mid of fit\n");
    int c_size=(size>CHUNKSIZE?size:CHUNKSIZE);
    void* ptr;
    if(!(ptr=mem_extend(c_size))){
        printf("mem_extend error\n");
        return NULL;
    }
    // printf("pos1 of fit\n");
    delete_list(ptr);
    // printf("pos2 of fit\n");
    if(!cut_two(ptr,size)){
        // printf("pos3 of fit\n");
        PUT(HEAD(ptr),PACK(c_size,1));
        PUT(TAIL(ptr),PACK(c_size,1));
    }
    // printf("end of fit\n");
    return ptr;
}

// void place(void* p,int size){

// }

void add_list(char* elem,int size){
    // printf("start of add\n");
    int index=get_list_index(size);         //这里size单位是字
    // printf("%d\n",index);
    // printf("pos1 of add\n");
    PUT(NEXT_PTR(elem),list[index]);
    if(list[index]){
        PUT(PRE_PTR(list[index]),elem);
    }
    // printf("pos2 of add\n");
    PUT(&list[index],elem);
    PUT(PRE_PTR(elem),&list[index]);
    // printf("%x\n",PRE_PTR(elem));
    // printf("%x\n",GET(elem));
    // printf("end of add\n");
    return;
}

void* join(char* elem){                 //剩下重置空闲队列
    // printf("start of join\n");
    // printf("%x\n",GETSIZE(elem-WORD));
    // printf("%x\n",HEAD(PREV(elem)));
    // printf("%x\n",ALLOCED(HEAD(PREV(elem)))); //GET处segment fault
    int judge1=ALLOCED(HEAD(PREV(elem)));
    // printf("hhh\n");
    // printf("%x %x %x %x\n",GET(elem-WORD),GET(elem),GET(elem+WORD),GET(elem+WORD*2));
    int judge2=ALLOCED(HEAD(NEXT(elem)));
    // printf("%x\n",GET(HEAD(NEXT(elem))));
    // printf("pos1 of join\n");
    if(judge1&&judge2){
        return elem;
    }else if(!judge1&&judge2){
        char* pre=PREV(elem);
        int size=(GETSIZE(HEAD(elem)))+(GETSIZE(HEAD(pre)));
        delete_list(pre);
        PUT(HEAD(pre),PACK(size,0));
        PUT(TAIL(pre),PACK(size,0));
        return pre;
    }else if(judge1&&!judge2){
        char* next=NEXT(elem);
        int size=(GETSIZE(HEAD(elem)))+(GETSIZE(HEAD(next)));
        delete_list(next);
        PUT(HEAD(elem),PACK(size,0));
        PUT(TAIL(elem),PACK(size,0));
        return elem;
    }else{
        char* pre=PREV(elem);
        char* next=NEXT(elem);
        delete_list(pre);
        delete_list(next);
        int size=(GETSIZE(HEAD(elem)))+(GETSIZE(HEAD(pre)))+(GETSIZE(HEAD(next)));
        PUT(HEAD(pre),PACK(size,0));
        PUT(TAIL(pre),PACK(size,0));
        return pre;
    }
}

void delete_list(char*elem){
    // printf("start of delete\n");
     unsigned int tmp=GET(NEXT_PTR(elem));
    //  printf("before if\n");
     if(tmp){
        // printf("%x\n",GET(HEAD(elem)));
        // printf("%x\n",elem);
        // printf("%x\n",tmp);
        // printf("%x\n",GET(PRE_PTR(elem)));
        // printf("%x %x %x %x\n",GET(elem-WORD),GET(elem),GET(elem+WORD),GET(elem+WORD*2));
            PUT(PRE_PTR(tmp),GET(PRE_PTR(elem)));
        }
        // printf("mid of delete\n");
        // printf("%x\n",GET(PRE_PTR(elem)));
        if(GET(PRE_PTR(elem))<heap_start){
            PUT(GET(PRE_PTR(elem)),tmp);
        }else{
            PUT(NEXT_PTR(GET(PRE_PTR(elem))),tmp);
        }
        PUT(PRE_PTR(elem),NULL);
        PUT(NEXT_PTR(elem),NULL);
        // printf("end of delete\n");
    return;
}

//切成两块且把第二块放在空闲队列里，第一块视为已分配。need_size必须为满足约束条件的块
void* cut_two(char*elem,int need_size){     //单位为字节
    // printf("start of cut\n");
    int minus=GETSIZE(HEAD(elem))-need_size;
    if(minus<4*WORD){
        return NULL;
    }
    // printf("pos1 of cut\n");
    PUT(HEAD(elem),PACK(need_size,1));
    PUT(TAIL(elem),PACK(need_size,1));
    // printf("pos2 of cut\n");
    char* next=NEXT(elem);
    PUT(HEAD(next),PACK(minus,0));
    // printf("pos3 of cut\n");
    PUT(TAIL(next),PACK(minus,0));
    add_list(next,minus/WORD);
    // printf("end of cut\n");
    return next;
}










