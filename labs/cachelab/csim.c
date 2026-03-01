#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

typedef struct {
    int vaild;
    int tag;
    int latency;
} CacheLine;
typedef struct {
    int set_index;
    CacheLine* cacheLine;
} Set;

int hit_count = 0;
int miss_count = 0;
int eviction_count = 0;
int s = 0, E = 0, b = 0;
Set* cache;

unsigned long long get_set_index(unsigned long long address){
    unsigned long long mask = (unsigned long long) ~0>>(64-s);
    return (address >> b) & mask;
}

unsigned long long get_tag(unsigned long long address){
    return address>>(s+b);
}

void cache_access(unsigned long long set_index, unsigned long long tag){
    Set* pt = cache + set_index;
    int max_latency = 0;
    CacheLine* override_pt = &pt -> cacheLine[0];
    int i;
    for(i = 0; i < E; i++){
        CacheLine* line_pt = &pt -> cacheLine[i];
        line_pt->latency ++;
    }
    for(i = 0; i < E; i++){
        CacheLine* line_pt = &pt -> cacheLine[i];
        if (line_pt->vaild == 0){
            line_pt->vaild = 1;
            line_pt->latency = 0;
            line_pt->tag = tag;
            miss_count++;
            return;
        }
        if(line_pt->tag == tag) {
            line_pt->tag = tag;
            line_pt->latency = 0;
            hit_count++;
            return;
        }
        if(max_latency < line_pt->latency) {
            override_pt = line_pt;
            max_latency = line_pt->latency;
        }
    }
    miss_count++;
    eviction_count++;
    override_pt->tag = tag;
    override_pt->latency = 0;
    return;
}

int main(int argc, char *argv[]) {
    int opt;
    char *trace_file = NULL;
    int verbose = 0;

    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
            case 'h':
                break;
            case 'v':
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
                break;
            default:
                printf("参数输入错误了喵！\n");
                exit(1);
        }
    }

    int S = 1<<s;

    FILE *trace_fp = fopen(trace_file, "r");
    if (trace_fp == NULL) {
        printf("打开文件失败");
        exit(1);
    }

    char operation;
    unsigned long long address;
    int size;

    cache = (Set*) malloc(S * sizeof(Set));
    for(int i = 0; i < S; i++){
        Set* pt = &cache[i];
        pt -> set_index = i;
        pt -> cacheLine = (CacheLine*) malloc(E * sizeof(CacheLine));
        for(int j = 0; j < E; j++){
            pt->cacheLine[j].vaild=0;
            pt->cacheLine[j].tag=0;
            pt->cacheLine[j].latency=0;
        }
    }

    while (fscanf(trace_fp, " %c %llx,%d", &operation, &address, &size) > 0) {
        if (verbose) {
            printf("%c %llx,%d\n", operation, address, size);
        }
        unsigned long long set_index = get_set_index(address);
        unsigned long long tag = get_tag(address);
        switch (operation) {
            case 'I':
                break;
            case 'L': 
                cache_access(set_index, tag);
                break;
            case 'S': 
                cache_access(set_index, tag);
                break;
            case 'M': 
                cache_access(set_index, tag);
                cache_access(set_index, tag);
                break;
            default:
                break;
        }
    }
    printSummary(hit_count, miss_count, eviction_count);
    fclose(trace_fp);
    return 0;
}