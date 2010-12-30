#include "../common.h"

/* OS X, Cygwin and MSVC don't have the fmemopen() function, so we provide our own, which just
 * saves the buffer to a file and then returns its file handle
 */

FILE *fmemopen (void *buf, size_t size, const char *opentype)
{
  FILE *f;
  assert(strcmp(opentype, "r") == 0);
  char* tmp_fname = _tempnam("%TMP%", "fmemopen");
  f = fopen(tmp_fname, "wt");
  fwrite(buf, 1, size, f);
  fclose(f);
  f = fopen(tmp_fname, "rt");

  return f;
}
