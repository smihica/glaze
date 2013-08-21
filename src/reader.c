#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>

#include "glaze.h"
#include "object.h"
#include "reader.h"
#include "symbol.h"
#include "string.h"
#include "cons.h"

#include "char_functions_table.h"

static glz_obj* s_eof;
static glz_obj* s_dot;
static glz_obj* s_quote;
static glz_obj* s_quasiquote;
static glz_obj* s_unquote;
static glz_obj* s_unquote_splicing;
static glz_obj* s_rparen;
static glz_obj* s_lparen;
static glz_obj* s_rbrack;
static glz_obj* s_lbrack;
static glz_obj* s_shortfn;

void reader_init()
{
#define s(name, c) s_##name = (glz_obj*)(symbol_new(c, strlen(c)))
    s(eof,              "");
    s(dot,              ".");
    s(quote,            "quote");
    s(quasiquote,       "quasiquote");
    s(unquote,          "unquote");
    s(unquote_splicing, "unquote_splicing");
    s(rparen,           "rparen");
    s(lparen,           "lparen");
    s(rbrack,           "rbrack");
    s(lbrack,           "lbrack");
    s(shortfn,          GLZ_SHADOW "shortfn");
#undef sym
}

inline int reader_whitespace_p(int c)
{
    return (c == 0x20 || (0x09 <= c && c <= 0x0D));
}

inline int reader_delimited_p(int c)
{
    if (reader_whitespace_p(c)) return 1;
    if (127 < c) err("invalid character %U while reading identifier", c);
    return reader_delimiter_char_p(c);
}

inline int reader_readermacro_p(int c)
{
    return c == '#';
}

int reader_get(reader_source* s)
{
    char c;
    int res;

    if (s->ungetbuf_valid) {
        s->ungetbuf_valid = 0;
        s->read++;
        return s->ungetbuf;
    }

top:
    switch (s->tt) {
    case READER_SOURCE_FD:
        res = read(s->src.fd, &c, 1);
        break;
    case READER_SOURCE_FP:
        res = fread(&c, sizeof(char), 1, s->src.fp);
        break;
    case READER_SOURCE_STR:
        c = *(s->src.str + s->read);
        res = (int)c;
        break;
    default:
        bug("unknown input type.");
    }

    if (res < 0) {
        if (errno == EINTR) goto top;
        else                fatal("some error occurred in reader_get() - %s", strerror(errno));
    } else if (c == 0 || res == 0) {
        c = EOF;
    }

    s->ungetbuf = (int)c;
    s->read++;

    return (int)c;
}

inline void reader_unget(reader_source* s)
{
    s->ungetbuf_valid = 1;
    s->read--;
}

inline int reader_lookahead(reader_source* s)
{
    int c = reader_get(s);
    reader_unget(s);
    return c;
}

size_t reader_read_thing(reader_source* s, char* buf, size_t size)
{
    size_t i = 0;
    while ((i+1) < size) {
        int c = reader_lookahead(s);
        if (c == 0 || c == EOF) {
            buf[i] = '\0';
            return i;
        }
        if (reader_delimited_p(c)) {
            buf[i] = 0;
            return i;
        }
        reader_get(s);
        if (127 < c) err("invalid character %U while reading identifier", c);

        buf[i++] = c;
    }
    err("token buffer overflow while reading identifier, %s ...", buf);
    return 0;
}

glz_obj* reader_read_number(reader_source* s)
{
    return NULL;
}

glz_obj* reader_read_string(reader_source* s)
{
    char small_buf[READER_READ_STRING_BUF_SIZE];
    char* buf = small_buf;
    size_t bufsize = READER_READ_STRING_BUF_SIZE;
    size_t i = 0;
    while (1) {
        if (bufsize <= i+1) {
            bufsize += bufsize;
            int flag = 0;
            if (buf == small_buf) {
                buf = (char*)malloc(bufsize);
                flag = 1;
            } else {
                buf = (char*)realloc(buf, bufsize);
            }
            if (buf == NULL) {
                fatal("memory exhausted while reading string.");
            }
            if (flag) {
                memcpy(buf, small_buf, i);
            }
        }
        int c = reader_get(s);

        if (i == 0 && c == EOF) return s_eof;

        buf[i] = c;
        if (c == '"') {
            buf[i] = '\0';
            glz_obj* result = (glz_obj*)string_new(buf, i);
            if (buf != small_buf) free(buf);
            return result;
        }
        if (c == EOF) {
            if (buf != small_buf) free(buf);
            err("unexpected end-of-file while reading string");
        }
        i++;
    }
}

