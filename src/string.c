#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "object.h"
#include "string_hash.h"
#include "string.h"

glz_string_hash* string_table;

void string_init()
{
    string_table = string_hash_init();
}

void string_exit()
{
    string_hash_destroy(string_table);
}

glz_string* string_new(const char *str, size_t l)
{
    if (l < GLZ_SSTRING_LIMIT) {
        uint32_t hash = string_hash_hash_string(str);
        glz_string_hash_iter it;
        int existp = string_hash_is_exist_by_hash(string_table, str, hash, &it);
        if (existp) {
            glz_string* s = string_hash_get_by_iter(string_table, it);
            return s;
        }
        glz_string* stro = object_string_new(str, l, hash);

        // Don't set 'str' in the second argument.
        // The pointer will be copied internal.
        string_hash_put_by_hash(string_table, ((const char*)(stro+1)), hash, stro);

        return stro;
    }
    return object_string_new(str, l, 0);
}

inline int string_lstring_compare(glz_string* s1, glz_string* s2)
{
    size_t size = s1->size;
    return ((s1->tt == s2->tt) &&
            (s1->tt == GLZ_TT_LSTRING) &&
            (size == s2->size) &&
            (memcmp(string_raw(s1), string_raw(s2), size) == 0));
}

inline int string_compare(glz_string* s1, glz_string* s2)
{
    return (s1 == s2) || string_lstring_compare(s1, s2);
}

inline ssize_t string_print_fp(glz_string* s, FILE* fp)
{
    return (ssize_t)fprintf(fp, "\"%s\"", ((const char*)(s+1)));
}
