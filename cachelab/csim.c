#include "cachelab.h"
#include <semaphore.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include "assert.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>


#define  UNVALID 1;
#define  VALID 0;
#define MAX_SIZE 256;

typedef struct Line {
    int valid; 
    unsigned long tag; 
    unsigned long time_stamp;  // used for LRU 
} Line; 

typedef struct {
    int size; 
    int capacity; 
    Line* lines; 
} Set;


typedef struct {
    int size;
    Set* sets; 
} Cache; 

sem_t mutex; 
Cache* cache;

int v; 

int VV = 0 ; 

void init_cache( unsigned int B, unsigned int S, unsigned int E );

void write_cache(unsigned long address, unsigned int B, unsigned int S, unsigned int E,
    unsigned int* hit_cnt, unsigned int* miss_cnt, unsigned int* evict_cnt);
Line* read_cache(unsigned long address,  unsigned int B, unsigned  int S, unsigned int E,
    unsigned int* hit_cnt, unsigned int* miss_cnt, unsigned int* evict_cnt); 

unsigned int find_set_idx(unsigned long address, unsigned int B, unsigned int S, unsigned int E);

unsigned int get_tag(unsigned long address, unsigned int B, unsigned S, unsigned int E);
Set* find_set(unsigned long address, unsigned int B, unsigned int S, unsigned int E);

Line*  find_evicted_line(Set* set);

void evict(Set* set);

Line* find_avail_line(Set* set); 

Line* hit(unsigned long address, unsigned int B, unsigned int S, unsigned int E);

void fill_line(Line* line, unsigned long address, unsigned int B, unsigned int S, unsigned int E);

void set_line_ts(Line* line, unsigned long timestamp); 

Line* find_by_tag(Set* set, unsigned long tag);


Line* find_evicted_line(Set* set) {
    Line* evict_line = set->lines;

    for(int i = 0; i < set->capacity; i++) {
        Line cur = set->lines[i];
        assert(cur.valid == 1);

        if (cur.time_stamp > evict_line ->time_stamp) {
            evict_line = set->lines + i;

            return evict_line; 
        }
    }

    return evict_line; 
}

Line* find_by_tag(Set* set, unsigned long tag) {
    
    for(int i = 0; i < set->capacity; i++) {
        if( set->lines[i].valid == 1 && set->lines[i].tag == tag) {
            return set->lines + i ;
        }
    }
    return NULL;
}


Line* hit(unsigned long address, unsigned int B, unsigned int S, unsigned int E) {
    if (v) {
        printf("start hit\n");
    }
    unsigned long tag = get_tag(address, B, S, E); 
    
    if(v) {
        printf("tag is %lu\n", tag);
    }
    Set* set = find_set(address, B, S, E);

    Line* line =  find_by_tag(set, tag); 

    if(v) {
        printf("end hit \n");
    }
    return line ;
}

unsigned int get_tag(unsigned long address, unsigned int B, unsigned S, unsigned int E) {
    return (address >> (S + B));
}


void fill_line(Line* line, unsigned long address, unsigned int B, unsigned int S, unsigned int E) {
    assert(line->valid == 0);
    unsigned long curtag = get_tag(address, B, S, E);
    // assert(line->tag != curtag);
    line->valid = 1; 
    line->tag = curtag;

    line->time_stamp = (unsigned long)time(NULL); 

    return;  
}
Line* find_avail_line(Set* set) {
    Line* line = NULL;
    for (int i = 0; i < set->capacity; i ++) {
        if (set->lines[i].valid == 0) {
            return set->lines + i; 
        }
    }

    assert (line != NULL);
    return line;
}

void evict(Set* set) {
    assert(set->size >= set->capacity);

    Line* line = find_evicted_line(set); 

    line -> valid = 0 ;

    return ;    
}
unsigned int find_set_idx(unsigned long address, 
    unsigned int B, unsigned int S, unsigned int E) {
        
    unsigned long temp = (address >> B); 

    return temp ^ ((temp >> S) << S) ;  
}

