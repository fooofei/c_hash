
#pragma once

#include <stdbool.h>
#include <stdint.h>

struct xhash;

struct xhash* xhash_ctor(int size);
// key 会丢失一部分位
int xhash_add(struct xhash* h, uint64_t key, void* data);
void* xhash_search(struct xhash* h, uint64_t key);
void xhash_delete(struct xhash* h, uint64_t key);
void xhash_dtor(struct xhash* h);
