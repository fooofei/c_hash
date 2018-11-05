

//#include <mcheck.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <stdarg.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>



#include "hash.h"

#define EXPECT(expr) \
    do { \
    if(!(expr)) \
        { \
        fprintf(stderr, "unexpect %s  (%s:%d)\n",#expr, __FILE__, __LINE__); \
        fflush(stderr);\
        } \
    } while (0)

struct hash_key {
    uint32_t key1;
    uint32_t key2;
};
struct hash_item {
    struct hash_key key;
    struct hash_item * hash_next;
    uint32_t data;
};

static bool equal(const void * dst0, const void * src0) {
    const struct hash_key * dst = dst0;
    const struct hash_key * src = src0;

    return (dst->key1 == src->key1 &&
        dst->key2 == src->key2);
}
uint64_t key_hash(void * key0) {
    struct hash_key * key = key0;
    uint64_t r;
    r = key->key1;
    r = r << 32;
    r += key->key2;
    return r;
}

static uint32_t g_node_cnt = 0;

struct hash_item * item_new(uint32_t i) {
    struct hash_item * item = calloc(1, sizeof(struct hash_item));
    g_node_cnt += 1;
    item->key.key1 = 2 * i;
    item->key.key2 = 2 * i + 1;
    item->data = i;
    return item;
}

void test_hash_basic() {
    struct hash_item * all_item[0x200] = { 0 };
    uint32_t i;

    struct hash * hash = hash_new(10, offset_of(struct hash_item, key), offset_of(struct hash_item, hash_next),
        equal, key_hash);

    for (i = 1; i < 6; i += 1) {
        struct hash_item * item = item_new(i);
        hash_add(hash, &item->key, item);
        printf("hash_add key=%p item=%p\n", &item->key, item);
    }

    {
        struct hash_key key = { 0 };

        key.key1 = 2 * 1;
        key.key2 = 2 * 1 + 1;
        EXPECT(hash_search(hash, &key) != NULL);

        key.key1 = 2 * 2;
        key.key2 = 2 * 2 + 1;
        EXPECT(hash_search(hash, &key) != NULL);

        key.key1 = 2 * 5;
        key.key2 = 2 * 5 + 1;
        EXPECT(hash_search(hash, &key) != NULL);

        key.key1 = 2 * 7;
        key.key2 = 2 * 7 + 1;
        EXPECT(hash_search(hash, &key) == NULL);
    }
    {
        struct hash_key key = { 0 };

        key.key1 = 2 * 1;
        key.key2 = 2 * 1 + 1;
        struct hash_item * item = hash_search(hash, &key);
        EXPECT(item != NULL);
        EXPECT( 0 == hash_delete(hash, &key));
        EXPECT(0 != hash_delete(hash, &key));
        free(item);
        g_node_cnt -=1;
    }

    struct hash_iterator hash_it = { 0 };

    hash_iterator_set(&hash_it, hash);
    void * key;
    void * data;
    i = 0;
    for (; (hash_iterator_get(&hash_it, &key, &data));) {
        all_item[i++]=(struct hash_item *)data;
        printf("hash iter key=%p item=%p\n", key, data);
    }

    hash_iterator_end(&hash_it);

    hash_free(hash);


    for (i = 0; i < 0x200; i += 1) {
        if (all_item[i]) {
            free(all_item[i]);
            g_node_cnt -= 1;
        }
    }

}

int main(){
    //mtrace();
    test_hash_basic();

    return 0;
}
