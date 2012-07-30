#include "core.h"
#include "object.h"
#include "reader.h"
#include "symbol_table.h"

namespace glaze {

    Object* Reader::S_EOF;
    Object* Reader::S_DOT;
    Object* Reader::S_LPAREN;
    Object* Reader::S_RPAREN;
    Object* Reader::S_LBRACK;
    Object* Reader::S_RBRACK;
    Object* Reader::S_QUOTE;
    Object* Reader::S_QUASIQUOTE;
    Object* Reader::S_UNQUOTE;
    Object* Reader::S_UNQUOTE_SPLICING;

    Reader::Reader() : symbol_table()
    {
        init();
    }

    Reader::~Reader()
    {
        m_save.clear();
    }

    Object* Reader::read()
    {
        return read_expr();
    }

    Object* Reader::read(int fd)
    {
        set_source(fd);
        return read_expr();
    }

    Object* Reader::read(FILE* fp)
    {
        set_source(fp);
        return read_expr();
    }

    Object* Reader::read(const char* src, size_t* read_size)
    {
        set_source(src);
        Object* ret = read_expr();
        (*read_size) = m_read;
        return ret;
    }

    void Reader::set_source(int fd)
    {
        clean();

        m_fd = fd;
        m_fp = NULL;
        m_src = NULL;
    }

    void Reader::set_source(FILE* fp)
    {
        clean();

        m_fd = -1;
        m_fp = fp;
        m_src = NULL;
    }

    void Reader::set_source(const char* src)
    {
        clean();

        m_fd = -1;
        m_fp = NULL;
        m_src = src;
    }

    void Reader::clean()
    {
        m_ungetbuf = EOF;
        m_ungetbuf_valid = false;
        m_read = 0;
    }

    void Reader::save_state()
    {
        Reader::state now = { m_fd, m_fp, m_src, m_ungetbuf, m_ungetbuf_valid, m_read };
        m_save.push_back(now);
    }

    void Reader::resolv_state()
    {
        if (m_save.empty()) CALLERROR("state is not saved.");
        Reader::state* last = &(m_save.back());

        m_fd = last->fd;
        m_fp = last->fp;
        m_src = last->src;
        m_ungetbuf = last->ungetbuf;
        m_ungetbuf_valid = last->ungetbuf_valid;
        m_read = last->read;

        m_save.pop_back();
    }

    void Reader::init() {

#define MKSYM(NAME, STRING) NAME = make_symbol(STRING)

        // MKSYM(S_EOF, "t");
        // MKSYM(S_EOF, "nil");

        MKSYM(S_EOF, "");
        MKSYM(S_DOT, ".");
        MKSYM(S_LPAREN, "(");
        MKSYM(S_RPAREN, ")");
        MKSYM(S_LBRACK, "[");
        MKSYM(S_RBRACK, "]");
        MKSYM(S_QUOTE,  "quote");
        MKSYM(S_QUASIQUOTE, "quasiquote");
        MKSYM(S_UNQUOTE, "unquote");
        MKSYM(S_UNQUOTE_SPLICING, "unquote-splicing");

        m_fd = -1;
        m_fp = NULL;
        m_src = NULL;

        clean();
    }


//
// util
//
    inline bool Reader::whitespace_p(int c) {
        return (c == 0x0020 || (0x0009 <= c && c <= 0x000d));
    }

    bool Reader::delimited(int c)
    {
        if (whitespace_p(c)) return true;
        if (c > 127) CALLERROR("invalid character %U while reading identifier", c);
        return DELIMITER_CHARP(c);
    }

    int Reader::get()
    {
        if (m_ungetbuf_valid) {
            m_ungetbuf_valid = false;
            m_read++;
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
                    CALLERROR("some error occured in get(). (%s)", strerror(errno));
                }
            } else if (res == 0) {
                c = EOF;
            }

            m_ungetbuf = (int)c;
            m_read++;

