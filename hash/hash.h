/**
 *  A less malloc hash table.
 *  Not support hash table resize at running.
 * 
 */
#pragma once


#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/types.h>
#include <stdbool.h>


struct hash;
struct stringer;

static inline void stringer_printf(struct stringer * arg0, const char * fmt, ...){
    (void)arg0;
    (void)fmt;
}

#ifndef offset_of
#define offset_of(str, member)	((char *)(&((str *)0)->member) - (char *)0)
#endif

struct hash * hash_new(size_t size, uint32_t key_off, uint32_t next_off,
       bool (*keyequal)(const void * dst, const void * src),
        uint64_t (*keyhash)(void * key)
    );
void hash_clear(struct hash * hash);
void hash_free(struct hash * hash);
/* if exists, do nothing. */
int hash_add(struct hash * hash, void * key, void * data);
void * hash_search(struct hash * hash, void * key);
int hash_delete(struct hash * hash, void *key);
uint64_t BKDRHash(void* key);
uint32_t hash_items_count(struct hash * hash);
uint32_t hash_size(struct hash * hash);
void hash_stat(struct hash * hash, struct stringer * string, const char * name);
void hash_stat_brief(struct hash * hash, struct  stringer * string);


struct hash_iterator {
    struct hash * hash;
    uint32_t entry_idx;
    void * entry_item;
};

void hash_iterator_set(struct hash_iterator * , struct hash *);
// Return `*data` on get success 
void * hash_iterator_get(struct hash_iterator *, void ** key, void ** data);
void hash_iterator_end(struct hash_iterator *);


