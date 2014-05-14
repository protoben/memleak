#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAME "memleak"
#define CHUNK ((size_t)1024)
#define DIE(...) do{ fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE); }while(0)

/* Option flags */
typedef uint8_t flag_t;
#define KEEP_RUNNING ((flag_t)0x01)
#define BYTES        ((flag_t)0x02)
#define KBYTES       ((flag_t)0x04)
#define MBYTES       ((flag_t)0x08)
#define GBYTES       ((flag_t)0x10)
#define POLL         ((flag_t)0x20)

typedef struct _opt_st
{
  flag_t flags;
  size_t limit;
  size_t chunk;
  char unit_string[3];
  size_t unit_scale;
} opts_t;

size_t strtosize(char *str)
{
  size_t size;
  char *unit;

  size = strtoull(str, &unit, 10);
  switch(*unit)
  {
    case 'b': case 'B': case '\0': break;
    case 'k': case 'K': size *= 1024; break;
    case 'm': case 'M': size *= 1048576L; break;
    case 'g': case 'G': size *= 1073741824L; break;
    default: DIE("memleak: Unknown unit: %s", unit);
  }

  return size;
}

#define OPTSTRING "rbkmgpl:c:"
#define USAGE "Usage: " NAME " [-rp] [-bkmg] [-l limit[bkmg]] [-c chunk[bkmg]]\n"
void parseargs(int argc, char **argv, opts_t *optsp)
{
  int opt;

  /* Default options */
  optsp->flags = 0;
  optsp->limit = SIZE_MAX;
  optsp->chunk = CHUNK;

  while((opt = getopt(argc, argv, OPTSTRING)) != -1)
    switch(opt)
    {
      case 'b': optsp->flags |= BYTES; break;
      case 'c': optsp->chunk = strtosize(optarg); break;
      case 'g': optsp->flags |= GBYTES; break;
      case 'k': optsp->flags |= KBYTES; break;
      case 'l': optsp->limit = strtosize(optarg); break;
      case 'm': optsp->flags |= MBYTES; break;
      case 'p': optsp->flags |= POLL; break;
      case 'r': optsp->flags |= KEEP_RUNNING; break;
      case 'h':
      default: DIE(USAGE);
    }

  switch(optsp->flags & (BYTES | KBYTES | MBYTES | GBYTES))
  {
    case 0: /* Default to bytes */
    case BYTES: strcpy(optsp->unit_string, ""); optsp->unit_scale = 1; break;
    case KBYTES: strcpy(optsp->unit_string, "kB"); optsp->unit_scale = 1024; break;
    case MBYTES: strcpy(optsp->unit_string, "MB"); optsp->unit_scale = 1048576L; break;
    case GBYTES: strcpy(optsp->unit_string, "GB"); optsp->unit_scale = 1073741824L; break;
    default: DIE(NAME ": Only one of -b, -k, -m, or -g may be specified\n");
  }
}

void eat_memory(opts_t *optsp)
{
  register void *p;
  register size_t alloc = 0;
  register enum {QUIT, EATING, LIMITED, FULL} state;

  state = EATING;
  while(state)
    switch(state)
    {
      case EATING:
        errno = 0;
        p = malloc(optsp->chunk);

        if(alloc >= optsp->limit) state = FULL;
        else if(p)
        {
          putchar('.');
          alloc += optsp->chunk;
          break;
        }
        else if(errno == ENOMEM) state = LIMITED;
        else DIE(NAME ": malloc(): %s\n", strerror(errno));

        printf("\nLimit reached at %llu %s\n",
               (long long unsigned)alloc/optsp->unit_scale, optsp->unit_string);
        if(!(optsp->flags & KEEP_RUNNING)) state = QUIT;
        break;
      case LIMITED:
        if((optsp->flags & POLL) && malloc(optsp->chunk))
        {
          alloc += optsp->chunk;
          state = EATING;
          printf("Limit break! How? Consuming memory from %llu %s\n",
                 (long long unsigned)alloc/optsp->unit_scale, optsp->unit_string);
        }
        break;
      case FULL:
      default:
        break;
    }
}

int main(int argc, char **argv)
{
  opts_t opts;

  parseargs(argc, argv, &opts);

  eat_memory(&opts);

  return EXIT_SUCCESS;
}
