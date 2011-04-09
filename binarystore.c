#include "binarystore.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define file(store) (FILE *)((store)->userdata)

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
static int bin_w_brk(struct storage *store)
{
  return 0;
}

static int bin_w_int_pak(struct storage *store, int arg)
{
  char buffer[5];
  size_t size = pack_int(arg, buffer);
  return (int)fwrite(buffer, sizeof(char), size, file(store));
}

static int bin_r_int_pak(struct storage *store)
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

static int bin_w_int(struct storage *store, int arg)
{
  return (int)fwrite(&arg, sizeof(arg), 1, file(store));
}

static int bin_r_int(struct storage *store)
{
  int result;
  fread(&result, sizeof(result), 1, file(store));
  return result;
}

static int bin_w_flt(struct storage *store, float arg)
{
  return (int)fwrite(&arg, sizeof(arg), 1, file(store));
}

static float bin_r_flt(struct storage *store)
{
  float result;
  fread(&result, sizeof(result), 1, file(store));
  return result;
}

static int bin_w_str(struct storage *store, const char *tok)
{
  int result;
  if (tok == NULL || tok[0] == 0) {
    result = store->w_int(store, 0);
  } else {
    int size = (int)strlen(tok);
    result = store->w_int(store, size);
    result += (int)fwrite(tok, size, 1, file(store));
  }
  return result;
}

static int bin_r_str_buf(struct storage *store, char *result, size_t size)
{
  int i;
  size_t rd, len;

  i = store->r_int(store);
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

static int bin_w_bin(struct storage *store, void *arg, size_t size)
{
  int result;
  int len = (int)size;

  result = store->w_int(store, len);
  if (len > 0) {
    result += (int)fwrite(arg, len, 1, file(store));
  }
  return result;
}

static int bin_r_bin(struct storage *store, void *result, size_t size)
{
  int len = store->r_int(store);
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

static int bin_open(struct storage *store, const char *filename, int mode)
{
  const char *modes[] = { 0, "rb", "wb", "ab" };
  FILE *F = fopen(filename, modes[mode]);
  store->userdata = F;
  if (F) {
    if (mode == IO_READ) {
      int stream_version = 0;
      stream_version = bin_r_int(store);
    } else {
      bin_w_int(store, STREAM_VERSION);
    }
  }
  return (F == NULL);
}

static int bin_close(struct storage *store)
{
  return fclose(file(store));
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
