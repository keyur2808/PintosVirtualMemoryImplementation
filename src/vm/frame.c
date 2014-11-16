#include <stdio.h>
#include "devices/timer.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "vm/frame.h"
#include "vm/page.h"
#include "userprog/pagedir.h"
#include "vm/swap.h"
#include <string.h>
/* Initialize the frame manager. */

void
frame_init (void)
{
lst=(struct list*)malloc(sizeof(struct list));
list_init(lst);
k=list_begin(lst);
lock_init(&llock);
}

void *
frame_evict(void)
{
bool accessed;
bool flag=false;
void *kptr;
int j=0;
struct frame *frame;
for (j=0;j<2;j++)
{
if (k==list_end(lst))k=list_begin(lst);
 while(k!=list_end(lst))
 {
 frame=list_entry(k, struct frame, el);
 accessed=page_accessed_recently(frame->pageptr);
  if (accessed)
  {
  pagedir_set_accessed (frame->t->pagedir, frame->paddr, false);
  }
  else
  {
  lock_acquire(&llock);
  list_remove(k);
  lock_release(&llock);
  frame_lock(frame->pageptr);
  kptr=frame->base;
  page_deallocate(frame->pageptr->addr);
  flag=true; 
  break; 
  } 
  k=list_next(k);
 }
 if(flag==true)break;
}
return (kptr);
}


/* Tries to allocate and lock a frame for PAGE.
   Returns the frame if successful, false on failure. */

struct frame *
frame_alloc_and_lock (struct page *page)
{
void *kpage=palloc_get_page(PAL_USER|PAL_ZERO);
if (kpage==NULL)
{
kpage=frame_evict();
}
memset(kpage,0,PGSIZE);
struct frame *frame=(struct frame*)malloc(sizeof(struct frame));
frame->base=kpage;
frame->paddr=page->addr;
frame->pageptr=page;
frame->t=thread_current();
lock_acquire(&llock);
list_push_back(lst,&frame->el);
lock_release(&llock);
return frame;
}

/* Locks P's frame into memory, if it has one.
   Upon return, p->frame will not change until P is unlocked. */

void
frame_lock (struct page *p)
{
 struct frame *frame = p->frame;
  if (frame != NULL)
    {
      lock_acquire (&frame->frlock);

    }

}

/* Releases frame F for use by another page.
   F must be locked for use by the current process.
   Any data in F is lost. */

void
frame_free (struct frame *f)
{

if (lock_held_by_current_thread(&f->frlock))
{
free(f);
}

}


/* Unlocks frame F, allowing it to be evicted.
   F must be locked for use by the current process. */

void
frame_unlock (struct frame *f)
{

if (lock_held_by_current_thread(&f->frlock))
lock_release(&f->frlock);

}


