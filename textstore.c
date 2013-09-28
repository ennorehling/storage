#include "textstore.h"
#include "storage.h"

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
  putc('\n', (FILE *) store.data);
  return 1;
}

static int txt_w_int(HSTORAGE store, int arg)
{
  return fprintf((FILE *) store.data, "%d ", arg);
}

static int txt_r_int(HSTORAGE store, int * result)
{
  int err = fscanf((FILE *) store.data, "%d", result);
  return (err==1) ? 0 : EOF;
}

static int txt_w_flt(HSTORAGE store, float arg)
{
  return fprintf((FILE *) store.data, "%f ", arg);
}

static int txt_r_flt(HSTORAGE store, float * result)
{
  int err = fscanf((FILE *) store.data, "%f", result);
  return (err==1) ? 0 : EOF;
}

static int txt_w_tok(HSTORAGE store, const char *tok)
{
  int result;
  if (tok == NULL || tok[0] == 0) {
    result = fputc(NULL_TOKEN, (FILE *) store.data);
  } else {
#ifndef NDEBUG
    const char *find = strchr(tok, ' ');
    if (!find)
      find = strchr(tok, NULL_TOKEN);
    assert(!find || !"reserved character in token");
    assert(tok[0] != ' ');
#endif
    result = fputs(tok, (FILE *) store.data);
  }
  fputc(' ', (FILE *) store.data);
  return result;
}

static int txt_r_tok_buf(HSTORAGE store, char *result, size_t size)
{
  char format[16];
  if (result && size > 0) {
    format[0] = '%';
    sprintf(format + 1, "%zus", size);
    if (fscanf((FILE *) store.data, format, result)!=1) {
      return EOF;
    }
    if (result[0] == NULL_TOKEN) {
      result[0] = 0;
    }
  } else {
    /* trick to skip when no result expected */
    return fscanf((FILE *) store.data, "%*s");
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
  int result = fwritestr((FILE *) store.data, str);
  fputc(' ', (FILE *) store.data);
  return result + 1;
}

static int txt_r_str_buf(HSTORAGE store, char *result, size_t size)
{
  freadstr((FILE *) store.data, result, size);
  return 0;
}

static int txt_w_bin(HSTORAGE store, const void *arg, size_t size)
{
  int bytes;
  
  bytes = fprintf((FILE *) store.data, "%zu ", size);
  if (bytes>0 && size>0) {
	  size_t i;
	  const unsigned char * buf = (const unsigned char *)arg;
	  for (i=0;i!=size;++i) {
		  int b = fprintf((FILE *) store.data, "%02x", buf[i]);
		  if (b<0) return b;
		  bytes += b;
	  }
  }
  return bytes;
}

static int txt_r_bin(HSTORAGE store, void *result, size_t len)
{
  int bytes;
  size_t size;
  
  bytes = fscanf((FILE *) store.data, "%zu ", &size);
  if (bytes>0 && size>0) {
	  size_t i;
	  unsigned char * buf = (unsigned char *)result;
	  for (i=0;i!=size;++i) {
<<<<<<< Updated upstream
		  unsigned int uc;
		  int b = fscanf((FILE *) store.data, "%02x", &uc);
=======
		  unsigned int ui;
      unsigned char uc;
		  int b = fscanf((FILE *) store.data, "%02x", &ui);

      uc = (unsigned char)ui;
>>>>>>> Stashed changes
		  if (b<0) return b;
		  if (i<len) buf[i] = (unsigned char)uc;
		  bytes += b;
	  }
  }
  return bytes;
}

const storage_i text_api = {
  txt_w_brk,
  txt_w_int, txt_r_int,
  txt_w_flt, txt_r_flt,
  txt_w_tok, txt_r_tok_buf,
  txt_w_str, txt_r_str_buf,
  txt_w_bin, txt_r_bin,
};

void txtstore_init(struct storage * store, FILE *F) {
  store->api = &text_api;
  store->handle.data = F;
}

void txtstore_done(struct storage * store) {
  assert(store->api==&text_api);
  fclose((FILE *)store->handle.data);
}
