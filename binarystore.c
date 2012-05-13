#include "binarystore.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define file(store) (FILE *)(store)

#define STREAM_VERSION 2

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
  return 0;
}

static int bin_w_int_pak(HSTORAGE store, int arg)
{
  char buffer[5];
  size_t size = pack_int(arg, buffer);
  return (int)fwrite(buffer, sizeof(char), size, file(store));
}

static int bin_r_int_pak(HSTORAGE store)
{
  int v = 0;
  char ch;

  fread(&ch, sizeof(char), 1, file(store));
  while (ch & 0x80) {
    v = (v << 7) | (ch & 0x7f);
    fread(&ch, sizeof(char), 1, file(store));
  }
  v = (v << 6) | (ch & 0x3f);

  if (ch & 0x40) {
    v = ~v + 1;
  }
  return v;
}

static int bin_w_flt(HSTORAGE store, float arg)
{
  return (int)fwrite(&arg, sizeof(arg), 1, file(store));
}

static float bin_r_flt(HSTORAGE store)
{
  float result;
  fread(&result, sizeof(result), 1, file(store));
  return result;
}

static int bin_w_str(HSTORAGE store, const char *tok)
{
  int result;
  if (tok == NULL || tok[0] == 0) {
    result = bin_w_int_pak(store, 0);
  } else {
    int size = (int)strlen(tok);
    result = bin_w_int_pak(store, size);
    result += (int)fwrite(tok, size, 1, file(store));
  }
  return result;
}

static int bin_r_str_buf(HSTORAGE store, char *result, size_t size)
{
  int i;
  size_t rd, len;

  i = bin_r_int_pak(store);
  assert(i >= 0);
  if (i == 0) {
    result[0] = 0;
  } else {
    len = (size_t) i;
    rd = (len<size) ? len : (size - 1);
    fread(result, sizeof(char), rd, file(store));
    if (rd < len) {
      fseek(file(store), (long)(len - rd), SEEK_CUR);
      result[size - 1] = 0;
      return ENOMEM;
    } else {
      result[len] = 0;
    }
  }
  return 0;
}

static int bin_w_bin(HSTORAGE store, void *arg, size_t size)
{
  int result;
  int len = (int)size;

  result = bin_w_int_pak(store, len);
  if (len > 0) {
    result += (int)fwrite(arg, len, 1, file(store));
  }
  return result;
}

static int bin_r_bin(HSTORAGE store, void *result, size_t size)
{
  int len = bin_r_int_pak(store);
  if (len > 0) {
    if ((size_t) len > size) {
      /* "destination buffer too small */
      fseek(file(store), len, SEEK_CUR);
      return ENOMEM;
    } else {
      fread(result, len, 1, file(store));
      return 0;
    }
  }
  return EILSEQ;
}

static HSTORAGE bin_open(FILE * F, int mode)
{
  return (HSTORAGE)F;
}

static int bin_close(HSTORAGE store)
{
  return 0;
}

const storage binary_store = {
  bin_w_brk,                    /* newline (ignore) */
  bin_w_int_pak, bin_r_int_pak, /* int storage */
  bin_w_flt, bin_r_flt,         /* float storage */
  bin_w_str, bin_r_str_buf,     /* token storage */
  bin_w_str, bin_r_str_buf,     /* string storage */
  bin_w_bin, bin_r_bin,         /* binary storage */
  bin_open, bin_close
};
