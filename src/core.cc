#include "core.h"

ssize_t fdprintf(int fd, const char *fmt, ...) {
  va_list ap;
  FILE *f = fdopen(fd, "a");
  int rc;

  va_start(ap, fmt);
  rc = vfprintf(f, fmt, ap);
  fclose(f);
  va_end(ap);
  return rc;
}
