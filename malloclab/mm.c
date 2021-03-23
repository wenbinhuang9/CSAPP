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
#define CLASS_SIZE 12

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
static char* first_class_root;


static int get_class_number(size_t words) {
    for(int i = 0; i < CLASS_SIZE; i++) {
        if (words <= (1<<(i +1)) && words > (1<<i)) {
            return i;
        }
    }

    return CLASS_SIZE - 1;
}

static char* get_class_root_by_class_no(size_t class_no) {
   return (char*)first_class_root + 2 * class_no * WSIZE; 
}
static char* get_class_root(size_t words){
    int class_num = get_class_number(words);

    return get_class_root_by_class_no(class_num);
}

static void disconnect(void *bp) {
    unsigned int pre = GET_PREV(bp);
    unsigned int next = GET_NEXT(bp);

    PUT_NEXT(pre, next);

    if(next) { PUT_PREV(next, pre);} 
}

static void insert_after_root(void *bp) {
    size_t words = GET_SIZE(HDRP(bp)) / WSIZE;
    char* root = get_class_root(words);

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
    if((heap_listp = mem_sbrk((4 + 2 * CLASS_SIZE) * WSIZE)) == (void *) -1) {
        return -1;
    }

    PUT(heap_listp, 0);  // padding alignment 
    first_class_root = heap_listp + 1 * WSIZE;
    unsigned int offset = 0;
    for( int i = 0; i < CLASS_SIZE; i++) {
        offset += WSIZE;
        PUT(heap_listp + offset, 0);  //each class next
        offset +=WSIZE;
        PUT(heap_listp + offset, 0);  //each class prev
    }
    offset += WSIZE; 
    PUT(heap_listp + offset, PACK(DSIZE, 1));
    offset += WSIZE;
    char* correst_heap_listp = heap_listp + offset;
    PUT(heap_listp + offset, PACK(DSIZE, 1));
    offset += WSIZE;
    PUT(heap_listp + offset, PACK(0, 1));

    heap_listp = correst_heap_listp;

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
    unsigned int class_no = get_class_number(size / WSIZE);

    for (int num = class_no; num < CLASS_SIZE; num++) {
        char* root = get_class_root_by_class_no(num);
        unsigned int bp; 

        for(bp = GET_NEXT(root); bp != 0; bp = GET_NEXT(bp)) {
            if(GET_SIZE(HDRP(bp)) >= size) {
                return (void*)bp;
            }
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


static size_t get_adjust_size(size_t size) {
   size_t asize;
   if (size <= DSIZE) {
       asize = 2 *DSIZE;
   }else {
        asize = DSIZE * ( (size + DSIZE + (DSIZE - 1)) / DSIZE);
   }
   return asize;
}

void realloc_place(void *bp, size_t size) {
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

        insert_after_root(next_bp);
    }
    return; 
}
int is_prev_alloc(void* bp) {
    void* pre_bp = PREV_BLKP(bp); 
    size_t prev_alloc = GET_ALLOC(FTRP(pre_bp));

    return prev_alloc;
}
size_t get_coalesce_size(void* bp) {
    void* pre_bp = PREV_BLKP(bp); 
    void* next_bp = NEXT_BLKP(bp); 
    size_t prev_alloc = GET_ALLOC(FTRP(pre_bp));
    size_t next_alloc = GET_ALLOC(HDRP(next_bp));  
    size_t total_size =  GET_SIZE(HDRP(bp));

    if(!prev_alloc) {
        total_size += GET_SIZE(HDRP(pre_bp));  
    }
    if(!next_alloc) {
        total_size += GET_SIZE(HDRP(next_bp));  
    }
    return total_size; 
}

void* realloc_coalesce(void* bp, size_t newsize) {
    void* pre_bp = PREV_BLKP(bp); 
    void* next_bp = NEXT_BLKP(bp);
    size_t prev_alloc = GET_ALLOC(FTRP(pre_bp));
    size_t next_alloc = GET_ALLOC(HDRP(next_bp));
    size_t next_size = GET_SIZE(HDRP(next_bp));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc) {
        return bp;
    }else if ((prev_alloc && !next_alloc) || ((size + next_size) >= newsize && !next_alloc)) {
        // prev allocated, next free
        size += GET_SIZE(HDRP(next_bp)); // plus next alloc size 
        PUT(HDRP(bp), PACK(size, 0));// reset header
        PUT(FTRP(bp), PACK(size, 0));// reset footer 

        disconnect(next_bp);
        return bp;
    }else if(!prev_alloc && next_alloc) {
        // prev free, next allocated
        size += GET_SIZE(HDRP(pre_bp));
        // set prev footer 
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(pre_bp), PACK(size, 0));
        
        disconnect(pre_bp);
        bp = pre_bp; 

        return bp;
    } else {
        // both free
        size_t prev_size = GET_SIZE(HDRP(pre_bp));
        size_t next_size = GET_SIZE(HDRP(next_bp)) ;  
        printf("realloc_coalesce both free prev size is %d, next size is %d", prev_size, next_size); 
        size += (prev_size +  next_size); // plus both size 
        PUT(HDRP(pre_bp), PACK(size, 0));// set header of prev
        PUT(FTRP(next_bp), PACK(size, 0));// set footer of next;

        disconnect(pre_bp);
        disconnect(next_bp);
        bp = pre_bp;
    }
    return bp;
}

