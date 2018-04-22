/*
 * pattern - by Daniel Roberson @dmfroberson
 *
 * This is a recreation of Metasploit's pattern_create and
 * pattern_offset scripts. Written in C for environments without
 * Ruby or Python installed.
 *
 * Usage:
 *   pattern 128
 *   <prints out pattern>
 *
 *   pattern -o Ad8A 128
 *   114
 *
 *   pattern -o 0x41643841
 *   114
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int endian;

/* getendian() -- Determine endianness of machine.
 *
 * Returns:
 *    1 if big endian
 *    0 if little endian
 *    -1 if something went wrong
 */
int getendian() {
  union {
    unsigned int    i;
    unsigned char   x[4];
  } u;

  u.i = 0xAABBCCDD;

  if (u.x[0] == 0xAA)
    return 1;
  else if (u.x[0] == 0xDD)
    return 0;

  return -1;
}


char *pattern_create(unsigned int length) {
  // TODO: This currently wraps the pattern around at 20280 bytes.
  //       Print 'A' * multiples of 20280 + pattern so only the last
  //       portion gets the pattern. Otherwise, pattern matches will
  //       not be correct.
  int             i;
  char            x[] = "Aa0";
  char            *out;

  out = malloc(length);
  if (out == NULL) {
    perror("malloc()");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < length; i++) {
    out[i] = x[i % 3];

    if (i % 3 == 0) {
      x[2]++;

      if ((i % 10 == 0)) {
        x[2] = '0';
        x[1]++;
      }

      if ((i % (10 * 26) == 0)) {
        x[1] = 'a';
        x[0]++;
      }

      if ((i % (10 * 26 * 26)) == 0) {
        x[0] = 'A';
      }
    }
  }

  return out;
}


int main(int argc, char *argv[]) {
  int             opt;
  int             len;
  char            *p;
  char            *o;
  char            *offset = NULL;
  char            *buf = NULL;

  endian = getendian();
  if (endian == -1) {
    fprintf(stderr, "Unable to determine endianness. Toggle with -e\n");
    endian = 0; // default to little
  }

  while((opt = getopt(argc, argv, "eo:")) != -1) {
    switch(opt) {
      case 'e': /* toggle endianness */
        endian = endian ? 0 : 1;
        break;
      case 'o': /* offset in ASCII or Hexadecimal */
        offset = optarg;
        break;
      default:
        break;
    }
  }

  argc -= optind;
  argv += optind;

  if (argc != 1) {
    fprintf(stderr, "usage: pattern <length> [-o <offset>]\n");
    return EXIT_FAILURE;
  }

  len = atoi(argv[0]);
  if (len == 0) {
    fprintf(stderr, "Length must be greater than zero.\n");
    return EXIT_FAILURE;
  }

  p = pattern_create(len);

  // TODO: split this into its own function.
  if (offset != NULL) {
    int i, x, offsetlen;
    char *hexbyte;
    char curhex[2];

    curhex[2] = '\0';

    if (offset[0] == '0' && offset[1] == 'x') {
      offsetlen = (strlen(offset) - 2) / 2;
      buf = malloc(offsetlen);
      if (buf == NULL) {
        perror("malloc()");
        return EXIT_FAILURE;
      }

      for(x = 0, i = 2; i < strlen(offset); x++, i += 2) {
        if (strlen(offset) % 2 == 1 && i == 2) {
          // offset length is odd, so assume that missing byte is 0
          // and realign incrementor
          curhex[0] = '0';
          curhex[1] = offset[i];
          i--;
        } else {
          curhex[0] = offset[i];
          curhex[1] = offset[i + 1];
        }

        // reversed for little endian.
        // TODO: detect endianness
        buf[offsetlen - x - 1] = strtol(curhex, &hexbyte, 16);
      }
    }

    if (buf) {
      offset = buf;
    }
    o = strstr(p, offset);
    if (o != NULL)
      printf("%ld\n", o - p);
    else
      printf("not found\n");

    free(buf);
  } else {
    printf("%s\n", p);
  }

  return EXIT_SUCCESS;
}

