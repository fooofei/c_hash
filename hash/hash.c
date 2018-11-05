#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#ifdef __linux__
#include <asm-generic/errno-base.h> // EINVAL ENOMEM
#endif
//#include <rte_rwlock.h>

#include "hash.h"
//#include "stringer.h"

//
//
// fake defines
struct rte_rwlock{
    int cnt;
};
typedef struct rte_rwlock rte_rwlock_t;

static inline void rte_rwlock_init(rte_rwlock_t * arg0){
    (void)arg0;
}
//
//
//

struct entry {
    void * next;
    uint32_t cnt;
    rte_rwlock_t lock;
};
struct hash{
    bool(*keyequal)(const void * dst, const void * src);
    uint64_t (*keyhash)(void * key);
    uint32_t key_off;
    uint32_t next_off;
    uint32_t entrys_size;
    uint32_t entrys_used;
    uint32_t items_cnt;
    struct entry entrys[0];
} ;

typedef unsigned long long uint64;

static inline void __read_lock_entry(struct entry * e){
    //rte_rwlock_read_lock(&e->lock);
}
static inline void __read_unlock_entry(struct entry * e){
    //rte_rwlock_read_unlock(&e->lock);
}
static inline void __write_lock_entry(struct entry * e){
    //rte_rwlock_write_lock(&e->lock);
}
static inline void __write_unlock_entry(struct entry * e){
    //rte_rwlock_write_unlock(&e->lock);
}
static inline void  ** hash_list_next_ptr(struct hash * hash, void * item){
    return (void **)((uintptr_t)item + hash->next_off);
}
static inline void * hash_key_ptr(struct hash * hash, void * item){
    return (void*)((uintptr_t)item + hash->key_off);
}


/**
 * if found, then **entry_item !=NULL  
 */
static void inline __hash_find(struct hash * hash, void * key, struct entry ** entry, void *** entry_item){
    uint32_t entry_idx = 0;
    uint32_t i=0;
    struct entry  * en = NULL;
    void ** en_item = NULL;

    entry_idx = hash->keyhash(key) % hash->entrys_size;
    en = &(hash->entrys[entry_idx]);
    *entry = en;    
    if(en->cnt <=0){
        return;
    }
    /* when searching, lock the table, others cannot add */
    __read_lock_entry(en);
    for (i=0,en_item = &en->next; 
        en->cnt>0 && (*en_item) && i<en->cnt ;
        (en_item = hash_list_next_ptr(hash,*en_item)), i++)
    {
        if (hash->keyequal(hash_key_ptr(hash,*en_item), key)){
            break;
        }
    }
    __read_unlock_entry(en);
    *entry_item = en_item;
}
uint64_t BKDRHash(void* key){
    uint32_t seed = 131; /// 31 131 1313 13131 131313 etc..
    uint32_t hash = 0;
    uint32_t i    = 0;
    int8_t* str = (int8_t*)key;
    uint32_t len = strlen(key);

    for (i = 0; i < len; str++, i++)
    {
        hash = (hash * seed) + (*str);

    }
    return hash;
}

struct hash * hash_new(size_t size, uint32_t key_off, uint32_t next_off,
       bool (*keyequal)(const void * dst, const void * src),
        uint64_t (*keyhash)(void * key)
){
    struct hash * hash=NULL;

    hash = calloc(1, sizeof(struct hash) + size * sizeof(struct entry));
    if(NULL == hash){
        return NULL;
    }
    hash->key_off = key_off;
    hash->next_off = next_off;
    hash->entrys_size = size;
    hash->entrys_used= 0;
    hash->items_cnt = 0;
    hash->keyequal = keyequal;
    hash->keyhash = keyhash;

    uint32_t i;
    for(i=0; i<size; i+=1){
        rte_rwlock_init(&hash->entrys[i].lock);
    }

    return hash;
}

void hash_clear(struct hash * hash){
    hash->entrys_used=0;
    hash->items_cnt=0;
    
    uint32_t i;
    for(i=0; i<hash->entrys_size; i+=1){
        hash->entrys[i].next=NULL;
        hash->entrys[i].cnt=0;
        rte_rwlock_init(&hash->entrys[i].lock);
    }
}
void hash_free(struct hash * hash){
    free(hash);
}
int hash_add(struct hash * hash, void * key, void * item){
    void ** entry_item = NULL;
    struct entry * entry=NULL;

    if(!(hash && key && item)){
        return -1;
    }
    __hash_find(hash, key, &entry, &entry_item);
    if(entry_item && *entry_item){
        // found
        return -2;
    }

    // add to entry
    // entry is the first item, entry_item is the last item
    // note *hash_list_next_ptr(hash, item)=NULL;
    __write_lock_entry(entry);
    *hash_list_next_ptr(hash, item) = entry->next;
    entry->next = item;
    if(entry->cnt ==0){
        __sync_add_and_fetch(&hash->entrys_used, 1);
    }
    entry->cnt +=1;
    __write_unlock_entry(entry);
    __sync_add_and_fetch(&hash->items_cnt, 1);
    return 0;
}
void * hash_search(struct hash * hash, void * key){
    void ** entry_item = NULL;
    struct entry * entry = NULL;

    __hash_find(hash, key, &entry, &entry_item);
    if(entry_item){
        return *entry_item;
    }
    return NULL;
}

