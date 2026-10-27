#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAXREF 100
#define MAXFRAME 20

static int default_refs[] = {
  7, 0, 1, 2, 0, 3, 0, 4, 2, 3,
  0, 3, 2, 1, 2, 0, 1, 7, 0, 1
};

static void
printframes(int *frames, int nframes, int fault)
{
  int j;
  printf("[");
  for (j = 0; j < nframes; j++) {
    if (frames[j] < 0)
      printf(" -");
    else
      printf(" %d", frames[j]);
  }
  printf(" ] %s\n", fault ? "FAULT" : "HIT");
}

static int
runfifo(int *refs, int nrefs, int nframes, int trace)
{
  int frames[MAXFRAME];
  int i, j, hit, next = 0, faults = 0;

  for (i = 0; i < nframes; i++)
    frames[i] = -1;

  if (trace)
    printf("\nFIFO trace (%d frames):\n", nframes);

  for (i = 0; i < nrefs; i++) {
    hit = 0;
    for (j = 0; j < nframes; j++) {
      if (frames[j] == refs[i]) {
        hit = 1;
        break;
      }
    }
    if (!hit) {
      frames[next] = refs[i];
      next = (next + 1) % nframes;
      faults++;
    }
    if (trace) {
      printf("%d: ", refs[i]);
      printframes(frames, nframes, !hit);
    }
  }
  return faults;
}

static int
runlru(int *refs, int nrefs, int nframes, int trace)
{
  int frames[MAXFRAME], last_used[MAXFRAME];
  int i, j, hit, victim, faults = 0;

  for (i = 0; i < nframes; i++) {
    frames[i] = -1;
    last_used[i] = -1;
  }

  if (trace)
    printf("\nLRU trace (%d frames):\n", nframes);

  for (i = 0; i < nrefs; i++) {
    hit = 0;
    for (j = 0; j < nframes; j++) {
      if (frames[j] == refs[i]) {
        hit = 1;
        last_used[j] = i;
        break;
      }
    }

    if (!hit) {
      victim = -1;
      for (j = 0; j < nframes; j++) {
        if (frames[j] == -1) {
          victim = j;
          break;
        }
      }
      if (victim == -1) {
        victim = 0;
        for (j = 1; j < nframes; j++) {
          if (last_used[j] < last_used[victim])
            victim = j;
        }
      }
      frames[victim] = refs[i];
      last_used[victim] = i;
      faults++;
    }

    if (trace) {
      printf("%d: ", refs[i]);
      printframes(frames, nframes, !hit);
    }
  }
  return faults;
}

int
main(int argc, char *argv[])
{
  int refs[MAXREF];
  int nrefs, nframes, i, fifo_faults, lru_faults;
  int custom = 0;

  if (argc > 1) {
    if (argc < 3 || argc - 2 > MAXREF) {
      printf("Usage: pagerepl [frames ref1 ref2 ...]\n");
      exit(1);
    }
    nframes = atoi(argv[1]);
    nrefs = argc - 2;
    if (nframes < 1 || nframes > MAXFRAME) {
      printf("Frame count must be between 1 and %d\n", MAXFRAME);
      exit(1);
    }
    for (i = 0; i < nrefs; i++)
      refs[i] = atoi(argv[i + 2]);
    custom = 1;
  } else {
    nrefs = sizeof(default_refs) / sizeof(default_refs[0]);
    for (i = 0; i < nrefs; i++)
      refs[i] = default_refs[i];
    nframes = 3;
  }

  if (!custom) {
    printf("Reference string: ");
    for (i = 0; i < nrefs; i++)
      printf("%d%s", refs[i], i == nrefs - 1 ? "\n" : " ");
  }

  fifo_faults = runfifo(refs, nrefs, nframes, 1);
  lru_faults = runlru(refs, nrefs, nframes, 1);

  printf("\nResults for %d frames (%d references):\n", nframes, nrefs);
  printf("FIFO faults=%d hits=%d hit ratio=%d/%d\n",
         fifo_faults, nrefs - fifo_faults, nrefs - fifo_faults, nrefs);
  printf("LRU  faults=%d hits=%d hit ratio=%d/%d\n",
         lru_faults, nrefs - lru_faults, nrefs - lru_faults, nrefs);

  if (!custom) {
    printf("\nFrame-count comparison:\n");
    printf("Frames  FIFO faults/hit%%       LRU faults/hit%%\n");
    for (nframes = 3; nframes <= 5; nframes++) {
      fifo_faults = runfifo(refs, nrefs, nframes, 0);
      lru_faults = runlru(refs, nrefs, nframes, 0);
      printf("%d       %d/%d%%              %d/%d%%\n",
             nframes, fifo_faults,
             (nrefs - fifo_faults) * 100 / nrefs,
             lru_faults,
             (nrefs - lru_faults) * 100 / nrefs);
    }
  }

  exit(0);
}

