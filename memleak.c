#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#define NAME "memleak"
#define CHUNK ((size_t)1024)

/* Option flags */
typedef uint8_t flag_t;
#define KEEP_RUNNING ((flag_t)0x01)
#define BYTES        ((flag_t)0x02)
#define KBYTES       ((flag_t)0x04)
#define MBYTES       ((flag_t)0x08)
#define GBYTES       ((flag_t)0x10)

#define OPTSTRING "rbkmg"
#define USAGE "Usage: " NAME " [-" OPTSTRING "]\n"
#define DIE(...) do{ fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE); }while(0)
uint16_t parseargs(int argc, char **argv)
{
  int opt;
  flag_t flags = 0;

  while((opt = getopt(argc, argv, OPTSTRING)) != -1)
    switch(opt)
    {
      case 'r': flags |= KEEP_RUNNING; break;
      case 'b': flags |= BYTES; break;
      case 'k': flags |= KBYTES; break;
      case 'm': flags |= MBYTES; break;
      case 'g': flags |= GBYTES; break;
      default: DIE(USAGE);
    }
  
  return flags;
}

void eat_memory(flag_t flags)
{
  register void *p;
  register size_t alloc = 0;
  char *unitstr;
  size_t scale;
  register eating = 1;

  switch(flags & (BYTES | KBYTES | MBYTES | GBYTES))
  {
    case 0: /* Default to bytes */
    case BYTES: unitstr = ""; scale = 1; break;
    case KBYTES: unitstr = "kB"; scale = 1024; break;
    case MBYTES: unitstr = "MB"; scale = 1048576L; break;
    case GBYTES: unitstr = "GB"; scale = 1073741824L; break;
    default: DIE(NAME ": Only one of -b, -k, -m, or -g may be specified\n");
  }

  while(1)
  {
    errno = 0;
    p = malloc(CHUNK);

    if(!p && eating)
      switch(errno)
      {
        case ENOMEM: /* Limit found. */
          printf("Limit reached at %llu %s\n", alloc/scale, unitstr) ;
          eating = 0;
          if(!(flags & KEEP_RUNNING)) exit(ENOMEM);
          break;
        default: /* Any other error means something went wrong. */
          DIE(NAME ": malloc(): %s\n", strerror(errno));
      }
    else if(!eating && p)
    {
      ++eating;
      printf("Limit removed. Continuing to consume memory from %llu %s\n", alloc/scale, unitstr);
    }

    alloc += CHUNK;
  }

}

int main(int argc, char **argv)
{
  flag_t flags;

  flags = parseargs(argc, argv);

  eat_memory(flags);

  assert("Should never get here.");
  return 0;
}
