#include "vm/swap.h"
#include <bitmap.h>
#include <debug.h>
#include <stdio.h>
#include "devices/block.h"
#include "vm/frame.h"
#include "vm/page.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

/* The swap device. */
static struct block *dev_swap;

/* Used swap pages. */
static struct bitmap *swap_bitmap;

/* Protects swap_bitmap. */
static struct lock swap_lock;

/* Number of sectors per page. */
#define PAGE_SECTORS (PGSIZE / BLOCK_SECTOR_SIZE)


/* Sets up swap. */
void
swap_init (void) 
{
dev_swap=block_get_role(BLOCK_SWAP);
block_sector_t dev_swap_size=block_size(dev_swap);
if (dev_swap!=NULL)
{
swap_bitmap=bitmap_create(dev_swap_size/BLOCK_SECTOR_SIZE);
}
lock_init(&swap_lock);
}

/* Swaps in page P, which must have a locked frame
   (and be swapped out). */
void
swap_in (struct page *p,size_t idx) 
{
if (lock_held_by_current_thread(&p->frame->frlock)==false)
{
lock_try_acquire(&p->frame->frlock);
}
int k=0;
while(k<PAGE_SECTORS)
{
block_read(dev_swap,idx,p->frame->base+(k*512));
idx=idx+1;
k++;
}
bitmap_set_multiple (swap_bitmap,p->idx_,PAGE_SECTORS, false);
p->idx_=0;
if (lock_held_by_current_thread(&p->frame->frlock))
lock_release(&p->frame->frlock);

}

/* Swaps out page P, which must have a locked frame. */
//Track sector no idx from where data is stored and put in page entry
bool
swap_out (struct page *p) 
{
if (lock_held_by_current_thread(&p->frame->frlock)==false)
{
bool b=lock_try_acquire(&p->frame->frlock);
if (!b)return false;
}
else
{
lock_acquire(&swap_lock);
size_t idx=bitmap_scan_and_flip(swap_bitmap,0,PAGE_SECTORS,false);
p->idx_= idx;
int k=0;
while(k<PAGE_SECTORS)
{
block_write(dev_swap,idx,p->frame->base+(k*512));
idx=idx+1;
k++;
}
p->pinfo->loc=SWAP;
lock_release(&p->frame->frlock);
return true;
}
}
