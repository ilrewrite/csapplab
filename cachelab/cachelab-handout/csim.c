#include "cachelab.h"
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

typedef struct Elem
{
    int vaild;
    int tag;
    int offset;
    int least;
}Elem,*line;

typedef line* cache;

typedef struct Item{        //用于存储字符串切割后的结果
    char* type;
    int address;
    int size;
}Item,*item;

item split(char* str){        
    item res= (item)malloc(sizeof(Item));
    res->type=(char*)malloc(20*sizeof(char));
    sscanf(str," %s %x,%d",res->type,&(res->address),&(res->size));
    if(!strcmp(res->type,"I"))
        return NULL;
    return res;
}

int get_file_len(char* filename){
    FILE* f=fopen(filename,"r");
    char buffer[25]="";
    int len=0;
    while(fgets(buffer,20,f)!=NULL){
        if(buffer[0]==' ')
            len++;
    }
    fclose(f);
    return len;
}

int main(int argc,char** argv)
{
    // int is_verbose=0;
    int s=0;
    int b=0;
    int E=0;
    char* filename;
    // if(!strcmp(argv[0],"-h")){
    //     return 0;
    // }
    // else if(argc<8){
    //     return 1;
    // }else if(argc>8){
    //     is_verbose=1;
    // }
    for(int i=0;i<argc;i++){        //may be have to implement -h
        if(!strcmp(argv[i],"-v")){
            // is_verbose=1;
        }else if(!strcmp(argv[i],"-s")){
            s=atoi(argv[++i]);
        }else if(!strcmp(argv[i],"-b")){
            b=atoi(argv[++i]);
        }else  if(!strcmp(argv[i],"-E")){
            E=atoi(argv[++i]);
        }else if(!strcmp(argv[i],"-t")){
            filename=argv[++i];
        }
    }
    if(!(s&&b&&E)){
        printf("error args");
        return 1;
    }
    int S=1<<s;
    cache mycache=(line*)malloc((S)*sizeof(line));
    for(int i=0;i<S;i++){
        mycache[i]=(line)malloc(E*sizeof(Elem));
        for(int j=0;j<E;j++){
            mycache[i][j].vaild=0;
            mycache[i][j].tag=-1;
        }
    }
    int tag=32-b-s;
    // int len=get_file_len(filename);
    // char** mymap=(char**)malloc(sizeof(char*)*len);
    FILE* f=fopen(filename,"r");
    char buffer[25]="";
    int index=0;
    int miss=0;
    int hit=0;
    int eviction=0;
    while(fgets(buffer,20,f)!=NULL){
        item tmp=split(buffer);
        if(!tmp){
            continue;
        }
        int set_number=(unsigned int)((tmp->address)<<tag)>>(tag+b);
        int mem_tag=(unsigned int)(tmp->address)>>(b+s);
        int is_hit=0;
        int back_way=-1;
        int least_index=-1;
        for(int i=0;i<E;i++){
            if(back_way==-1&&mycache[set_number][i].vaild==0){
                back_way=i;
            }else if(mycache[set_number][i].vaild&&mycache[set_number][i].tag==mem_tag){
                hit++;
                if(!strcmp(tmp->type,"M")){
                    hit++;
                }
                is_hit=1;
                mycache[set_number][i].least=index;
                break;
            }else if(mycache[set_number][i].vaild){
                if(least_index==-1){
                    least_index=i;
                }else{
                    least_index=mycache[set_number][i].least<mycache[set_number][least_index].least?i:least_index;
                }
            }
        }
        if(!is_hit){
            miss++;
            if(back_way!=-1){
                mycache[set_number][back_way].least=index;
                mycache[set_number][back_way].tag=mem_tag;
                mycache[set_number][back_way].vaild=1;
            }else if(least_index!=-1){
                eviction++;
                mycache[set_number][least_index].least=index;
                mycache[set_number][least_index].tag=mem_tag;
                mycache[set_number][least_index].vaild=1;
            }
            if(!strcmp(tmp->type,"M")){
                hit++;
            }
        }
        free(tmp->type);
        free(tmp);
        index++;
    }
    fclose(f);
    for(int i=0;i<S;i++){
        free(mycache[i]);
    }
    free(mycache);
    printSummary(hit,miss,eviction);
    return 0;
}
