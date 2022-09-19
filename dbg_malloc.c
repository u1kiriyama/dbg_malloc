#include "dbg_malloc_p.h"

unsigned int dbg_malloc_count = 0;
unsigned int dbg_free_count = 0;
unsigned int dbg_malloc_number = 0;
unsigned int dbg_malloc_size = 0;

struct memory_count * dbg_memory_count_head = NULL;
struct memory_count * dbg_memory_count_tail = NULL;
struct memory_block * dbg_memory_block_hash[DBG_HASH_NUM];

#define LOG(m, f, l) \
    fprintf(stderr, \
    "%s:line%d:%s() : " m " (FILE:%s, LINE:%d)\n", \
    __FILE__, __LINE__, __FUNCTION__, f, l)

void dbg_init()
{
    static int initialized = 0;
    if (initialized) return;
    dbg_memory_count_head = NULL;
    dbg_memory_count_tail = NULL;
    memset(dbg_memory_block_hash, 0, sizeof(dbg_memory_block_hash));
    initialized = 1;
}

void * dbg_malloc(size_t size, const char * file, int line)
{
    void * p = NULL;
    static int id = 0;
    struct memory_block * mbp = NULL;
    struct memory_count * mcp;
    struct memory_block ** hash;

    dbg_init();

    if (!size) {
        LOG("size of zero!", file, line);
        goto err;
    }

    for (mcp = dbg_memory_count_head; mcp; mcp = mcp->next) {
        if ((mcp->line == line) && (mcp->file == file)) break;
    }

    if (mcp == NULL) {
        mcp = malloc(sizeof(struct memory_count));
        if (mcp == NULL) {
            LOG("lack of memory for mcp!", file, line);
            goto err;
        }

        memset(mcp, 0, sizeof(*mcp));
        mcp->file = file;
        mcp->line = line;

        mcp->prev = dbg_memory_count_tail;
        mcp->next = NULL;
        if (dbg_memory_count_tail)
            dbg_memory_count_tail->next = mcp;
        else
            dbg_memory_count_head = mcp;
        dbg_memory_count_tail = mcp;
    }

    mbp = malloc(sizeof(struct memory_block));
    p = malloc(size);
    if ((mbp = NULL) || (p == NULL)) {
        LOG("lack of memory!", file, line);
        goto err;
    }

    mbp->id = id++;
    mbp->size = size;
    mbp->t = time(NULL);
    mbp->mcp = mcp;
    mbp->data = p;

    mbp->prev = NULL;
    mbp->next = mcp->block_list;
    if (mcp->block_list)
        mcp->block_list->prev = mbp;
    mcp->block_list = mbp;

    hash = &(dbg_memory_block_hash[DBG_HASH_FUNC(p)]);
    mbp->hash_prev = NULL;
    mbp->hash_next = *hash;
    if (*hash)
        (*hash)->hash_prev = mbp;
    *hash = mbp;

    mcp->number++;
    mcp->alloc_count++;
    mcp->size += size;
    mcp->alloc_size += size;
    dbg_malloc_count++;
    dbg_malloc_number++;
    dbg_malloc_size +=size;

    return (p);

err:
    if (mbp) free(mbp);
    if (p) free(p);
    return (NULL);
}

void dbg_free(void * ptr, const char * file, int line)
{
    struct memory_block * mbp;
    struct memory_block ** hash;
    struct memory_count * mcp;
    struct free_count * fcp;

    dbg_init();

    if (!ptr) {
        LOG("NULL pointer!", file, line);
        goto err;
    }

    hash = &(dbg_memory_block_hash[DBG_HASH_FUNC(ptr)]);
    for (mbp = *hash; mbp; mbp = mbp->hash_next) {
        if (mbp->data == ptr) break;
    }

    if (!mbp) {
        LOG("not found!", file, line);
        free(ptr);
        return;
    }

    mcp = mbp->mcp;
    if (mcp == NULL) {
        LOG("invalid pointer!", file, line);
        goto err;
    }

    for (fcp = mcp->free_list; fcp; fcp = fcp->next) {
        if ((fcp->line == line) && (fcp->file == file)) break;
    }

    if (fcp == NULL) {
        fcp = malloc(sizeof(struct free_count));
        if (fcp == NULL) {
            LOG("lack of memory for fcp", file, line);
            goto err;
        }

        memset(fcp, 0, sizeof(*fcp));
        fcp->file = file;
        fcp->line = line;

        fcp->next = mcp->free_list;
        mcp->free_list = fcp;
    }

    if (mbp->prev)
        mbp->prev->next = mbp->next;
    else
        mcp->block_list = mbp->next;
    if (mbp->next)
        mbp->next->prev = mbp->next;

    if (mbp->hash_prev)
        mbp->hash_prev->hash_next = mbp->hash_next;
    else
        *hash = mbp->hash_next;
    if (mbp->hash_next)
        mbp->hash_next->hash_prev = mbp->hash_prev;
    
    mcp->number--;
    mcp->free_count++;
    mcp->size -= mbp->size;
    mcp->free_size += mbp->size;
    fcp->count++;
    fcp->size += mbp->size;
    dbg_free_count++;
    dbg_malloc_number--;
    dbg_malloc_size -= mbp->size;

    free(mbp);
    free(ptr);

err:
    return;
}