#ifndef GLAZE__READER_H_
#define GLAZE__READER_H_

#include "core.h"
#include "object.h"
#include "symbol_table.h"
#include "context.h"

namespace glaze {

#include "char_funcs_tbl.h"

    class Reader {
    public:
        Reader(Context* cont);
        ~Reader();

        Object* read();
        Object* read(int fd);
        Object* read(FILE* fp);
        Object* read(const char* src, size_t* read_size);

        void set_source(int fd);
        void set_source(FILE* fp);
        void set_source(const char* src);

        void save_state();
        void resolv_state();

        static Object* S_EOF;

    private:

        SymbolTable* symbol_table;

        typedef struct _state {
            int         fd;
            FILE*       fp;
            const char* src;
            int         ungetbuf;
            bool        ungetbuf_valid;
            size_t      read;
        } state;

        std::vector< state > m_save;

        // values
        int m_fd;
        FILE* m_fp;
        const char* m_src;

        size_t m_read;

        bool m_ungetbuf_valid;
        int  m_ungetbuf;

        // static bool    char_funcs_tbl_ready_p;
        // static uint8_t char_funcs_tbl[128];
        // static void    make_char_funcs_tbl();

        //funcs
        void init();
        bool whitespace_p(int c);
        bool delimited(int c);

        int get();
        int lookahead();
        void unget();
        Object* read_expr();
        void clean();

        // number
        size_t read_thing(char* buf, size_t size);
        Object* make_number(const char* buf, size_t len);
        Object* read_number();

        // string
        Object* make_string(const char* buf, size_t len);
        Object* read_string();

        // list
        Object* make_list_2items(Object* first, Object* second);
        Object* reverse_list(Object* lst, Object* tail);
        Object* read_list(bool bracketed);

        // symbol
        Object* make_symbol(const char* buf);
        Object* read_symbol();

        Object* read_token();
        void error(const char* fname, unsigned int line, const char* fmt, ...);

        static Object* S_DOT;
        static Object* S_LPAREN;
        static Object* S_RPAREN;
        static Object* S_LBRACK;
        static Object* S_RBRACK;
        static Object* S_QUOTE;
        static Object* S_QUASIQUOTE;
        static Object* S_UNQUOTE;
        static Object* S_UNQUOTE_SPLICING;

    };
}

#endif // GLAZE__READER_H_
