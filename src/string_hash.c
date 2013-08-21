#include <stdint.h>
#include "string_hash.h"

inline glz_string_hash* string_hash_init()
{
    return kh_init(str);
}

inline glz_string* string_hash_get(glz_string_hash* h,
                                   const char* key,
                                   glz_string* _default)
{
    glz_string_hash_iter it;
    it = kh_get(str, h, key);
    if (it == kh_end(h)) return _default;
    return kh_value(h, it);
}

inline glz_string* string_hash_get_by_hash(glz_string_hash* h,
                                           const char* key,
                                           uint32_t hash,
                                           glz_string* _default)
{
    glz_string_hash_iter it;
    it = kh_get_by_hash(str, h, key, (khint_t)hash);
    if (it == kh_end(h)) return _default;
    return kh_value(h, it);
}

inline glz_string* string_hash_get_by_iter(glz_string_hash* h,
                                           glz_string_hash_iter it)
{
    return kh_value(h, it);
}


inline int string_hash_put(glz_string_hash* h,
                           const char* key,
                           glz_string* val)
{
    int ret;
    glz_string_hash_iter it;
    it = kh_put(str, h, key, &ret);
    kh_value(h, it) = val;
    return ret;
}

inline int string_hash_put_by_hash(glz_string_hash* h,
                                   const char* key,
                                   uint32_t hash,
                                   glz_string* val)
{
    int ret;
    glz_string_hash_iter it;
    it = kh_put_by_hash(str, h, key, (khint_t)hash, &ret);
    kh_value(h, it) = val;
    return ret;
}


inline void string_hash_put_by_iter(glz_string_hash* h,
                                    glz_string_hash_iter it,
                                    glz_string* val)
{
    kh_value(h, it) = val;
}

inline int string_hash_is_exist(glz_string_hash* h,
                                const char* key,
                                glz_string_hash_iter* it)
{
    *it = kh_get(str, h, key);
    if (*it == kh_end(h)) return 0;
    return 1;
}

inline int string_hash_is_exist_by_hash(glz_string_hash* h,
                                        const char* key,
                                        uint32_t hash,
                                        glz_string_hash_iter* it)
{
    *it = kh_get(str, h, key);
    if (*it == kh_end(h)) return 0;
    return 1;
}

inline void string_hash_destroy(glz_string_hash* h)
{
    kh_destroy(str, h);
}

inline size_t string_hash_size(glz_string_hash* h)
{
    return (size_t)kh_size(h);
}