glz_obj* reader_read_list(reader_source* s, int bracketed)
{
    glz_obj* token;
    glz_obj* lis = glz_nil;
    while ((token = reader_read_token(s)) != s_eof)
    {
        if (token == s_rparen) {
            if (bracketed) err("bracketed list terminated by parenthesis");
            return reader_nreverse(lis, glz_nil);
        }
        if (token == s_rbrack) {
            if (!bracketed) err("parenthesized list terminated by bracket");
            return reader_nreverse(lis, glz_nil);
        }
        if (token == s_lparen) {
            lis = (glz_obj*)cons_new(reader_read_list(s, 0), lis);
            continue;
        }
        if (token == s_lbrack) {
            lis = (glz_obj*)cons_new(reader_read_bracketed_list(s), lis);
            continue;
        }
        if (token == s_dot) {
            if (lis == glz_nil) err("misplaced dot('.') while reading list");
            glz_obj* rest = reader_read_expr(s);
            if (rest == s_dot) err("misplaced dot('.') while reading list");
            token = reader_read_token(s);
            if (token == s_rparen) {
                if (bracketed) err("bracketed list terminated by parenthesis");
                lis = reader_nreverse(lis, rest);
                return lis;
            }
            if (token == s_rbrack) {
                if (!bracketed) err("parenthesized list terminated by bracket");
                lis = reader_nreverse(lis, rest);
                return lis;
            }
            if (token == s_eof) err("unexpected end-of-file while reading list");
            err("more than one item following dot('.') while reading list");
        }
        lis = (glz_obj*)cons_new(token, lis);
    }
    return s_eof;
}

glz_obj* reader_read_bracketed_list(reader_source* s)
{
    glz_obj* body = reader_read_list(s, 1);
    return (glz_obj*)list_new2(s_shortfn, body);
}

glz_obj* reader_read_symbol(reader_source* s)
{
    char buf[GLZ_SYMBOL_LIMIT];
    size_t len = reader_read_thing(s, buf, GLZ_SYMBOL_LIMIT);
    if (len == 0) return s_eof;
    return (glz_obj*)symbol_new(buf, len);
}

glz_obj* reader_read_token(reader_source* s)
{
    int c;
top:
    c = reader_get(s);
    if (c == EOF) return s_eof;
    if (127 < c) err("Invalid character. c = %d at %d", c, s->read);

    if (reader_whitespace_p(c)) goto top;

    if (isdigit(c)) {
        reader_unget(s);
        return reader_read_number(s);
    }

    if (reader_readermacro_p(c)) {
        c = reader_get(s);
        if (c == '|') {
            char tmp;
            c = '\0';
            while (c != EOF) {
                tmp = c;
                c = reader_get(s);
                if (tmp == '|' && c == '#') goto top;
            }
            return s_eof;
        }
        err("reader-macro is not implemented yet.");
    }

    switch (c) {
    case ';': {
        while ((c = reader_get(s)) != EOF) {
            if (c == '\n' || c == '\r') goto top;
        }
        return s_eof;
    }
    case '"':   return reader_read_string(s);
    case '(':   return s_lparen;
    case ')':   return s_rparen;
    case '[':   return s_lbrack;
    case ']':   return s_rbrack;

    case '\'': {
        glz_obj* obj = reader_read_expr(s);
        if (obj == s_eof) err("unexpected end-of-file following quotation-mark(')");
        return (glz_obj*)list_new2(s_quote, obj);
    }
    case '`': {
        glz_obj* obj = reader_read_expr(s);
        if (obj == s_eof) err("unexpected end-of-file following grave-accent(`)");
        return (glz_obj*)list_new2(s_quasiquote, obj);
    }
    case ',': {
        c = reader_get(s);
        if (c == EOF) err("unexpected end-of-file following comma(,)");
        if (c == '@') return (glz_obj*)list_new2(s_unquote_splicing, reader_read_expr(s));
        reader_unget(s);
        return (glz_obj*)list_new2(s_unquote, reader_read_expr(s));
    }
    default:
        reader_unget(s);
        return reader_read_symbol(s);
    }
}

glz_obj* reader_read_expr(reader_source* s)
{
    glz_obj* token = reader_read_token(s);
    if (token == s_rparen) err("unexpected closing parenthesis");
    if (token == s_rbrack) err("unexpected closing bracket");
    if (token == s_lparen) return reader_read_list(s, 0);
    if (token == s_lbrack) return reader_read_bracketed_list(s);
    return token;
}

glz_obj* reader_nreverse(glz_obj* list, glz_obj* tail)
{
    glz_obj* r = tail;
    glz_obj* temp;
    while(is_cons(list)) {
        temp = list->cons.cdr;
        list->cons.cdr = r;
        r = list;
        list = temp;
    }
    return r;
}

glz_obj* reader_read_fp(FILE* fp)
{
    reader_source s;
    s.tt = READER_SOURCE_FP;
    s.ungetbuf = '\0';
    s.ungetbuf_valid = 0;
    s.read = 0;
    s.src.fp = fp;
    return reader_read_expr(&s);
}
