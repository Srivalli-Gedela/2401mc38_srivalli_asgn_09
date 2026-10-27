#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PAGE_SIZE 4096

int
main(void)
{
  char *p;

  printf("\n--- Page table before sbrk ---\n");
  vmprint();

  p = sbrk(4 * PAGE_SIZE);
  if (p == SBRK_ERROR) {
    printf("pttest: sbrk failed\n");
    exit(1);
  }

  printf("\n--- Page table after sbrk(4 pages) ---\n");
  vmprint();

  // Touch the four newly allocated pages so their contents are observable.
  p[0] = 'A';
  p[PAGE_SIZE] = 'B';
  p[2 * PAGE_SIZE] = 'C';
  p[3 * PAGE_SIZE] = 'D';

  printf("\n--- Page table after touching the four pages ---\n");
  vmprint();
  exit(0);
}