int hash_delete(struct hash * hash, void * key){
    struct entry * entry = NULL;
    void ** entry_item = NULL;

    if(!(hash && key)){
        return -EINVAL;
    }
    __hash_find(hash, key, &entry, &entry_item);

    if(!(entry_item && *entry_item)){
        // not found
        return -1;
    }
    
    __write_lock_entry(entry);
    void * tmp = *entry_item;
    *entry_item = *hash_list_next_ptr(hash, tmp);
    *hash_list_next_ptr(hash, tmp) = NULL;
    entry->cnt -= 1;
    if(entry->cnt == 0){
        __sync_sub_and_fetch(&hash->entrys_used, 1);
    }
    __write_unlock_entry(entry);
    __sync_sub_and_fetch(&hash->items_cnt, 1);
    return 0;
}
uint32_t hash_items_count(struct hash * hash){
    if(hash)
    {
        return hash->items_cnt;
    }
    return 0;
}
uint32_t hash_size(struct hash * hash){
    if(hash){
        return hash->entrys_size;
    }
    return 0;
}
void hash_iterator_set(struct hash_iterator * hash_iter, struct hash * hash){
    memset(hash_iter, 0, sizeof(*hash_iter));
    hash_iter->hash = hash;
    if(hash){
        hash_iter->entry_idx = 0;
        if(hash_iter->entry_idx<hash->entrys_size){
            struct entry * en = &hash->entrys[hash_iter->entry_idx];
            __read_lock_entry(en);
            hash_iter->entry_item = en->next;
        }
    }
}
// Return `*data` on get success 
void * hash_iterator_get(struct hash_iterator * hash_iter, void ** key, void ** data){
    if(!(hash_iter && hash_iter->hash)){
        return NULL;
    }

    struct hash * hash = hash_iter->hash;
    if(hash_iter->entry_item){
        *key = hash_key_ptr(hash, hash_iter->entry_item);
        *data = hash_iter->entry_item;
        hash_iter->entry_item = *hash_list_next_ptr(hash, hash_iter->entry_item);
        return *data;
    }
    if(hash_iter->entry_idx < hash->entrys_size){
        struct entry * en = &hash->entrys[hash_iter->entry_idx];
        __read_unlock_entry(en);
    }
    else{
        return NULL;
    }

    for(;;){
        hash_iter->entry_idx +=1;
        if(!(hash_iter->entry_idx < hash->entrys_size)){
            break;
        }
        struct entry * en = &hash->entrys[hash_iter->entry_idx];
        if(en->cnt > 0){
            __read_lock_entry(en);
            if(en ->cnt > 0 && en->next){
                hash_iter->entry_item = en->next;
                return hash_iterator_get(hash_iter, key, data);
            }
            __read_unlock_entry(en);
        }
    }
    return NULL;
}
void hash_iterator_end(struct hash_iterator * hash_iter){
    if(!(hash_iter && hash_iter->hash)){
        return;
    }
    struct hash * hash = hash_iter->hash;
    if(hash_iter->entry_idx < hash->entrys_size){
        struct entry * en = &hash->entrys[hash_iter->entry_idx];
        __read_unlock_entry(en);
    }
    memset(hash_iter, 0, sizeof(*hash_iter));
}
void hash_stat(struct hash * hash, struct stringer * string, const char * name) {
    if (!(hash && string)) {
        return;
    }
    if (!name) {
        name = "";
    }

    stringer_printf(string, "hash stat of %s ", name);
    hash_stat_brief(hash, string);

    uint32_t i;
    for (i = 0;i<hash->entrys_size;i+=1) {
        struct entry * en = &hash->entrys[i];
        if (en->cnt > 0) {
            stringer_printf(string, "[%u]%u\n", i, en->cnt);
        }
    }
}
void hash_stat_brief(struct hash  * hash,  struct stringer  * string){
    if(!(hash && string)){
        return;
    }
    stringer_printf(string, "entrys_used=%llu /entrys_size=%llu= %.2f items_cnt=%llu",
        (uint64)hash->entrys_used,
        (uint64)hash->entrys_size,
        (double)hash->entrys_used/hash->entrys_size,
        (uint64)hash->items_cnt
    );
}
