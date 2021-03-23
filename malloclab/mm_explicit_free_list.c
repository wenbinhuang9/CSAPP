// using implicit free list data structure 
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

#define WSIZE 4
#define DSIZE 8
#define CHUNSIZE (1 << 12)
#define MAX(x, y) ((x) > (y) ? (x) : (y))

// pack a size and allocated bit into a word 
#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = val)

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp)  + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE((char*)(bp)-DSIZE))

#define GET_NEXT(bp) (GET(bp))
#define PUT_NEXT(bp, val) (PUT(bp, (unsigned int)val))
#define GET_PREV(bp) (GET((char*)(bp) +  WSIZE))
#define PUT_PREV(bp, val) (PUT((char*)(bp) + WSIZE, (unsigned int)val))

static void *first_fit(size_t size);
static void place(void *p, size_t size);
static void *coalesce(void *p);
static void *extend_heap(size_t size);
static void insert_after_root(void *bp);
static void disconnect(void *bp);
static int mm_check();
static char* heap_listp; 
static char* root;

static void disconnect(void *bp) {
    unsigned int pre = GET_PREV(bp);
    unsigned int next = GET_NEXT(bp);

    PUT_NEXT(pre, next);

    if(next) { PUT_PREV(next, pre);} 
}

static void insert_after_root(void *bp) {
    unsigned int root_next = GET_NEXT(root);

    PUT_NEXT(root, bp);
    PUT_PREV(bp, root);

    PUT_NEXT(bp, root_next);
    if(root_next) PUT_PREV(root_next, bp);
}


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if((heap_listp = mem_sbrk(6 * WSIZE)) == (void *) -1) {
        return -1;
    }

    PUT(heap_listp, 0);  // padding alignment 
    root = heap_listp + 1 * WSIZE;
    PUT(heap_listp +  1 *WSIZE, 0);  //root next
    PUT(heap_listp +  2 *WSIZE, 0);  // root prev

    PUT(heap_listp + 3 * WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 4 * WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 5 * WSIZE, PACK(0, 1));

    heap_listp += (4 * WSIZE);

    if (extend_heap(CHUNSIZE/WSIZE) == NULL) {
        return -1;
    }
    //mm_check();
    return 0;
}

void place(void *bp, size_t size) {
    //split into two blocks
    size_t total_size = GET_SIZE(HDRP(bp));
    size_t second_block_size = total_size - size;

    if (second_block_size >= 2 * DSIZE) {
        //set first block header
        PUT(HDRP(bp), PACK(size, 1));
        //set first block footer
        PUT(FTRP(bp), PACK(size, 1));
        void * next_bp = NEXT_BLKP(bp);
        //set sec block header
        PUT(HDRP(next_bp), PACK(second_block_size, 0));
        //set sec block footer
        PUT(FTRP(next_bp), PACK(second_block_size, 0));

        disconnect(bp);

        insert_after_root(next_bp);
    }else {
        //set first block header
        PUT(HDRP(bp), PACK(total_size, 1));
        //set first block footer
        PUT(FTRP(bp), PACK(total_size, 1));

        disconnect(bp); 
    }

    return; 
}

void *extend_heap(size_t words) {
    char* bp;
    size_t size; 

    size = (words % 2 == 1) ? ((words + 1) * WSIZE):(words * WSIZE);

    if((long)(bp = mem_sbrk(size)) == -1) {
        return NULL;
    }

    PUT(HDRP(bp), PACK(size, 0));// set header size and free 
    PUT(FTRP(bp), PACK(size, 0)); //set footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); //set 

    return coalesce(bp);
}

static void *first_fit(size_t size) {
    unsigned int bp; 
    for(bp = GET_NEXT(root); bp != 0; bp = GET_NEXT(bp)) {
        if(GET_SIZE(HDRP(bp)) >= size) {
            return (void*)bp;
        }
    }
    return NULL;
}

