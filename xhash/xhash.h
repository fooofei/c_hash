
#pragma once

#include <stdint.h>
#include <stdbool.h>

struct xhash;

int xhash_ctor(struct xhash ** h);
int xhash_add(struct xhash * h, uint64_t key, void * data);
bool xhash_exists(struct xhash * h, uint64_t key);
void xhash_delete(struct xhash * h, uint64_t key);
void xhash_dtor(struct xhash ** h);