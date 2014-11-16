#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "devices/block.h"
#include "filesys/off_t.h"
#include "threads/synch.h"

enum location
{
PHYMEM,
SWAP,
DISK
};

enum pgtype
{
STACKPG,
FIL
};

struct pageinfo
{
struct file *fname;
size_t offset_;
size_t readbytes;
enum location loc;
};




/* Virtual page supplementary. */
struct page
  {
     void *addr;                 /* User virtual address. */
     bool writable;              /* Is writable; */
     struct frame *frame;        /* Page frame. */
     struct pageinfo *pinfo;     /* Additional info */
     struct hash_elem elem;
     enum pgtype pgtp;           /* File Location */
     size_t idx_;                /* Swap Start Sector Index */
  };

void page_exit (void);

struct page *page_allocate (void *, bool read_write);
void page_deallocate (void *vaddr);

bool page_in (void *fault_addr);
static bool do_page_in (struct page *);
bool page_out (struct page *);
bool page_accessed_recently (struct page *);

bool page_lock (const void *, bool will_write);
void page_unlock (const void *);

hash_hash_func page_hash;
hash_less_func page_less;

#endif /* vm/page.h */
