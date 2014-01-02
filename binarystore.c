#include "binarystore.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define file(hstore) (FILE *)(hstore.data)

static size_t pack_int(int v, char *buffer)
{
  int sign = (v < 0);

  if (sign) {
    v = ~v + 1;
    sign = 0x40;
  }
  if (v < 0x40) {
    buffer[0] = (char)(v | sign);
    return 1;
  } else if (v < 0x2000) {
    buffer[0] = (char)((v >> 6) | 0x80);
    buffer[1] = (char)((v & 0x3F) | sign);
    return 2;
  } else if (v < 0x100000) {
    buffer[0] = (char)(((v >> 13) & 0x7f) | 0x80);
    buffer[1] = (char)(((v >> 6) & 0x7f) | 0x80);
    buffer[2] = (char)((v & 0x3F) | sign);
    return 3;
  } else if (v < 0x8000000) {
    buffer[0] = (char)(((v >> 20) & 0x7f) | 0x80);
    buffer[1] = (char)(((v >> 13) & 0x7f) | 0x80);
    buffer[2] = (char)(((v >> 6) & 0x7f) | 0x80);
    buffer[3] = (char)((v & 0x3F) | sign);
    return 4;
  }
  buffer[0] = (char)(((v >> 27) & 0x7f) | 0x80);
  buffer[1] = (char)(((v >> 20) & 0x7f) | 0x80);
  buffer[2] = (char)(((v >> 13) & 0x7f) | 0x80);
  buffer[3] = (char)(((v >> 6) & 0x7f) | 0x80);
  buffer[4] = (char)((v & 0x3F) | sign);
  return 5;
}
/*
static int unpack_int(const char *buffer)
{
  int i = 0, v = 0;

  while (buffer[i] & 0x80) {
    v = (v << 7) | (buffer[i++] & 0x7f);
  }
  v = (v << 6) | (buffer[i] & 0x3f);

  if (buffer[i] & 0x40) {
    v = ~v + 1;
  }
  return v;
}
*/
static int bin_w_brk(HSTORAGE store)
{
  return store.data ? 0 : -1;
}

static int bin_w_int_pak(HSTORAGE hstore, int arg)
{
  char buffer[5];
  size_t size = pack_int(arg, buffer);
  return (int)fwrite(buffer, sizeof(char), size, file(hstore));
}

static int bin_r_int_pak(HSTORAGE hstore, int * result)
{
  int v = 0;
  size_t items;
  char ch;

  items = fread(&ch, sizeof(char), 1, file(hstore));
  if (items!=1) {
    return EOF;
  }
  while (ch & 0x80) {
    v = (v << 7) | (ch & 0x7f);
    items = fread(&ch, sizeof(char), 1, file(hstore));
    if (items!=1) {
      return EOF;
    }
  }
  if (result) {
    v = (v << 6) | (ch & 0x3f);

    if (ch & 0x40) {
      v = ~v + 1;
    }
    *result = v;
  }
  return 0;
}

static int bin_w_flt(HSTORAGE hstore, float arg)
{
  return (int)fwrite(&arg, sizeof(arg), 1, file(hstore));
}

static int bin_r_flt(HSTORAGE hstore, float * result)
{
  size_t items;
  float flt;
  
  items = fread(result ? result : &flt, sizeof(float), 1, file(hstore));
  return (items==1) ? 0 : EOF;
}

static int bin_w_str(HSTORAGE hstore, const char *tok)
{
  int result;
  if (tok == NULL || tok[0] == 0) {
    result = bin_w_int_pak(hstore, 0);
  } else {
    int size = (int)strlen(tok);
    result = bin_w_int_pak(hstore, size);
    result += (int)fwrite(tok, size, 1, file(hstore));
  }
  return result;
}

static int bin_r_str_buf(HSTORAGE hstore, char *result, size_t size)
{
  int i, err;

  err = bin_r_int_pak(hstore, &i);
  if (err!=0) {
    return err;
  }
  if (i == 0) {
    result[0] = 0;
  }
  else if (size > 0) {
    size_t rd, len, items;
    len = (size_t)i;
    rd = (len < size) ? len : (size - 1);
    items = fread(result, sizeof(char), rd, file(hstore));
    if (items!=rd) {
      return EOF;
    }
    else {
      if (rd < len) {
        err = fseek(file(hstore), (long)(len - rd), SEEK_CUR);
        if (err) {
          return err;
        }
        result[size - 1] = 0;
        return ENOMEM;
      }
      else {
        result[len] = 0;
      }
    }
  }
  else {
    err = fseek(file(hstore), (long)i, SEEK_CUR);
  }
  return err;
}

static int bin_w_bin(HSTORAGE hstore, const void *arg, size_t size)
{
  int result;
  int len = (int)size;

  result = bin_w_int_pak(hstore, len);
  if (len > 0) {
    result += (int)fwrite(arg, len, 1, file(hstore));
  }
  return result;
}

static int bin_r_bin(HSTORAGE hstore, void *result, size_t size)
{
  size_t items;
  int len, err;
  
  err = bin_r_int_pak(hstore, &len);
  if (err!=0) {
    return err;
  }
  if (len > 0) {
    if ((size_t) len > size) {
      /* "destination buffer too small */
      fseek(file(hstore), len, SEEK_CUR);
      return ENOMEM;
    } else {
      items = fread(result, len, 1, file(hstore));
      return (items==1) ? 0 : EOF;
    }
  }
  return EILSEQ;
}
/*
static HSTORAGE bin_open(FILE * F, int mode)
{
  return (HSTORAGE)F;
}

static int bin_close(HSTORAGE store)
{
  return 0;
}
*/
const storage_i binary_api = {
  bin_w_brk,                    /* newline (ignore) */
  bin_w_int_pak, bin_r_int_pak, /* int storage */
  bin_w_flt, bin_r_flt,         /* float storage */
  bin_w_str, bin_r_str_buf,     /* token storage */
  bin_w_str, bin_r_str_buf,     /* string storage */
  bin_w_bin, bin_r_bin,         /* binary storage */
/*  bin_open, bin_close */
};

void binstore_init(struct storage * store, FILE * F) {
  store->api = &binary_api;
  store->handle.data = F;
}

void binstore_done(struct storage * store) {
  assert(store->api==&binary_api);
  fclose((FILE *)store->handle.data);
}
