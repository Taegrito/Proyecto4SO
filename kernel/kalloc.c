// Asignador de memoria física con conteo de referencias por página
// (necesario para Copy-on-Write).

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // primera dirección después del kernel (kernel.ld)

struct run {
  struct run *next;
};

#define NPAGES ((PHYSTOP - KERNBASE) / PGSIZE)
#define PAIDX(pa) (((uint64)(pa) - KERNBASE) / PGSIZE)

struct {
  struct spinlock lock;
  struct run *freelist;
  int refs[NPAGES]; // referencias por página física; protegido por lock
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    kmem.refs[PAIDX(p)] = 1; // kfree() la baja a 0 y la libera
    kfree(p);
  }
}

// Baja en uno el conteo de la página; solo cuando llega a 0
// vuelve a la lista libre.
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&kmem.lock);
  if(kmem.refs[PAIDX(pa)] < 1)
    panic("kfree: refs");
  if(--kmem.refs[PAIDX(pa)] > 0){
    release(&kmem.lock);
    return;
  }
  release(&kmem.lock);

  // Nadie la usa: rellenar con basura para detectar usos colgantes.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Devuelve una página de 4096 bytes con conteo 1, o 0 si no hay memoria.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    kmem.refs[PAIDX(r)] = 1;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // basura
  return (void*)r;
}

// Suma una referencia a la página física pa (la usa uvmcopy).
void
krefinc(uint64 pa)
{
  acquire(&kmem.lock);
  if(kmem.refs[PAIDX(pa)] < 1)
    panic("krefinc");
  kmem.refs[PAIDX(pa)]++;
  release(&kmem.lock);
}

// Número de referencias actuales de la página física pa.
int
krefcount(uint64 pa)
{
  int n;
  acquire(&kmem.lock);
  n = kmem.refs[PAIDX(pa)];
  release(&kmem.lock);
  return n;
}

// Páginas físicas libres en todo el sistema.
int
kfreepages(void)
{
  int n = 0;
  struct run *r;

  acquire(&kmem.lock);
  for(r = kmem.freelist; r; r = r->next)
    n++;
  release(&kmem.lock);
  return n;
}

// Páginas físicas con más de una referencia.
int
ksharedpages(void)
{
  int n = 0;

  acquire(&kmem.lock);
  for(int i = 0; i < NPAGES; i++)
    if(kmem.refs[i] > 1)
      n++;
  release(&kmem.lock);
  return n;
}