Set* find_set(unsigned long address, 
    unsigned int B, unsigned int S, unsigned int E) {

    if(v) {
        printf("find_set start \n");
    }
    unsigned int set_idx = find_set_idx(address, B, S, E);
    assert(set_idx < cache->size);

    if(v) {
        printf("find_set end\n");
    }
    return cache->sets + set_idx;

}
void set_line_ts(Line* line, unsigned long timestamp) {
    line->time_stamp = timestamp;

    return ;
}
void unix_error(char *msg) /* Unix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

void P(sem_t *sem) 
{
    if (sem_wait(sem) < 0)
	unix_error("P error");
}

void V(sem_t *sem) 
{
    if (sem_post(sem) < 0)
	unix_error("V error");
}

// do I really need write_cache and read_cache ? 
// write cache can be done by reading cache 
void modify_cache( unsigned long address, unsigned int B, unsigned S, unsigned int E,
    unsigned int* hit_cnt, unsigned int* miss_cnt, unsigned int* evict_cnt) {
    read_cache(address, B, S, E, hit_cnt, miss_cnt, evict_cnt);

    write_cache(address, B, S, E, hit_cnt, miss_cnt , evict_cnt);
}

void write_cache(unsigned long address, 
    unsigned int B, unsigned int S, unsigned int E, 
    unsigned int* hit_cnt, unsigned int* miss_cnt, unsigned int* evict_cnt) {
    read_cache(address, B, S, E, hit_cnt, miss_cnt, evict_cnt);

    return ;
}


Line* read_cache(unsigned long address, 
    unsigned int B, unsigned  int S, unsigned int E, 
    unsigned int* hit_cnt, unsigned int* miss_cnt, unsigned int* evict_cnt) {
    
    if(v) {
        printf("address: %lu\n", address);
    }

    Line* line = hit(address, B, S, E);

    if(line != NULL) {
        // in cache
        if(VV) {
            printf(" hit");
        }
        *hit_cnt += 1; 
        unsigned long ts =(unsigned long )time(NULL); 
        set_line_ts(line, ts);
        return line; 
    }else {
        *miss_cnt += 1; 
        if(VV) {
            printf(" miss");

        }
    }
    // not in cache 
    Set* set = find_set(address, B, S, E);

    P(&mutex);
    if(set -> size >= set -> capacity) {
        *evict_cnt += 1; 
        evict(set);
        set->size -= 1; 
        if(VV) {
            printf(" eviction");
        }
    }

    line = find_avail_line( set );

    fill_line(line, address, B, S, E); 
    set->size += 1; 

    V(&mutex);
    return line; 

}

void init_cache(unsigned int B, unsigned int S, unsigned int E) {
    sem_init(&mutex, 0, 1);

    cache = (Cache* ) malloc(sizeof(Cache));

    unsigned int cache_size = (0x1 << S);
    cache -> size = cache_size; 

    Set* sets = (Set* )malloc( cache_size * sizeof(Set)); 

    for (int i = 0; i < cache_size; i++) {
        sets[i].capacity = E;
        sets[i].size = 0 ;

        Line* lines = (Line*) malloc(E * sizeof(Line));

        for( int j = 0; j < E; j++) {
            lines[j].valid = 0;
        }
        sets[i].lines = lines;
    }

    cache->sets = sets;
    
    return; 
}

void printHelp()
{
    printf("./csim: Missing required command line argument\n");
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("-h         Print this help message.\n");
    printf("-v         Optional verbose flag.\n");
    printf("-s <num>   Number of set index bits.\n");
    printf("-E <num>   Number of lines per set.\n");
    printf("-b <num>   Number of block offset bits.\n");
    printf("-t <file>  Trace file.\n\n");
    printf("Examples:\n");
    printf("linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

int main(int argc, char *argv[])
{
    unsigned int hit_cnt; 
    unsigned miss_cnt; 
    unsigned int evict_cnt; 
    char optch;
    char *addr_buf;
    char *cmd_buf = (char *)malloc(sizeof(char) * 256);
    char *cmd_temp_buf = (char *)malloc(sizeof(char) * 256);
    int help = 0, verbose = 0;
    FILE *input_file;
    unsigned int s_num, e_num, b_num; 

    verbose = 0;
  
    
    while ((optch = getopt(argc, argv, "hvVs:E:b:t:")) != -1)
    {
        switch (optch)
        {
        case 'h':
            help = 1;
            break;
        case 'v':
            VV = 1;
            break;
        case 'V':
            verbose = 1;
            v = verbose;
            break;
        case 's':
            s_num = atoi(optarg);
            break;
        case 'E':
            e_num = atoi(optarg);
            break;
        case 'b':
            b_num = atoi(optarg);
        case 't':
            input_file = fopen(optarg, "r");
        }
    }
    if(verbose) {
        printf("B:%u, S:%u, E:%u\n", b_num, s_num, e_num);
    }

    if(verbose) {
        printf("start init_cache\n");
    }
    init_cache(b_num, s_num, e_num); 
    
    if(verbose) {
        printf("end of init_cache\n");
    }
    
    if (s_num < 1 || e_num < 1 || b_num < 1 || help || input_file == NULL)
    {
        printHelp();
        return 0;
    }
    //main loop.
    while (fgets(cmd_temp_buf, 256, input_file) != NULL)
    {
        strncpy(cmd_buf, cmd_temp_buf, strlen(cmd_temp_buf) - 1);
        cmd_buf[strlen(cmd_temp_buf) - 1] = '\0';
        if (cmd_buf[0] == ' ')
        {
             if(VV) {
                printf("%s", cmd_buf + 1 );
            }
            addr_buf = strtok(&cmd_buf[3], ",");
            unsigned long addr = (unsigned long )strtol(addr_buf, &cmd_temp_buf, 16);

            switch (cmd_buf[1])
            {
            case 'S':
                if(verbose) {
                    printf("S, %s\n", addr_buf);

                    printf("start of write cache\n");
                }
                write_cache(addr, b_num, s_num, e_num, &hit_cnt, &miss_cnt, &evict_cnt);
                if(verbose) {
                    printf("end of write cache\n");
                }
                break;
            case 'L':   
                if(verbose) {
                    printf("L, %s\n", addr_buf);
                    printf("start of  read cache\n");
                }
                read_cache(addr, b_num, s_num, e_num, &hit_cnt, &miss_cnt, &evict_cnt);
                
                if(verbose) {
                    printf("end of read cache\n");
                }
                break;
            case 'M':
                if(verbose) {
                    printf("M, %s\n", addr_buf);
                    printf("start of modify cache\n");
                }
                modify_cache(addr, b_num, s_num, e_num, &hit_cnt, &miss_cnt, &evict_cnt);
                if(verbose) {
                    printf("end of modify cache\n");
                }
                break;
            }
            if(VV) {
                printf("\n");
            }
 

        }
    }

    printSummary(hit_cnt, miss_cnt, evict_cnt);
    return 0;
}
