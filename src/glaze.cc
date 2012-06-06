#include "core.h"
#include "object.h"
#include "shared.h"
#include "reader.h"
#include "eval.h"
#include "env.h"
#include "primitives.h"
#include "symbol_table.h"
#include "glaze.h"

namespace glaze {

    Interpreter::Interpreter(Config* conf) : m_initialized(false)
    {
        if (conf->require_gc_init()) {
            GC_INIT();
        }

        shared._nil     = new nil_t();
        shared.t        = new t_t();
        shared.undef    = new undef_t();

        shared.gs_acc   = 0;

        // Important this order.
        // Because g_symbol_table is refered in
        // the reader and the evaluator and the env.
        shared.symbols      = new symbol_table();

        shared.global_env   = new env_t();
        shared.evaluator    = new evaluator_t(&shared);
        shared.reader       = new reader_t(&shared);

        std::vector<const symbol_t*> primitive_variables    = std::vector<const symbol_t*>();
        std::vector<obj_t*> primitive_values                = std::vector<obj_t*>();

        primitives::setup_primitives(&primitive_variables, &primitive_values, &shared);
        int extend_result = shared.global_env->extend(&primitive_variables, &primitive_values);

        if (extend_result != 0)
            CALLERROR("in constructor of Interpreter. extending global_env is failed %d.", extend_result);

        init_cores();
    }

    Interpreter::~Interpreter()
    {
        shared.global_env->enclose();
    }

    void Interpreter::init_cores()
    {
        if (m_initialized) return;

        const int core_len = 5;
        const char* core_libs[core_len] = {
#include "arc/base.arc.core"
#include "arc/qq.arc.core"
#include "arc/list.arc.core"
#include "arc/math.arc.core"
#include "arc/compiler.arc.core"
        };

        for (int i = 0; i < core_len; i++ ){
            read_and_evaluate(core_libs[i]);
        }

        m_initialized = true;
    }

    int Interpreter::repl()
    {
        return repl(stdin, stdout);
    }

    int Interpreter::repl(FILE* fp_in, FILE* fp_out)
    {
        shared.reader->set_source(fp_in);
        return repl_iter(fp_out);
    }

    int Interpreter::repl(FILE* fp_in, int fd_out)
    {
        shared.reader->set_source(fp_in);
        return repl_iter(fd_out);
    }

    int Interpreter::repl(int fd_in, FILE* fp_out)
    {
        shared.reader->set_source(fd_in);
        return repl_iter(fp_out);
    }

    int Interpreter::repl(int fd_in, int fd_out)
    {
        shared.reader->set_source(fd_in);
        return repl_iter(fd_out);
    }

    int Interpreter::repl_iter(FILE* fp_out)
    {
        obj_t* read_result;
        obj_t* eval_result;

    retry:
        try {
            while (1) {
                fprintf(fp_out, "> ");
                read_result = shared.reader->read();
                if (read_result == shared.reader->S_EOF) break;
                eval_result = shared.evaluator->eval(read_result, shared.global_env);
                eval_result->print(fp_out);
                fprintf(fp_out, "\n");
            }
        } catch (const char* msg) {
            fprintf(stderr, "    -- %s\n", msg);
            goto retry;
        }

        return 0;

    }

    int Interpreter::repl_iter(int fd_out){
        obj_t* read_result;
        obj_t* eval_result;

    retry:
        try {
            while (1) {
                fdprintf(fd_out, "> ");
                read_result = shared.reader->read();
                if (read_result == shared.reader->S_EOF) break;
                eval_result = shared.evaluator->eval(read_result, shared.global_env);
                eval_result->print(fd_out);
                fdprintf(fd_out, "\n");
            }
        } catch (const char* msg) {
            fdprintf(fd_out, "    -- %s\n", msg);
            goto retry;
        }

        return 0;
    }

    obj_t* Interpreter::read_and_evaluate(const char* src)
    {
        obj_t* read_result = shared.undef;
        obj_t* eval_result = shared.undef;
        size_t read_size_1 = 0;
        size_t read_size = 0;

    retry:
        try {
            while (1) {
                read_result = shared.reader->read(src+read_size, &read_size_1);
                read_size += read_size_1;
                if (read_result == shared.reader->S_EOF) break;
                eval_result = shared.evaluator->eval(read_result, shared.global_env);
            }
        } catch (const char* msg) {
            fprintf(stderr, "    -- %s\n", msg);
            fflush(stderr);
            goto retry;
        }

        return eval_result;
    }

    obj_t* Interpreter::read()
    {
        return read(stdin);
    }

    obj_t* Interpreter::read(FILE* fp)
    {
        return shared.reader->read(fp);
    }

    obj_t* Interpreter::read(int fd)
    {
        return shared.reader->read(fd);
    }

    obj_t* Interpreter::read(const char* target, size_t* read_size)
    {
        return shared.reader->read(target, read_size);
    }

    obj_t* Interpreter::eval(obj_t* obj)
    {
        return shared.evaluator->eval(obj, shared.global_env);
    }

    const symbol_t* Interpreter::get_symbol(const char* name) {
        return shared.symbols->get(name);
    }

    void Interpreter::error(const char* fname, unsigned int line, const char* fmt, ...)
    {
        char fname_buf[32];
        remove_dir(fname, fname_buf, 32);

        va_list arg;
        va_start(arg, fmt);

        fprintf(stderr, "%s:%u !! ERROR in 'Interpreter' !! -- ", fname_buf, line);
        vfprintf(stderr, fmt, arg);
        fprintf(stderr, "\n\n");

        fflush(stderr);

        va_end(arg);

        throw "Interpreter Error !!";
    }

}
