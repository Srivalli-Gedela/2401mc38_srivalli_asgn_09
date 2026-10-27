#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PAGE_SIZE 4096

int
main(void)
{
  int i, pid, status;
  char *heap;

  heap = sbrklazy(32 * PAGE_SIZE);
  if (heap == SBRK_ERROR) {
    printf("faulttest: lazy sbrk failed\n");
    exit(1);
  }

  // Access exactly five different pages; each should fault in once.
  for (i = 0; i < 5; i++)
    heap[i * PAGE_SIZE] = 'A' + i;

  printf("Page faults handled after touching five pages: %d\n", getfaults());

  // Invalid access is performed by a child so the parent can demonstrate
  // that the kernel kills only the faulting process.
  pid = fork();
  if (pid < 0) {
    printf("faulttest: fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    volatile char *bad = heap + 32 * PAGE_SIZE;
    *bad = 'X'; // exactly one page beyond the allocated logical range
    printf("ERROR: invalid access was not killed\n");
    exit(2);
  }

  wait(&status);
  printf("Parent survived invalid-address test.\n");
  exit(0);
}
