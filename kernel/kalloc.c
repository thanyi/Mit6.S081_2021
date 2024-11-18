// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.
int pagemapcnt[(PHYSTOP - KERNBASE) / PGSIZE];    // 对每一个physical page标记维护的数组

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct spinlock mapcntlock;    // 针对pagemapcnt的数组的锁

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&mapcntlock, "mapcnt");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");
  /* 进行计数的操作必须要在memset之前！！！！ */
  int idx = getmapidx((uint64)pa);
  acquire(&mapcntlock);
  
  if(pagemapcnt[idx] > 1){  // 对pagemapcnt数组的申请判断最好加锁
    pagemapcnt[idx]--;  // 如果指向这片page的指针大于0，就不做操作,只减去计数
    release(&mapcntlock);
    return;
  }
  pagemapcnt[idx]--;
  release(&mapcntlock);
  // 如果 <= 0 就直接free
  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    // addmapcnt((uint64)(r));
    acquire(&mapcntlock);
    int idx = getmapidx((uint64)(r));
    // malloc将数组中cnt变为1,因为有可能是复用的chunk，可能原本在数组中不为0，为1
    pagemapcnt[idx] = 1;    
    release(&mapcntlock);
  }
    
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

// 用于获取page对应在pagemapcnt中的idx
int 
getmapidx(uint64 pa)
{
  pa = PGROUNDDOWN(pa);
  return (int)((pa - KERNBASE) / PGSIZE);
}

// 添加mapcnt数组中相关的idx
void
addmapcnt(uint64 pa)
{

  acquire(&mapcntlock);
  int idx = getmapidx(pa);
  pagemapcnt[idx] += 1;
  release(&mapcntlock);
}

// 添加mapcnt数组中相关的idx
void
reducemapcnt(uint64 pa)
{
  acquire(&mapcntlock);
  int idx = getmapidx(pa);
  pagemapcnt[idx] -= 1;
  release(&mapcntlock);
}