#include "textstore.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NULL_TOKEN '@'
#define isxspace(c) (c==160 || isspace(c))

static int txt_w_brk(HSTORAGE store)
{
  putc('\n', (FILE *) store);
  return 1;
}

static int txt_w_int(HSTORAGE store, int arg)
{
  return fprintf((FILE *) store, "%d ", arg);
}

static int txt_r_int(HSTORAGE store)
{
  int result;
  fscanf((FILE *) store, "%d", &result);
  return result;
}

static int txt_w_flt(HSTORAGE store, float arg)
{
  return fprintf((FILE *) store, "%f ", arg);
}

static float txt_r_flt(HSTORAGE store)
{
  double result;
  fscanf((FILE *) store, "%lf", &result);
  return (float)result;
}

static int txt_w_tok(HSTORAGE store, const char *tok)
{
  int result;
  if (tok == NULL || tok[0] == 0) {
    result = fputc(NULL_TOKEN, (FILE *) store);
  } else {
#ifndef NDEBUG
    const char *find = strchr(tok, ' ');
    if (!find)
      find = strchr(tok, NULL_TOKEN);
    assert(!find || !"reserved character in token");
    assert(tok[0] != ' ');
#endif
    result = fputs(tok, (FILE *) store);
  }
  fputc(' ', (FILE *) store);
  return result;
}

static int txt_r_tok_buf(HSTORAGE store, char *result, size_t size)
{
  char format[16];
  if (result && size > 0) {
    format[0] = '%';
    sprintf(format + 1, "%us", size);
    fscanf((FILE *) store, format, result);
    if (result[0] == NULL_TOKEN) {
      result[0] = 0;
    }
  } else {
    /* trick to skip when no result expected */
    fscanf((FILE *) store, "%*s");
  }
  return 0;
}

static int freadstr(FILE * F, char *start, size_t size)
{
  char *str = start;
  int quote = 0;
  for (;;) {
    int c = fgetc(F);

    if (isxspace(c)) {
      if (str == start) {
        continue;
      }
      if (!quote) {
        *str = 0;
        return (int)(str - start);
      }
    }
    switch (c) {
    case EOF:
      return EOF;
    case '"':
      if (!quote && str != start) {
        assert
          (!"file contains a \" that isn't at the start of a string.\n");
      }
      if (quote) {
        *str = 0;
        return (int)(str - start);
      }
      quote = 1;
      break;
    case '\\':
      c = fgetc(F);
      switch (c) {
      case EOF:
        return EOF;
      case 'n':
        if ((size_t) (str - start + 1) < size) {
          *str++ = '\n';
        }
        break;
      default:
        if ((size_t) (str - start + 1) < size) {
          *str++ = (char)c;
        }
      }
      break;
    default:
      if ((size_t) (str - start + 1) < size) {
        *str++ = (char)c;
      }
    }
  }
}

static int fwritestr(FILE * F, const char *str)
{
  int nwrite = 0;
  fputc('\"', F);
  if (str) {
    while (*str) {
      int c = (int)(unsigned char)*str++;
      switch (c) {
      case '"':
      case '\\':
        fputc('\\', F);
        fputc(c, F);
        nwrite += 2;
        break;
      case '\n':
        fputc('\\', F);
        fputc('n', F);
        nwrite += 2;
        break;
      default:
        fputc(c, F);
        ++nwrite;
      }
    }
  }
  fputc('\"', F);
  return nwrite + 2;
}

static int txt_w_str(HSTORAGE store, const char *str)
{
  int result = fwritestr((FILE *) store, str);
  fputc(' ', (FILE *) store);
  return result + 1;
}

static int txt_r_str_buf(HSTORAGE store, char *result, size_t size)
{
  freadstr((FILE *) store, result, size);
  return 0;
}

static int txt_w_bin(HSTORAGE store, void *arg, size_t size)
{
  assert(!"not implemented!");
  return 0;
}

static int txt_r_bin(HSTORAGE store, void *result, size_t size)
{
  assert(!"not implemented!");
  return 0;
}

static HSTORAGE txt_open(FILE * F, int mode)
{
  return (HSTORAGE)F;
}

static int txt_close(HSTORAGE store)
{
  return 0;
}

const storage text_store = {
  txt_w_brk,
  txt_w_int, txt_r_int,
  txt_w_flt, txt_r_flt,
  txt_w_tok, txt_r_tok_buf,
  txt_w_str, txt_r_str_buf,
  txt_w_bin, txt_r_bin,
  txt_open, txt_close
};