void *mm_naive_realloc(void *ptr, size_t size)
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

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    if (size == 0) {
        mm_free(ptr);
        return 0;
    }
    void *oldptr = ptr;
    void *newptr;
    void* pre_bp = PREV_BLKP(ptr); 
    void* next_bp = NEXT_BLKP(ptr);
    size_t prev_size = GET_SIZE(FTRP(pre_bp));
    size_t next_size = GET_SIZE(FTRP(next_bp));
    size_t prev_alloc = GET_ALLOC(FTRP(pre_bp));
    size_t next_alloc = GET_ALLOC(FTRP(next_bp));

    size_t newsize = get_adjust_size(size);
    size_t oldsize = GET_SIZE(HDRP(ptr));
    size_t is_prev_free = ! is_prev_alloc(ptr);
    
    if(newsize <= oldsize) {
     //   printf("realloc in place\n");
        realloc_place(ptr, newsize);
    //    mm_check();
        return ptr;
    }

    // newsize > oldsize
    // try to coalesce the previous and next free block
    // todo debug here 
    // stop in here todo how to debug here ? 
    // forget this one , debug in hte other day. 
    size_t coalesce_size = get_coalesce_size(ptr);
    if (coalesce_size >= newsize) {
        
       // printf("realloc by coalesce newsize is %u, old size is%u, prev_size is %u, next size is %u, pre alloc is %u, next alloc is %u\n", newsize, oldsize, prev_size, next_size, prev_alloc, next_alloc);
        void* newbp = realloc_coalesce(ptr, newsize);
      //  printf("ptr is %u, newbp is %u\n", (unsigned int) ptr, (unsigned int) newbp);
     //   printf("coalesced size is %d alloc state is %d\n", GET_SIZE(HDRP(newbp)), GET_ALLOC(HDRP(newbp)));
        if ((unsigned int) oldptr != (unsigned int) newbp){
            //printf("get into copy mem\n");
            if ( size < oldsize ) {
                oldsize = size;
            }
            char origin = *(char*)ptr;
            memmove((char *)newbp, (char *)ptr, oldsize);
        }

        realloc_place(newbp, newsize);
    //    mm_check();
        return newbp; 
    }
    // printf("realloc by naive way\n");
    
    newptr =  mm_naive_realloc(ptr, size);
   // mm_check();
    return newptr;
}


int check_each_free_block() {
    unsigned int bp;
    unsigned int lower = (unsigned int)mem_heap_lo();
    unsigned int high = (unsigned int) mem_heap_hi();
    for(int num = 0; num < CLASS_SIZE; num ++) {
        void* first_class_root = get_class_root_by_class_no(num);

        for(bp = (unsigned int)first_class_root; bp != 0; bp = GET_NEXT(bp)) {
            if (bp < lower || bp > high) {
                printf("invalid pointer %u", bp);

                return 0;
            }
            if(bp != (unsigned int) first_class_root && GET_ALLOC(HDRP(bp)) !=0) {
                printf("free list marked allocated");
                return 0;
            }
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
    size_t size = GET_SIZE(HDRP(bp));
    void* root = get_class_root(size/WSIZE);

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
    printf("start mm_check\n");
    if (check_each_free_block() == 0) {
        return 0;
    }

    if(check_whole_blocks() == 0) {
        return 0; 
    }

    return 1;

}








