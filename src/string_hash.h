#ifndef GLAZE__HASH_H_
#define GLAZE__HASH_H_

#include <stdint.h>
#include "lib/khash.h"
#include "object.h"

KHASH_MAP_INIT_STR(str, glz_string*)

typedef khash_t(str) glz_string_hash;
typedef khiter_t     glz_string_hash_iter;

#define string_hash_hash_string(...) ((uint32_t)kh_str_hash_func(__VA_ARGS__))

extern inline glz_string_hash* string_hash_init();
extern inline glz_string* string_hash_get(glz_string_hash* h, const char* key, glz_string* _default);
extern inline glz_string* string_hash_get_by_hash(glz_string_hash* h, const char* key, uint32_t hash, glz_string* _default);
extern inline glz_string* string_hash_get_by_iter(glz_string_hash* h, glz_string_hash_iter it);
extern inline int string_hash_put(glz_string_hash* h, const char* key, glz_string* val);
extern inline int string_hash_put_by_hash(glz_string_hash* h, const char* key, uint32_t hash, glz_string* val);
extern inline void string_hash_put_by_iter(glz_string_hash* h, glz_string_hash_iter it, glz_string* val);
extern inline int string_hash_is_exist(glz_string_hash* h, const char* key, glz_string_hash_iter* it);
extern inline int string_hash_is_exist_by_hash(glz_string_hash* h, const char* key, uint32_t hash, glz_string_hash_iter* it);
extern inline void string_hash_destroy(glz_string_hash* h);
extern inline size_t string_hash_size(glz_string_hash* h);

#endif
