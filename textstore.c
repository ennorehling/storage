#include "textstore.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NULL_TOKEN '@'
#define isxspace(c) (c==160 || isspace(c))

static int txt_w_brk(struct storage *store)
{
  putc('\n', (FILE *) store->userdata);
  return 1;
}

static int txt_w_int(struct storage *store, int arg)
{
  return fprintf((FILE *) store->userdata, "%d ", arg);
}

static int txt_r_int(struct storage *store)
{
  int result;
  fscanf((FILE *) store->userdata, "%d", &result);
  return result;
}

static int txt_w_flt(struct storage *store, float arg)
{
  return fprintf((FILE *) store->userdata, "%f ", arg);
}

static float txt_r_flt(struct storage *store)
{
  double result;
  fscanf((FILE *) store->userdata, "%lf", &result);
  return (float)result;
}

static int txt_w_tok(struct storage *store, const char *tok)
{
  int result;
  if (tok == NULL || tok[0] == 0) {
    result = fputc(NULL_TOKEN, (FILE *) store->userdata);
  } else {
#ifndef NDEBUG
    const char *find = strchr(tok, ' ');
    if (!find)
      find = strchr(tok, NULL_TOKEN);
    assert(!find || !"reserved character in token");
    assert(tok[0] != ' ');
#endif
    result = fputs(tok, (FILE *) store->userdata);
  }
  fputc(' ', (FILE *) store->userdata);
  return result;
}

static int txt_r_tok_buf(struct storage *store, char *result, size_t size)
{
  char format[16];
  if (result && size > 0) {
    format[0] = '%';
    sprintf(format + 1, "%us", size);
    fscanf((FILE *) store->userdata, format, result);
    if (result[0] == NULL_TOKEN) {
      result[0] = 0;
    }
  } else {
    /* trick to skip when no result expected */
    fscanf((FILE *) store->userdata, "%*s");
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

static int txt_w_str(struct storage *store, const char *str)
{
  int result = fwritestr((FILE *) store->userdata, str);
  fputc(' ', (FILE *) store->userdata);
  return result + 1;
}

static int txt_r_str_buf(struct storage *store, char *result, size_t size)
{
  freadstr((FILE *) store->userdata, result, size);
  return 0;
}

static int txt_open(struct storage *store, const char *filename, int mode)
{
  const char *modes[] = { 0, "rt", "wt", "at" };
  FILE *F = fopen(filename, modes[mode]);
  store->userdata = F;
  return (F == NULL);
}

static int txt_w_bin(struct storage *store, void *arg, size_t size)
{
  assert(!"not implemented!");
  return 0;
}

static int txt_r_bin(struct storage *store, void *result, size_t size)
{
  assert(!"not implemented!");
  return 0;
}

static int txt_close(struct storage *store)
{
  return fclose((FILE *) store->userdata);
}

const storage text_store = {
  txt_w_brk,
  txt_w_int, txt_r_int,
  txt_w_flt, txt_r_flt,
  txt_w_tok, txt_r_tok_buf,
  txt_w_str, txt_r_str_buf,
  txt_w_bin, txt_r_bin,
  txt_open, txt_close,
  NULL
};
