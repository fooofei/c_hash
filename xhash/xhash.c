

#include "xhash.h"
#include "murmur3.h"

/** 
 * |---------------|-----|----------|----------|---------------|
 *                        5 dowrite  4 is tail  0~3 readcount
 *   key & FFFFFFFFFFFFFFC0
 *  最多支持 2^4 -1=15 个线程同时读取
 */

static const uint64_t KeyMask = FFFFFFFFFFFFFFC0;

union xhash_key
{
    uint64_t key;
    struct 
    {
        uint64_t 
    };
}

struct xhash_entry
{
    uint64_t key;
    void * data;
};

struct xhash
{
    uint32_t capacity;
    uint32_t used;
    struct xhash_entry * entry;
}; 

static inline bool xhash_key_equal(uint64_t key, uint64_t target_key)
{
    return (key == (key & FFFFFFFFFFFFFFC0));
}

static inline uint64_t xhash_key_collision(struct xhash * h, uint64_t key, uint32_t i)
{
    return ((key % h->size) + i ) % h->size;
}

int xhash_ctor(struct xhash ** arg0, uint32_t size)
{
    struct xhash * h=0;

    h = calloc(1, sizeof(struct xhash) + size * sizeof(struct xhash_entry));
    h->capacity = size;
    *arg0 = h;
    return 0;
}

void xhash_dtor(struct xhash ** h)
{
    free(*h);
    *h=0;
}

int xhash_add(struct xhash * h, uint64_t key, void * data)
{
    uint32_t index=0;
    struct xhash_entry * en=0;
    enum{ __max_load_factor = 0.75,};

    if(h->used >= (h->capacity * __max_load_factor))
    {
        /** no have enough slots. */
        return -1;
    }
    if(key ==0)
    {
        return -1;
    }
    index = key % h->capacity;
    en = &h->entry[index];
    en->key = key;
    en->data = data;
    h->used +=1;
    return 0;
}

bool xhash_exists(struct xhash * h, uint64_t key)
{
    uint32_t index=0;
    struct xhash_entry * en=0;

    if(key ==0)
    {
        return -1;
    }
    index = key % h->capacity;
    en = &h->entry[index];
    return en->key == key;
}

void xhash_delete(struct xhash * h, uint64_t key)
{
    uint32_t index=0;
    struct xhash_entry * en=0;

    if(key ==0)
    {
        return -1;
    }
    index = key % h->capacity;
    en = &h->entry[index];
    memset(en, 0, sizeof(*en));
    h->used -= 1;
    return 0;
}

