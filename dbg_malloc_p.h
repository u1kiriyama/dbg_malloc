#ifndef _DBG_MALLOC_P_H_INCLUDED_
#define _DBG_MALLOC_P_H_INCLUDED_

#define DBG_MALLOC_NOCHANGE

#include <time.h>
#include "dbg_malloc.h"

struct memory_count {
    const char * file;
    int line;
    int number;
    unsigned int alloc_count;
    unsigned int free_count;
    int size;
    unsigned int alloc_size;
    unsigned int free_size;
    struct memory_block * block_list;
    struct free_count * free_list;
    struct memory_count * prev;
    struct memory_count * next;
};

struct memory_block {
    int id;
    int size;
    time_t t;
    struct memory_count * mcp;
    struct memory_block * prev;
    struct memory_block * next;
    struct memory_block * hash_prev;
    struct memory_block * hash_next;
    void * data;
};

struct free_count {
    const char * file;
    int line;
    unsigned int count;
    unsigned int size;
    struct free_count * next;
};

extern unsigned int dbg_malloc_count;
extern unsigned int dbg_free_count;
extern unsigned int dbg_malloc_number;
extern unsigned int dbg_malloc_size;
extern struct memory_count * dbg_memory_count_head;
extern struct memory_count * dbg_memory_count_tail;

#define DBG_HASH_NUM 997
#define DBG_HASH_FUNC(p) \
(( \
    (((unsigned long int)(p) >> 4) & 0xf) + \
    ((unsigned long int)(p) >> 8)\
    ) % DBG_HASH_NUM)

extern struct memory_block * dbg_memory_block_hash[DBG_HASH_NUM];

void dbg_init();

#endif