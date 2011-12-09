#include "core.h"
#include "object.h"
#include "reader.h"
#include "symbol_table.h"

#define CHAR_MAP_SYMBOL         0x01
#define CHAR_MAP_INITIAL        0x02
#define CHAR_MAP_DELIMITER      0x04

#define SYMBOL_CHARP(x)         ((s_char_map[x] & CHAR_MAP_SYMBOL) != 0)
#define INITIAL_CHARP(x)        ((s_char_map[x] & CHAR_MAP_INITIAL) != 0)
#define DELIMITER_CHARP(x)      ((s_char_map[x] & CHAR_MAP_DELIMITER) != 0)

namespace glaze {

    bool reader_t::s_char_map_ready;
    uint8_t reader_t::s_char_map[128];

    void
    reader_t::make_char_map()
    {
        if (s_char_map_ready) return;
        for (int i = 1; i < array_sizeof(s_char_map); i++) {
            s_char_map[i]  = ((isalnum(i) || strchr(".!?*+-/:<=>$%&@^_~", i)) ? CHAR_MAP_SYMBOL : 0);
            s_char_map[i] |= ((isalpha(i) || strchr("!?*/:<=>$%&^_~", i)) ? CHAR_MAP_INITIAL : 0);
            s_char_map[i] |= (strchr("()[]\";#", i) ? CHAR_MAP_DELIMITER : 0);
        }
        s_char_map_ready = true;
    }

    reader_t::reader_t(Shared* sh)
    {
        shared = sh;
        init();
    }

    reader_t::~reader_t()
    {
        // nothing to do.
    }

    obj_t* reader_t::read()
    {
        return read_expr();
    }

    obj_t* reader_t::read(int fd)
    {
        set_source(fd);
        return read_expr();
    }

    obj_t* reader_t::read(FILE* fp)
    {
        set_source(fp);
        return read_expr();
    }

    obj_t* reader_t::read(const char* src)
    {
        set_source(src);
        return read_expr();
    }

    void reader_t::set_source(int fd)
    {
        clean();

        m_fd = fd;
        m_fp = NULL;
        m_src = NULL;
    }

    void reader_t::set_source(FILE* fp)
    {
        clean();

        m_fd = -1;
        m_fp = fp;
        m_src = NULL;
    }

    void reader_t::set_source(const char* src)
    {
        clean();

        m_fd = -1;
        m_fp = NULL;
        m_src = src;
    }

    void reader_t::clean()
    {
        m_ungetbuf = EOF;
        m_ungetbuf_valid = false;
        m_read = 0;
    }

    void reader_t::init() {

        make_char_map();

#define mksymbol(NAME, STRING) NAME = make_symbol(STRING)

        mksymbol(S_EOF, "");
        mksymbol(S_DOT, ".");
        mksymbol(S_LPAREN, "(");
        mksymbol(S_RPAREN, ")");
        mksymbol(S_LBRACK, "[");
        mksymbol(S_RBRACK, "]");
        mksymbol(S_QUOTE,  "quote");
        mksymbol(S_QUASIQUOTE, "quasiquote");
        mksymbol(S_UNQUOTE, "unquote");
        mksymbol(S_UNQUOTE_SPLICING, "unquote-splicing");

        m_fd = -1;
        m_fp = NULL;
        m_src = NULL;

        clean();
    }


//
// util
//
    inline bool reader_t::whitespace_p(int c) {
        return (c == 0x0020 || (0x0009 <= c && c <= 0x000d));
    }

    bool
    reader_t::delimited(int c)
    {
        if (whitespace_p(c)) return true;
        if (c > 127) CALLERROR("invalid character %U while reading identifier", c);
        return DELIMITER_CHARP(c);
    }

    int reader_t::get() {
        if (m_ungetbuf_valid) {
            m_ungetbuf_valid = false;
            return m_ungetbuf;

        } else {
            char c;
            int res;
        get_top:
            if (m_fd != -1) {
                res = ::read(m_fd, &c, 1);
            } else if (m_fp != NULL) {
                res = fread(&c, sizeof(char), 1, m_fp);
            } else if (m_src != NULL) {
                c = *(m_src + m_read);
                res = (int)c;
            } else {
                CALLERROR("unknown input type.");
            }

            if (res < 0) {
                if (errno == EINTR) {
                    goto get_top;
                } else {
                    CALLERROR("some error occured in get() errno -> %d", errno);
                }
            } else if (res == 0) {
                c = EOF;
            }

            m_ungetbuf = (int)c;
            m_read++;

            return (int)c;
        }
    }