void* coalesce(void* bp) {
    void* pre_bp = PREV_BLKP(bp); // todo implement PRE here 
    void* next_bp = NEXT_BLKP(bp);
    size_t prev_alloc = GET_ALLOC(FTRP(pre_bp));
    size_t next_alloc = GET_ALLOC(HDRP(next_bp));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc) {
        insert_after_root(bp);
        return bp;
    }else if (prev_alloc && !next_alloc) {
        // prev allocated, next free
        size += GET_SIZE(HDRP(next_bp)); // plus next alloc size 
        PUT(HDRP(bp), PACK(size, 0));// reset header
        PUT(FTRP(bp), PACK(size, 0));// reset footer 

        disconnect(next_bp);
        insert_after_root(bp);
        
    }else if(!prev_alloc && next_alloc) {
        // prev free, next allocated
        size += GET_SIZE(HDRP(pre_bp));
        // set prev footer 
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(pre_bp), PACK(size, 0));
        
        disconnect(pre_bp);
        bp = pre_bp; 

        insert_after_root(bp);
    } else {
        // both free 
        size += (GET_SIZE(HDRP(next_bp)) + GET_SIZE(FTRP(pre_bp))); // plus both size 
        PUT(HDRP(pre_bp), PACK(size, 0));// set header of prev
        PUT(FTRP(next_bp), PACK(size, 0 ));// set footer of next;

        disconnect(pre_bp);
        disconnect(next_bp);
        bp = pre_bp;

        insert_after_root(bp);
    }
    return bp;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
   size_t asize;
   size_t extend_size; 

   void* bp;

   if (size == 0) {
       return NULL;
   }
   if (size <= DSIZE) {
       asize = 2 *DSIZE;
   }else {
        asize = DSIZE * ( (size + DSIZE + (DSIZE - 1)) / DSIZE);
   }

   if ( (bp = first_fit(asize)) != NULL) {
       place(bp, asize);

       return bp;
   }
   extend_size = MAX(asize ,CHUNSIZE);

   if( (bp = extend_heap(extend_size/WSIZE)) == NULL) {
       return NULL;
   }
   place(bp, asize);
   //mm_check(); 
   return bp;
}



/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));// set header free 
    PUT(FTRP(bp), PACK(size, 0));// set header free 

    coalesce(bp); 

    //mm_check();   
}



/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    if (size == 0) {
        mm_free(ptr);
        return 0 ;
    }
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize =  GET_SIZE(HDRP(ptr));

    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}





int check_each_free_block() {
    unsigned int bp;
    unsigned int lower = (unsigned int)mem_heap_lo();
    unsigned int high = (unsigned int) mem_heap_hi();

    for(bp = (unsigned int)root; bp != 0; bp = GET_NEXT(bp)) {
        if (bp < lower || bp > high) {
            printf("invalid pointer %u", bp);

            return 0;
        }
        if(bp != (unsigned int) root && GET_ALLOC(HDRP(bp)) !=0) {
            printf("free list marked allocated");
            return 0;
        }
    }

    return 1;
}

int isfree(char * bp) {
    if ((unsigned int)(bp) == 0) {
        return 1;
    }
    return GET_ALLOC(HDRP(bp)) == 0; 
}

int find_block_in_free_list(char * bp) {
    unsigned int p;

    for(p = (unsigned int)GET_NEXT(root); p != 0; p = GET_NEXT(p)) {
        if ((unsigned int)bp == p) {
            return 1;
        }
    }
    return 0;
}
int is_valid_address(unsigned int bp){
    unsigned int low = (unsigned int)mem_heap_lo();
    unsigned int high = (unsigned int) mem_heap_hi();

    return bp <= high && bp >= low? 1:0;
}
int check_whole_blocks() {
    char* bp; 

    for(bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (isfree(bp) && (isfree(NEXT_BLKP(bp)) || isfree(PREV_BLKP(bp)))) {
            printf("continuous free blocks not being coalesced");
            return 0 ;
        }
        if(isfree(bp) && find_block_in_free_list(bp) == 0) {
            printf("free blokcs not in free list");
            return 0;
        }
        if(is_valid_address((unsigned int) bp) == 0) {
            printf("invalid address");
            return 0;   
        }
    } 

    return 1;
}

static int mm_check() {
    
    if (check_each_free_block() == 0) {
        return 0;
    }

    if(check_whole_blocks() == 0) {
        return 0; 
    }

    return 1;

}








