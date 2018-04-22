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
 *
 * TODO malloc() wrapper
 * TODO usage() function
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


/* Global variables */
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


/* pattern_create() -- Creates pattern.
 * TODO
 */
char *pattern_create(unsigned int length) {
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


/* pattern_offset() - Calculate and output offset.
 * TODO
 */
void pattern_offset(const char *pattern, const char *offset) {
  int         i, x;
  char        *hexoffset = NULL;
  char        *optr;


  /* If offset begins with "0x", convert hex to ASCII */
  if (offset[0] == '0' && offset[1] == 'x') {
    int       offsetlen;
    char      curhex[2];
    char      byte;

    curhex[2] = '\0';
    offsetlen = (strlen(offset) - 2) / 2;
    hexoffset = malloc(offsetlen);
    if (hexoffset == NULL) {
      perror("malloc()");
      exit(EXIT_FAILURE);
    }

    for(x = 0, i = 2; i < strlen(offset); x++, i += 2) {
      if (strlen(offset) % 2 == 1 && i == 2) {
        /* Offset length is odd. Assume that the missing byte
         * is 0 (Ex: 0xf == 0x0f) and realign the incrementor.
         */
        curhex[0] = '0';
        curhex[1] = offset[i];
        i--;
      } else {
        curhex[0] = offset[i];
        curhex[1] = offset[i + 1];
      }

      byte = strtol(curhex, NULL, 16);
      if (endian == 0)
        hexoffset[offsetlen - x - 1] = byte;
      else
        hexoffset[x] = byte;
    }
  }

  /* Do simple pointer arithmetic to calculate offset.
   *
   * TODO this might be a better location than pattern_create() deal with
   * the pattern wrapping after 20280 bytes. Rather than creating a pattern
   * with the last 20280 bytes patterned: AAAA...PATTERN, display the last
   * matching pattern's offset.
   */
  if (hexoffset)
    offset = hexoffset;

  optr = strstr(pattern, offset);
  if (optr != NULL)
    printf("%ld\n", optr - pattern);
  else
    printf("not found\n");

  if (hexoffset)
    free(hexoffset);
}


/* main() -- Program's entry point.
 */
int main(int argc, char *argv[]) {
  int             opt;
  int             len;
  char            *p;
  char            *offset = NULL;

  endian = getendian();
  if (endian == -1) {
    fprintf(stderr, "Unable to determine endianness. Toggle with -e\n");
    endian = 0; /* default to little endian */
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

  if (offset != NULL) {
    pattern_offset(p, offset);
  } else {
    printf("%s\n", p);
  }

  return EXIT_SUCCESS;
}