    int reader_t::lookahead()
    {
        if (m_ungetbuf_valid) return m_ungetbuf;

        int ch = get();
        unget();
        return ch;
    }

    inline void reader_t::unget() {
        m_ungetbuf_valid = true;
    }

    size_t reader_t::read_thing(char* buf, size_t size)
    {
        size_t i = 0;
        while (i + 1 < size) {
            int c = lookahead();
            if (c == EOF) {
                buf[i] = 0;
                return i;
            }
            if (delimited(c)) {
                buf[i] = 0;
                return i;
            }
            get();
            if (c < 128) buf[i++] = c;
            else CALLERROR("invalid character %U while reading identifier", c);
        }
        CALLERROR("token buffer overflow while reading identifier, %s ...", buf);
    }


//
// number
//
    obj_t* reader_t::make_number(const char* buf, size_t len)
    {
        number_t* ret = new number_t();

        bool floated = false;

        for(int i=0; i < len+1; i++) {
            if (buf[i] == '.') {
                floated = true;
                break;
            }
        }

        if (floated) {
            *ret = (double)atof(buf);
        } else {
            *ret = (int64_t)atoll(buf);
        }
        return ret;
    }

    obj_t* reader_t::read_number()
    {
        char buf[READ_NUMBER_BUFFER_SIZE];
        size_t len = read_thing(buf, sizeof(buf));

        if (len == 0)
            return S_EOF;

        if (len == 1 && buf[0] == '.') return S_DOT;
        if (len == 1 && (buf[0] == '+' || buf[0] == '-')) {
            return make_symbol(buf);
        }

        return make_number(buf, len);
    }

//
// string
//
    obj_t* reader_t::make_string(const char* buf, size_t len)
    {
        return new string_t(buf, len);
    }

    obj_t* reader_t::read_string()
    {
        char small_buf[READ_STRING_SMALL_BUFFER_SIZE];
        char* buf = small_buf;
        try {
            size_t bufsize = array_sizeof(buf);
            size_t i = 0;
            while (true) {
                if (bufsize <= i+1) {
                    bufsize += bufsize; // bufsize *= 2 ?
                    if (buf == small_buf) {
                        buf = (char*)malloc(bufsize);
                        memcpy(buf, small_buf, i);
                    } else {
                        buf = (char*)realloc(buf, bufsize);
                    }
                    if (buf == NULL) {
                        CALLERROR("memory exhausted while reading string.");
                        exit(1);
                    }
                }
                int c = get();
                if (i == 0 && c == EOF)
                    return S_EOF;
                buf[i] = c;

                if (c == '"') {
                    buf[i] = 0;
                    obj_t* result = make_string(buf, i);
                    if (buf != small_buf) free(buf);
                    return result;
                }
                if(c == EOF) {
                    CALLERROR("unexpected end-of-file while reading string");
                }
                i++;
            }
        } catch (...) {
            if (buf != small_buf) { free(buf); }
            throw;
        }
    }


//
// list
//
    inline obj_t* reader_t::make_list_2items(obj_t* first, obj_t* second) {
        return new cons_t(first, (new cons_t(second, shared->_nil)));
    }

    obj_t* reader_t::reverse_list(obj_t* lst, obj_t* tail)
    {
        obj_t* r = tail;
        obj_t* temp;
        while (CONSP(lst)) {
            temp = CDR(lst);
            ((cons_t*)lst)->set_cdr(r);
            r = lst;
            lst = temp;
        }
        return r;
    }

