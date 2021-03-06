#ifndef GLAZE__GLAZE_H_
#define GLAZE__GLAZE_H_

#include "core.h"
#include "object.h"
#include "env.h"
#include "eval.h"
#include "reader.h"
#include "shared.h"

namespace glaze {

    class Config {
    public:
        Config(bool require_gc_init=true) {
            m_require_gc_init = require_gc_init;
        };

        bool m_require_gc_init;
        bool require_gc_init() { return m_require_gc_init; };
        bool require_gc_init(bool cond) { m_require_gc_init = cond; return cond; };

    };

    class Interpreter
    {
    public:

        Interpreter(Config* conf);
        virtual ~Interpreter();

        int repl();
        int repl(FILE* fp_in, FILE* fp_out);
        int repl(FILE* fp_in, int fd_out);
        int repl(int fd_in, FILE* fp_out);
        int repl(int fd_in, int fd_out);

        obj_t* read_and_evaluate(const char* src);

        obj_t* read();
        obj_t* read(FILE* fp);
        obj_t* read(int fd);
        obj_t* read(const char* target, size_t* read_size);

        obj_t* eval(obj_t* obj);
        const symbol_t* get_symbol(const char* name);
        void error(const char* fname, unsigned int line, const char* fmt, ...);

        Shared shared;

    private:

        bool m_initialized;
        void init_cores();

        int repl_iter(FILE* fp_out);
        int repl_iter(int fd_out);
    };

}

#endif // GLAZE__GLAZE_H_
