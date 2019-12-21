

#include "xhash.h"
#include "murmur3.h"

/**
 * http://www.memoryhole.net/kyle/2011/06/02/lf_hash.c
 * 
 * /

/** 
 * |---------------|----------|----------|----------|---------------|
 *                        
 *   0-3read count 4dowrite    5is_tail 
 *  最多支持 2^4 -1=15 个线程同时读取
 */
union xhash_key {
    uint64_t key;
    struct {
        uint64_t rd_cnt : 4;
        uint64_t write : 1;
        uint64_t tail : 1;
        uint64_t data : 58;
    };
};

#define DATA_MASK 0x3FFFFFFFFFFFFFF

static inline uint64_t make_key(uint64_t rd_cnt, uint64_t write, uint64_t tail, uint64_t data)
{
    uint64_t v1 = (0xF & rd_cnt) << 60;
    uint64_t v2 = (0x1 & write) << 59;
    uint64_t v3 = (0x1 & tail) << 58;
    uint64_t v4 = (DATA_MASK & data);
    uint64_t h = (v1 | v2 | v3 | v4);
    return h;
}

static inline uint64_t key_get_rd_cnt(uint64_t key)
{
    return (key >> 60) & 0xF;
}

static inline uint64_t key_get_write(uint64_t key)
{
    return (key >> 59) & 0x1;
}

static inline uint64_t key_get_tail(uint64_t key)
{
    return (key >> 58) & 0x1;
}

static inline uint64_t key_get_data(uint64_t key)
{
    return (key & DATA_MASK);
}

static inline void key_clear_rd_cnt(uint64_t* key)
{
    *key = ((*key) & 0xFFFFFFFFFFFFFFF);
}

static inline void key_clear_write(uint64_t* key)
{
    // only write bit=0
    *key = ((*key) & 0xF7FFFFFFFFFFFFFF);
}

static inline void key_clear_tail(uint64_t* key)
{
    *key = ((*key) & 0xFBFFFFFFFFFFFFFF);
}

static inline void key_clear_data(uint64_t* key)
{
    *key = ((*key) & 0xFC00000000000000);
}

static inline void key_set_rd_cnt(uint64_t* key, uint64_t rd_cnt)
{
    key_clear_rd_cnt(key);
    *key |= ((0xF & rd_cnt) << 60);
}

static inline void key_set_write(uint64_t* key, uint64_t write)
{
    key_clear_write(key);
    *key |= ((0x1 & write) << 59);
}

static inline void key_set_tail(uint64_t* key, uint64_t tail)
{
    key_clear_tail(key);
    *key |= ((0x1 & tail) << 58);
}

static inline void key_set_data(uint64_t* key, uint64_t data)
{
    key_clear_data(key);
    *key |= (DATA_MASK & data);
}

struct xhash_entry {
    uint64_t key;
    void* data;
};

struct xhash {
    uint64_t capacity;
    uint64_t used;
    struct xhash_entry entry[0];
};

bool atomic_cmp_set(uint64_t* dst, uint64_t src, uint64_t new_)
{
    // TODO implement this.
    *dst = new_;
    return true;
}

uint64_t atomic_get(uint64_t* dst)
{
    // TODO implement this.
    return *dst;
}

void atomic_add(uint64_t * dst)
{
    // TODO implement this.
    *dst += 1;
}

int xhash_add(struct xhash* hash, uint64_t key, void* data)
{
    uint32_t idx = 0;
    struct xhash_entry* en = 0;

    uint64_t used = atomic_get(&hash->used);
    uint64_t cap = atomic_get(&hash->capacity);
    cap = (uint64_t)((double)cap * 0.7);
    if (used >= cap) {
        // not have enough slots. 
        return -1;
    }

    uint64_t key_data = key_get_data(key);
    idx = key_data % hash->capacity;

    // wait do write = 0
    do
    {
        uint64_t tmp = atomic_get(&(hash->entry[idx].key));
        uint64_t expect = tmp;
        key_clear_write(&expect);
        uint64_t new_ = tmp;
        key_set_write(&new_, 1);
    } while (atomic_cmp_set(&(hash->entry[idx].key), expect, new_));
    
    // wait read cnt = 0
    do
    {
        uint64_t tmp = atomic_get(&(hash->entry[idx].key));
        uint64_t expect = tmp;
        key_clear_rd_cnt(&expect);
        uint64_t new_ = expect;
    } while (atomic_cmp_set(&(hash->entry[idx].key), expect, new_));

    index = key % h->capacity;
    en = &h->entry[index];
    en->key = key;
    en->data = data;
    
    atomic_add(&hash->used);
    return 0;
}

static inline uint64_t xhash_key_collision(struct xhash* h, uint64_t key, uint32_t i)
{
    return ((key % h->size) + i) % h->size;
}

struct xhash* xhash_ctor(int size)
{
    struct xhash* h = 0;
    h = calloc(1, sizeof(struct xhash) + size * sizeof(struct xhash_entry));
    h->capacity = size;
    return h;
}

void xhash_dtor(struct xhash* h)
{
    free(h);
}

void* xhash_search(struct xhash* h, uint64_t key)
{

}

bool xhash_exists(struct xhash* h, uint64_t key)
{
    uint32_t index = 0;
    struct xhash_entry* en = 0;

    if (key == 0) {
        return -1;
    }
    index = key % h->capacity;
    en = &h->entry[index];
    return en->key == key;
}

void xhash_delete(struct xhash* h, uint64_t key)
{
    uint32_t index = 0;
    struct xhash_entry* en = 0;

    if (key == 0) {
        return -1;
    }
    index = key % h->capacity;
    en = &h->entry[index];
    memset(en, 0, sizeof(*en));
    h->used -= 1;
    return 0;
}