            return (int)c;
        }
    }

    int Reader::lookahead()
    {
        if (m_ungetbuf_valid) return m_ungetbuf;

        int ch = get();
        unget();
        return ch;
    }

    inline void Reader::unget()
    {
        m_ungetbuf_valid = true;
        m_read--;
    }

    size_t Reader::read_thing(char* buf, size_t size)
    {
        size_t i = 0;
        while (i + 1 < size) {
            int c = lookahead();
            if (c == 0 || c == EOF) {
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

    // Object Readers

    Object* Reader::make_number(const char* buf, size_t len)
    {
        Number* ret = new Number();

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

    Object* Reader::read_number()
    {
        char buf[READ_NUMBER_BUFFER_SIZE];
        size_t len = read_thing(buf, sizeof(buf));

        if (len == 0) return S_EOF;

        if (len == 1 && buf[0] == '.') return S_DOT;
        if (len == 1 && (buf[0] == '+' || buf[0] == '-')) {
            return make_symbol(buf);
        }

        return make_number(buf, len);
    }

//
// string
//
    Object* Reader::make_string(const char* buf, size_t len)
    {
        return new Str(buf, len);
    }

    Object* Reader::read_string()
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
                    Object* result = make_string(buf, i);
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
    inline Object* Reader::make_list_2items(Object* first, Object* second) {
        return new Cons(first, (new Cons(second, const_cast<Object*>(&Object::nil))));
    }

    Object* Reader::reverse_list(Object* lst, Object* tail)
    {
        Object* r = tail;
        Object* temp;
        while (lst->isCons()) {
            temp = CDR(lst);
            ((Cons*)lst)->setCdr(r);
            r = lst;
            lst = temp;
        }
        return r;
    }

    Object* Reader::read_list(bool bracketed)
    {
        Object* _nil = const_cast<Object*>(&Object::nil);
        Object* lst = _nil;
        Object* token;

        // TODO: up efficiency.
        // save first-ptr and last-ptr of list.

        while ((token = read_token()) != S_EOF) {
            if (token == S_RPAREN) {
                if (bracketed) {
                    CALLERROR("bracketed list terminated by parenthesis");
                }
                lst = reverse_list(lst, _nil);
                return lst;
            }
            if (token == S_RBRACK) {
                if (!bracketed) {
                    CALLERROR("parenthesized list terminated by bracket");
                }
                lst = reverse_list(lst, _nil);
                return lst;
            }
            if (token == S_LPAREN) {
                lst = new Cons(read_list(false), lst);
                continue;
            }
            if (token == S_LBRACK) {
                lst = new Cons(read_list(true), lst);
                continue;
            }
            if (token == S_DOT) {
                if (lst == _nil) {
                    CALLERROR("misplaced dot('.') while reading list");
                }
                Object* rest = read_expr();
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
            if (token->isCons()) { }
            lst = new Cons(token, lst);
        }

        CALLERROR("unexpected end-of-file while reading list");

        return const_cast<Object*>(&Object::undef);
    }

/*
** symbol
*/
    Object* Reader::make_symbol(const char* buf)
    {
        if (strcmp(buf, "nil") == 0) return const_cast<Object*>(&Object::nil);
        if (strcmp(buf, "t") == 0) return const_cast<Object*>(&Object::t);

        return const_cast<Symbol*>(symbol_table.get(buf));
    }

    Object* Reader::read_symbol()
    {
        char buf[MAX_READ_SYMBOL_LENGTH];
        size_t len = read_thing(buf, array_sizeof(buf));
        if (len == 0) return S_EOF;
        return make_symbol(buf);
    }

//
// root
//
    Object* Reader::read_token()
    {
        int c;

    top:
        c = get();

        if (c == 0 || c == EOF) return S_EOF;
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
            Object* obj = read_expr();
            if (obj == S_EOF) CALLERROR("unexpected end-of-file following quotation-mark(')");
            return make_list_2items(S_QUOTE, obj);
        }
        case '`' : {
            Object* obj = read_expr();
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

    Object* Reader::read_expr()
    {
        Object* token = read_token();

        if (token == S_RPAREN) CALLERROR("unexpected closing parenthesis");
        if (token == S_RBRACK) CALLERROR("unexpected closing bracket");
        if (token == S_LPAREN) return read_list(false);
        if (token == S_LBRACK) return read_list(true);
        return token;
    }

    void Reader::error(const char* fname, unsigned int line, const char* fmt, ...)
    {
        char fname_buf[32];
        remove_dir(fname, fname_buf, 32);

        va_list arg;
        va_start(arg, fmt);

        fprintf(stderr, "%s:%u !! ERROR in 'READER' !! -- ", fname_buf, line);
        vfprintf(stderr, fmt, arg);
        fprintf(stderr, "\n\n");

        fflush(stderr);

        va_end(arg);

        throw "READER_ERROR";
    }
}
