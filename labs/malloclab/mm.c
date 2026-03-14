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

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Poppin' Party",
    /* First member's full name */
    "Tab_1bit0",
    /* First member's email address */
    "frojbk@gmail.com",
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

#define GROUP 12
#define SINGLE 4
#define DOUBLE 8
#define OFFSET -4
#define MINIMUM_SIZE 16

static char* heap_listp;
static char* endp;
static size_t* seg;

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define HEADER(p) (*(unsigned int *)((char *)(p) + OFFSET))
#define SIZE(p) (HEADER(p) & ~0x1)
#define ISALLOCATED(p) (HEADER(p) & 0x1)
#define FOOTER(p) (*(unsigned int *)((char *)(p) + SIZE(p) + (OFFSET*2)))
#define SET_HEADER(p, size, alloc) (PUT((unsigned int *)((char *)(p) + OFFSET), PACK(size, alloc)))
#define SET_FOOTER(p) (FOOTER(p) = HEADER(p))
#define GET_PREV_FOOTER(p) (*(unsigned int *)((char *)(p) + (OFFSET*2)))
#define GET_PREV_SIZE(p) (GET_PREV_FOOTER(p) & ~0x1)
#define GET_PREV(p) ((size_t *)((char *)(p) - GET_PREV_SIZE(p)))
#define GET_PREV_ISALLOCATED(p) (GET_PREV_FOOTER(p) & 0x1)
#define GET_NEXT(p) ((size_t *)((char *)(p) + SIZE(p)))
#define GET_NEXT_HEADER(p) (*(unsigned int *)((char *)(GET_NEXT(p)) + OFFSET))
#define GET_NEXT_SIZE(p) (GET_NEXT_HEADER(p) & ~0x1)
#define GET_NEXT_ISALLOCATED(p) (GET_NEXT_HEADER(p) & 0x1)
#define SPLIT(p, size, alloc) (PUT(((char *)(p) + SIZE(p) + OFFSET), PACK(size, alloc)))

