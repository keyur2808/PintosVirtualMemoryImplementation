#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <stdbool.h>
#include "threads/synch.h"
#include "page.h"

struct list *lst;
struct list_elem *k;
struct lock llock;
/* A physical frame. */
struct frame
{
    void *base;                 /* Kernel virtual base address. */
    void *paddr;                /* Mapped process page, if any. */
    struct page *pageptr;
    struct thread *t;
    struct lock frlock;
    struct list_elem el;
    

    /* ...............          other struct members as necessary */
};

void frame_init (void);
void *frame_evict(void);
struct frame *frame_alloc_and_lock (struct page *);
void frame_lock (struct page *);
void frame_free (struct frame *);
void frame_unlock (struct frame *);

#endif /* vm/frame.h */
