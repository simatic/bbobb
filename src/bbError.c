#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "bbError.h"

void bbErrorAtLine(int status, int errnum, char *filename, int linenum, char *format,...) {
  va_list ap;
  va_start(ap, format);
  fprintf(stderr,"%s:%d:", filename,linenum);
  vfprintf(stderr, format, ap);
  fprintf(stderr,":%s\n", strerror(errnum));
  if (status) {
    abort();            
  }
  va_end(ap);
}

/** 
 * @brief Same as @a bbErrorAtLine but without having to specify an
 * errnum
 */
void bbErrorAtLineWithoutErrnum(int status, char *filename, int linenum, char *format,...) {
  va_list ap;
  va_start(ap, format);
  fprintf(stderr,"%s:%d:", filename,linenum);
  vfprintf(stderr, format, ap);
  fputs("\n",stderr);
  if (status) {
    abort();            
  }
  va_end(ap);
}

