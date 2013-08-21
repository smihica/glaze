#ifndef GLAZE__READER_H_
#define GLAZE__READER_H_

#define READER_SOURCE_FD  0
#define READER_SOURCE_FP  1
#define READER_SOURCE_STR 2

#define READER_READ_STRING_BUF_SIZE 64

typedef struct reader_source {
    int tt;
    int ungetbuf_valid;
    int ungetbuf;
    size_t read;
    union {
        int   fd;
        FILE* fp;
        const char* str;
    } src;
} reader_source;

void reader_init();
extern inline int reader_whitespace_p(int c);
extern inline int reader_delimited_p(int c);
extern inline int reader_readermacro_p(int c);
int reader_get(reader_source* s);
extern inline void reader_unget(reader_source* s);
extern inline int reader_lookahead(reader_source* s);
size_t reader_read_thing(reader_source* s, char* buf, size_t size);
glz_obj* reader_read_number(reader_source* s);
glz_obj* reader_read_string(reader_source* s);
glz_obj* reader_read_list(reader_source* s, int bracketed);
glz_obj* reader_read_bracketed_list(reader_source* s);
glz_obj* reader_read_symbol(reader_source* s);
glz_obj* reader_read_token(reader_source* s);
glz_obj* reader_read_expr(reader_source* s);
glz_obj* reader_read_fp(FILE* fp);
glz_obj* reader_nreverse(glz_obj* list, glz_obj* tail);

#endif // GLAZE__READER_H_
