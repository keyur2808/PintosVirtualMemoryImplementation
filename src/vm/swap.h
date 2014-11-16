#ifndef VM_SWAP_H
#define VM_SWAP_H 1

#include <stdbool.h>
#include <stdio.h>

struct page;
void swap_init (void);
void swap_in (struct page *,size_t);
bool swap_out (struct page *);

#endif /* vm/swap.h */