    obj_t* reader_t::read_list(bool bracketed)
    {
        obj_t* lst = shared->_nil;
        obj_t* token;

        while ((token = read_token()) != S_EOF) {
            if (token == S_RPAREN) {
                if (bracketed) {
                    CALLERROR("bracketed list terminated by parenthesis");
                }
                lst = reverse_list(lst, shared->_nil);
                return lst;
            }
            if (token == S_RBRACK) {
                if (!bracketed) {
                    CALLERROR("parenthesized list terminated by bracket");
                }
                lst = reverse_list(lst, shared->_nil);
                return lst;
            }
            if (token == S_LPAREN) {
                lst = new cons_t(read_list(false), lst);
                continue;
            }
            if (token == S_LBRACK) {
                lst = new cons_t(read_list(true), lst);
                continue;
            }
            if (token == S_DOT) {
                if (lst == shared->_nil) {
                    CALLERROR("misplaced dot('.') while reading list");
                }
                obj_t* rest = read_expr();
                if (rest == S_DOT) CALLERROR("misplaced dot('.') while reading list");

                token = read_token();
                if (token == S_RPAREN) {
                    if (bracketed) {
                        CALLERROR("bracketed list terminated by parenthesis");
                    }
                    lst = reverse_list(lst, rest);
                    return lst;
                }
                if (token == S_RBRACK) {
                    if (!bracketed) {
                        CALLERROR("parenthesized list terminated by bracket");
                    }
                    lst = reverse_list(lst, rest);
                    return lst;
                }
                if (token == S_EOF) CALLERROR("unexpected end-of-file while reading list");
                CALLERROR("more than one item following dot('.') while reading list");
            }
            if (CONSP(token)) { }
            lst = new cons_t(token, lst);
        }

        CALLERROR("unexpected end-of-file while reading list");

        return shared->undef;
    }

/*
** symbol
*/
    obj_t* reader_t::make_symbol(const char* buf)
    {
        return const_cast<symbol_t*>(shared->symbols->get(buf));
    }

    obj_t* reader_t::read_symbol()
    {
        char buf[MAX_READ_SYMBOL_LENGTH];
        size_t len = read_thing(buf, array_sizeof(buf));
        if (len == 0)
            return S_EOF;
        return make_symbol(buf);
    }

//
// root
//
    obj_t* reader_t::read_token()
    {
        int c;

    top:
        c = get();

        if (c == EOF) return S_EOF;
        if (c > 0x007F)  CALLERROR("invalid character. c = %d at %d", c, m_read);

        if (whitespace_p(c)) goto top;

        if (isdigit(c)) {
            unget();
            return read_number();
        }

        switch (c) {
        case ';':   {
            while ((c = get()) != EOF) {
                if (c == '\n' || c == '\r') goto top;
            }
            return S_EOF;
        }
        case '"':   return read_string();
        case '(':   return S_LPAREN;
        case ')':   return S_RPAREN;
        case '[':   return S_LBRACK;
        case ']':   return S_RBRACK;

            // reader macros
        case '\'': {
            obj_t* obj = read_expr();
            if (obj == S_EOF) CALLERROR("unexpected end-of-file following quotation-mark(')");
            return make_list_2items(S_QUOTE, obj);
        }
        case '`' : {
            obj_t* obj = read_expr();
            if (obj == S_EOF) CALLERROR("unexpected end-of-file following grave-accent(`)");
            return make_list_2items(S_QUASIQUOTE, obj);
        }
        case '+' :
        case '.' :
        case '-':
            unget();
            return read_number();
        case '#':
            c = get();
            switch (c) {
            case EOF:
                CALLERROR("unexpected end-of-file following sharp-sign(#)");
            }
            CALLERROR("invalid lexical syntax #%c", c);
        case ',':
            c = get();
            if (c == EOF) CALLERROR("unexpected end-of-file following comma(,)");
            if (c == '@') return make_list_2items(S_UNQUOTE_SPLICING, read_expr());
            unget();
            return make_list_2items(S_UNQUOTE, read_expr());
        default:
            unget();
            return read_symbol();
        }
    }

    obj_t* reader_t::read_expr()
    {
        obj_t* token = read_token();

        if (token == S_RPAREN) CALLERROR("unexpected closing parenthesis");
        if (token == S_RBRACK) CALLERROR("unexpected closing bracket");
        if (token == S_LPAREN) return read_list(false);
        if (token == S_LBRACK) return read_list(true);
        return token;
    }

    void reader_t::error(const char* fname, unsigned int line, const char* fmt, ...)
    {
        va_list arg;
        va_start(arg, fmt);

        fprintf(stderr, "%s:%u !! ERROR in 'READER' !! -- ", fname, line);
        vfprintf(stderr, fmt, arg);
        fprintf(stderr, "\n\n");

        fflush(stderr);

        va_end(arg);

        throw "READER_ERROR";
    }

}
