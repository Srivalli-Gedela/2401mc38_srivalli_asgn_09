#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
uint64
sys_shm_get(void)
{
  struct proc *p = myproc();
  char *mem;

  // Return the existing shared page if this process already has one.
  if (p->shm_pa != 0)
    return SHMBASE;

  // Allocate one physical page.
  mem = kalloc();
  if (mem == 0)
    return -1;

  memset(mem, 0, PGSIZE);

  // Map the page into the process's user address space.
  if (mappages(p->pagetable, SHMBASE, PGSIZE,
               (uint64)mem, PTE_R | PTE_W | PTE_U) < 0) {
    kfree(mem);
    return -1;
  }

  p->shm_pa = (uint64)mem;

  return SHMBASE;
}


// Assignment 9: expose page-table printing to user space.
uint64
sys_vmprint(void)
{
  return vmprint();
}

// Assignment 9: return this process's successfully handled lazy page faults.
uint64
sys_getfaults(void)
{
  return myproc()->num_page_faults;
}
