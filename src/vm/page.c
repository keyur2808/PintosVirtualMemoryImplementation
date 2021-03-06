#include "vm/page.h"
#include <stdio.h>
#include <string.h>
#include "vm/frame.h"
#include "vm/swap.h"
#include "filesys/file.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"

/* Maximum size of process stack, in bytes. */
#define STACK_MAX (1024 * 1024)

struct page *
page_lookup (void *address)
{
struct thread *t=thread_current();
struct page p;
struct hash_elem *e;
p.addr = address;
e = hash_find (&t->pages, &p.elem);
if (e != NULL)
return (hash_entry (e, struct page, elem) );
else
return NULL;
}

/* Destroys a page, which must be in the current process's
   page table.  Used as a callback for hash_destroy(). */

static void
destroy_page (struct hash_elem *p_, void *aux UNUSED)
{
  struct page *pg = hash_entry (p_, struct page, elem);
  frame_lock (pg);
  if (pg->frame!=NULL)
   frame_free (pg->frame);
   free (pg);
}

/* Destroys the current process's page table. */
void
page_exit (void)
{
if (&thread_current ()->pages != NULL)
hash_destroy (&thread_current ()->pages, destroy_page);
}

/* Returns the page containing the given virtual ADDRESS,
   or a null pointer if no such page exists.
   Allocates stack pages as necessary. */
static struct page *
page_for_addr (void *address)
{
struct page *p;
if (address>=PHYS_BASE-STACK_MAX)
{
p=page_allocate(address,true);
p->pgtp=STACKPG;
do_page_in(p);
}
return p;
}

/* Locks a frame for page P and pages it in.
   Returns true if successful, false on failure. */
static bool
do_page_in (struct page *p)
{
p->frame=frame_alloc_and_lock(p);
if (p->frame==NULL)return false;
if(p->pinfo->loc==SWAP)
{
swap_in(p,p->idx_);
}
else
{
if (p->pgtp==FIL)
{
file_read_at(p->pinfo->fname,p->frame->base,p->pinfo->readbytes,p->pinfo->offset_);
}
}
p->pinfo->loc=PHYMEM;
struct thread *t=thread_current();
pagedir_set_page(t->pagedir,p->addr,p->frame->base,p->writable);
return true;
}

/* Faults in the page containing FAULT_ADDR.
   Returns true if successful, false on failure. */
bool
page_in (void *fault_addr)
{
struct page *p=page_lookup(pg_round_down(fault_addr));//PGMASK
if (p!=NULL)
return(do_page_in(p));
else
{
if(page_for_addr(pg_round_down(fault_addr))!=NULL)
return true;
else
return false;
}
}

/* Evicts page P.
   P must have a locked frame.
   Return true if successful, false on failure. */
bool
page_out (struct page *p)
{
  bool is_dirty;
  bool wrt=false;

  if ( (p->frame!=NULL) && (lock_held_by_current_thread (&p->frame->frlock)) )
  {
 
  is_dirty = pagedir_is_dirty (p->frame->t->pagedir, p->addr); 
  pagedir_clear_page (thread_current()->pagedir, p->addr);
     if (is_dirty)
        {
         wrt=swap_out(p);
          if ( (!wrt) && (p->pgtp == FIL )  ){
                    wrt = (file_write_at (p->pinfo->fname, p->frame->base, p->pinfo->readbytes,p->pinfo->offset_) == p->pinfo->readbytes);
                          p->pinfo->loc=DISK;
                          }
        }
   }
    
return wrt;

}


/* Returns true if page P's data has been accessed recently,
   false otherwise.
   P must have a frame locked into memory. */
bool
page_accessed_recently (struct page *p)
{
bool access_recent;
 access_recent=pagedir_is_accessed (p->frame->t->pagedir, p->addr);
return access_recent;

}

/* Adds a mapping for user virtual address VADDR to the page hash
   table.  Fails if VADDR is already mapped or if memory
   allocation fails. */
struct page *
page_allocate (void *vaddr, bool read_write)
{
struct page *p=(struct page *)malloc(sizeof(struct page));
p->pinfo = (struct pageinfo *)malloc(sizeof(struct pageinfo));
p->addr=vaddr;
p->writable=read_write;
 if (hash_insert(&thread_current()->pages, &p->elem) != NULL) 
 {
 free (p);
 return NULL;
 }

return p;
}

/* Evicts the page containing address VADDR
   and removes it from the page table. */
void
page_deallocate (void *vaddr)
{
 struct page *p = page_lookup(vaddr);
  if (p!=NULL)
  {
  if (p->frame!=NULL)
    {
     page_out (p); 
     frame_free (p->frame);
      
    }
  
  }
  
}

/* Returns a hash value for the page that E refers to. */
unsigned
page_hash (const struct hash_elem *e, void *aux UNUSED)
{
struct page *pge = hash_entry(e, struct page,elem);
return hash_bytes(&pge->addr, sizeof pge->addr);
}

/* Returns true if page A precedes page B. */
bool
page_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux UNUSED)
{
const struct page *a = hash_entry (a_, struct page, elem);
const struct page *b = hash_entry (b_, struct page, elem);
return a->addr < b->addr;
}

/* Tries to lock the page containing ADDR into physical memory.
   If WILL_WRITE is true, the page must be writeable;
   otherwise it may be read-only.
   Returns true if successful, false on failure. 
bool
page_lock (const void *addr, bool will_write)
{
}
*/
/* Unlocks a page locked with page_lock(). 
void
page_unlock (const void *addr)
{
}

*/
