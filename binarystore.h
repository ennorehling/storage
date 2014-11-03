#ifndef H_STORAGE_BINSTORE
#define H_STORAGE_BINSTORE
#ifdef __cplusplus
extern "C" {
#endif

struct storage;
struct stream;

void binstore_init(struct storage * store, struct stream * F);
void binstore_done(struct storage * store);

#ifdef __cplusplus
}
#endif
#endif