int getSeg(int size) {
    int i=0;
    while (i < GROUP - 1  && (size >> (1+i)) != 0){
        i++;
    }
    return i;
}
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(GROUP*DOUBLE)) == (void *)-1){
        return -1;
    }
    seg = (size_t *)heap_listp;
    heap_listp += (GROUP*DOUBLE);

    if ((heap_listp = mem_sbrk(4*SINGLE)) == (void *)-1){
        return -1;
    }

    PUT(heap_listp, 0);
    PUT(heap_listp + (1*SINGLE), PACK(DOUBLE, 1));
    PUT(heap_listp + (2*SINGLE), PACK(DOUBLE, 1));
    PUT(heap_listp + (3*SINGLE), PACK(0, 1));
    heap_listp += (2*SINGLE);
    endp = heap_listp + SINGLE;

    int i;
    for(i = 0; i < GROUP; i++){
        seg[i] = NULL;
    }
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int asize = ALIGN(size + DOUBLE);
    int i = getSeg(asize);
    while (i < GROUP){
        size_t *ptr = seg[i];
        size_t *prep = NULL;
        while (ptr != NULL){
            int k = SIZE(ptr) - asize;
            size_t* nextp;
            if (k >= 0) {
                if (k > 0 && k < MINIMUM_SIZE) {
                    asize = SIZE(ptr);
                    k = 0;
                }
                SET_HEADER(ptr, asize, 1);
                SET_FOOTER(ptr);
                nextp = (size_t *)GET_NEXT(ptr);
                if (prep == NULL) {
                    seg[i] = (size_t)*ptr;
                }else{
                    *prep = (size_t)*ptr;
                }
                if (k >= MINIMUM_SIZE){
                    SET_HEADER(nextp, k, 0);
                    SET_FOOTER(nextp);
                    int j = getSeg(k);
                    *(size_t*)nextp = seg[j];
                    seg[j] = (size_t)nextp;
                }
                return (void *)ptr;
            }
            prep = ptr;
            ptr = (size_t *)*ptr;
        }
        i++;
    }

    if (!GET_PREV_ISALLOCATED((char *)endp + SINGLE)) {
        size_t *prevp = GET_PREV((char *)endp + SINGLE);
        int psize = SIZE(prevp);
        int k = asize - psize;
        void *p = mem_sbrk(k);
        if (p == (void *)-1){
            return NULL;
        } else {
             int j = getSeg(psize);
            size_t *oldptr = (size_t *)(&seg[j]);
            while ((size_t *)*oldptr != prevp){
                oldptr = (size_t *)*oldptr;
            }
            *oldptr = *prevp;
            endp = (char*)p + k + OFFSET;
            PUT(endp, PACK(0, 1));
            SET_HEADER(prevp, asize, 1);
            SET_FOOTER(prevp);
            return prevp;
        }
    } else {
        void *p = mem_sbrk(asize);
        if (p == (void *)-1){
            return NULL;
        } else {
            endp = (char*)p + asize + OFFSET;
            PUT(endp, PACK(0, 1));
            SET_HEADER(p, asize, 1);
            SET_FOOTER(p);
            return (void *)((char *)p);
        }
    }


}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t *p = ptr;
    int newsize = SIZE(p);
    if (!GET_NEXT_ISALLOCATED(p)) {
        int nextsize = GET_NEXT_SIZE(p);
        newsize += nextsize;
        size_t *nextp = (size_t *) GET_NEXT(p);
        int j = getSeg(nextsize);
        size_t *oldptr = (size_t *)(&seg[j]);
        while ((size_t *)*oldptr != nextp){
            oldptr = (size_t *)*oldptr;
        }
        *oldptr = *nextp;
    }
    if (!GET_PREV_ISALLOCATED(p)) {
        int prevsize = GET_PREV_SIZE(p);
        newsize += prevsize;
        size_t *prevp = (size_t *) GET_PREV(p);
        int j = getSeg(prevsize);
        size_t *oldptr = (size_t *)(&seg[j]);
        while ((size_t *)*oldptr != prevp){
            oldptr = (size_t *)*oldptr;
        }
        *oldptr = *prevp;
        p = prevp;
    }
    int i = getSeg(newsize);
    SET_HEADER(p, newsize, 0);
    SET_FOOTER(p);
    *(size_t *)p = seg[i];
    seg[i] = (size_t)p;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    if (ptr == NULL){
        return mm_malloc(size);
    }
    if (size == 0){
        mm_free(ptr);
        return NULL;
    }
    void *newptr;
    size_t copySize;
    size = ALIGN(size+DOUBLE);
    copySize = SIZE(ptr);
    int k = copySize - size;

    if (k >= 0) {
        newptr = ptr;
        if (k < MINIMUM_SIZE){
            return newptr;
        }
        SET_HEADER(ptr, size, 1);
        SET_FOOTER(ptr);
        size_t * nextp = GET_NEXT(ptr);
        SET_HEADER(nextp, k, 0);
        SET_FOOTER(nextp);
        int j = getSeg(k);
        *nextp = seg[j];
        seg[j] = (size_t)nextp;
        return newptr;
    }

    int fullsize = GET_NEXT_ISALLOCATED(ptr)? copySize : copySize + GET_NEXT_SIZE(ptr);
    k = fullsize - size;
    if (k >= 0) {
        newptr = ptr;
        if (k < MINIMUM_SIZE){
            size = fullsize;
            k = 0;
        }
        size_t * nextp = GET_NEXT(ptr);
        int i = getSeg(SIZE(nextp));
        size_t *oldptr = (size_t *)(&seg[i]);
        while ((size_t *)*oldptr != nextp){
            oldptr = (size_t *)*oldptr;
        }
        *oldptr = *(size_t *)nextp;
        
        SET_HEADER(ptr, size, 1);
        SET_FOOTER(ptr);
        
        if (k != 0){
            nextp = GET_NEXT(ptr);
            SET_HEADER(nextp, k, 0);
            SET_FOOTER(nextp);
            int j = getSeg(k);
            *nextp = seg[j];
            seg[j] = (size_t)nextp;      
        }

        return newptr;
    }

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    if (size < copySize)
      copySize = size;
    memcpy(newptr, ptr, copySize - DOUBLE);
    mm_free(ptr);

    return newptr;
}














