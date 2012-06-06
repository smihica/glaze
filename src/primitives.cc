#include "core.h"
#include "object.h"
#include "reader.h"
#include "eval.h"
#include "env.h"
#include "primitives.h"
#include "symbol_table.h"

#undef CALLERROR(...)
#define CALLERROR(...) primitives_error(__FILE__, __LINE__, __VA_ARGS__)
#define ERROR_STR_LEN 1024

namespace glaze {

    namespace primitives {

        void primitives_error(const char* fname, unsigned int line, const char* fmt, ...)
        {
            size_t i = 0;
            char fname_buf[32];
            size_t size = ERROR_STR_LEN;
            char* buf = (char*)malloc(size);
            *buf = 0;

            remove_dir(fname, fname_buf, 32);

            va_list arg;
            va_start(arg, fmt);

            i += snprintf(buf + i, size - i, "%s:%u -- ", fname_buf, line);
            i += vsnprintf(buf + i, size - i, fmt, arg);

            va_end(arg);

            throw buf;
        }

        obj_t* arc_eval(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                CALLERROR("wrong number of arguments. 'eval' expects 1.");
            }

            obj_t* first = CAR(args);
            obj_t* rest  = CDR(args);

            if (!NILP(rest)) {
                CALLERROR("wrong number of arguments. 'eval' expects 1");
            }

            return shared->evaluator->eval(first, shared->global_env);
        }

        obj_t* arc_apply(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                CALLERROR("wrong number of arguments. 'apply' expects 1 or more.");
            }

            obj_t* first = CAR(args);
            obj_t* rest  = CDR(args);

            if (!FUNCTIONP(first)) {
                CALLERROR("wrong type of arguments. 'apply' expects the first argument of type <function>.");
            }

            if (NILP(rest)) {
                return shared->evaluator->apply(reinterpret_cast<function_t*>(first), shared->_nil);
            }

            obj_t* apply_args = rest;

            obj_t* pre_last_arg_cons = shared->_nil;
            obj_t* last_arg_cons     = shared->_nil;
            obj_t* last_arg          = shared->_nil;
            while (!NILP(rest)) {
                pre_last_arg_cons = last_arg_cons;
                last_arg_cons = rest;
                last_arg = CAR(rest);
                rest = CDR(rest);
            }

            if (!(CONSP(last_arg) || NILP(last_arg))) {
                CALLERROR("wrong type of arguments. 'apply' expects the last argument of type <cons> or nil.");
            }

            if (NILP(pre_last_arg_cons)) {
                apply_args = last_arg;
            } else {
                // check last arg.
                rest = last_arg;
                while (!NILP(rest)) {
                    if (!CONSP(rest)) CALLERROR("arguments must be a proper list.");
                    rest = CDR(rest);
                }
                reinterpret_cast<cons_t*>(pre_last_arg_cons)->set_cdr(last_arg);
            }

            return shared->evaluator->apply(reinterpret_cast<function_t*>(first), apply_args);

        }

        obj_t* compile(obj_t* args, Shared* shared)
        {
            const symbol_t* compile_sym = shared->symbols->get("compile-arc");
            obj_t* proc = shared->global_env->lookup(compile_sym);

            if (proc == NULL)
                CALLERROR("function (compile-arc x...) is not defined.");

            if (!CLOSUREP(proc))
                CALLERROR("symbol 'compile-arc must be an arc function.");

            if (NILP(args))
                CALLERROR("primitive procedure 'compile' expects an argument but no.");

            obj_t* target_fn_sym = CAR(args);

            if (!SYMBOLP(target_fn_sym))
                CALLERROR("primitive procedure 'compile' expects a symbol.");

            obj_t* target_fn = shared->global_env->lookup((const symbol_t*)target_fn_sym);

            if (target_fn == NULL)
                CALLERROR("target function is not defined.");

            if (!CLOSUREP(target_fn))
                CALLERROR("%s must be an arc function.", ((const symbol_t*)target_fn_sym)->name());

            closure_t* target = (closure_t*)target_fn;

            args = (new cons_t(target->param(), new cons_t(target->body(), shared->_nil)));

            shared->evaluator->apply_compound_procedure((closure_t*)proc, args);

            return shared->undef;
        }

        obj_t* plus(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                int64_t z = 0;
                return new number_t(z);
            }

            obj_t* first = CAR(args);
            obj_t* rest  = CDR(args);

            if (NUMBERP(first)) {
                const number_t* n = (number_t*)first;
                number_t* ret = new number_t(*n);

                while (!NILP(rest)) {

                    first = CAR(rest);

                    if (!NUMBERP(first)) CALLERROR("primitive procedure '+' some arguments are invalid.");

                    *ret += *((number_t*)first);

                    rest = CDR(rest);
                }
                return (obj_t*)ret;
            }

            if (STRINGP(first)){
                const char* src = ((string_t*)first)->c_str();
                string_t* ret = new string_t(src, strlen(src));

                while (!NILP(rest)) {
                    first = CAR(rest);

                    if (!STRINGP(first)) CALLERROR("primitive procedure '+' some arguments are invalid.");

                    string_t* p = (string_t*)first;
                    *ret += *p;

                    rest = CDR(rest);
                }

                return (obj_t*)ret;
            }

            CALLERROR("primitive procedure '+' some arguments are invalid.");
        }

        obj_t* minus(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                int64_t z = 0;
                return new number_t(z);
            }

            obj_t* first = CAR(args);
            obj_t* rest  = CDR(args);

            if (NUMBERP(first)) {
                const number_t* n = (number_t*)first;
                number_t* ret = new number_t(*n);
                while (!NILP(rest)) {
                    first = CAR(rest);

                    if (!NUMBERP(first)) CALLERROR("primitive procedure '-' some arguments are invalid.");

                    *ret -= *((number_t*)first);

                    rest = CDR(rest);
                }
                return (obj_t*)ret;

            }

            CALLERROR("primitive procedure '-' some arguments are invalid.");
        }

        obj_t* multiple(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                int64_t o = 1;
                return new number_t(o);
            }

            obj_t* first = CAR(args);
            obj_t* rest  = CDR(args);

            if (NUMBERP(first)) {
                const number_t* n = (number_t*)first;
                number_t* ret = new number_t(*n);
                while (!NILP(rest)) {
                    first = CAR(rest);

                    if (!NUMBERP(first)) CALLERROR("primitive procedure '*' some arguments are invalid.");

                    *ret *= *((number_t*)first);

                    rest = CDR(rest);
                }
                return (obj_t*)ret;
            }

            CALLERROR("primitive procedure '*' some arguments are invalid.");
        }

        obj_t* devide(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                CALLERROR("primitive procedure '/' requires at least one argument.");
            }

            obj_t* first = CAR(args);
            obj_t* rest  = CDR(args);

            if (NUMBERP(first)) {
                const number_t* n = (number_t*)first;
                number_t* ret = new number_t(*n);
                while (!NILP(rest)) {
                    first = CAR(rest);

                    if (!NUMBERP(first)) CALLERROR("primitive procedure '/' some arguments are invalid.");

                    *ret /= *((number_t*)first);

                    rest = CDR(rest);
                }
                return (obj_t*)ret;
            }

            CALLERROR("primitive procedure '/' some arguments are invalid.");
        }

        obj_t* equal(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                return shared->t;
            }

            obj_t* first = CAR(args);
            obj_t* rest  = CDR(args);

            if (NILP(rest)) {
                return shared->t;
            }

            if (NUMBERP(first)) {

                do {
                    const number_t* n = (number_t*)first;
                    first = CAR(rest);
                    if (!NUMBERP(first)) return shared->_nil;
                    const number_t* n2 = (number_t*)first;
                    if (*n != *n2) return shared->_nil;
                    rest = CDR(rest);

                } while (!NILP(rest));

                return shared->t;
            }

            if (STRINGP(first)) {

                do {
                    const string_t* s = (string_t*)first;
                    first = CAR(rest);
                    if (!STRINGP(first)) return shared->_nil;
                    const string_t* s2 = (string_t*)first;
                    if (*s != *s2) return shared->_nil;
                    rest = CDR(rest);

                } while (!NILP(rest));

                return shared->t;
            }

            // for SYMBOL, CONS and OTHER_OBJECTS
            do {
                const obj_t* s = first;
                first = CAR(rest);
                const obj_t* s2 = first;

                // memory comparing.
                if (s != s2) return shared->_nil;

                rest = CDR(rest);

            } while (!NILP(rest));

            return shared->t;

        }

        obj_t* smaller_than(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                return shared->t;
            }

            obj_t* first = CAR(args);
            obj_t* rest  = CDR(args);

            if (NILP(rest)) {
                return shared->t;
            }

            if (NUMBERP(first)) {

                do {
                    const number_t* n = (number_t*)first;
                    first = CAR(rest);
                    if (!NUMBERP(first)) CALLERROR("primitive procedure '<' some arguments are invalid.");
                    const number_t* n2 = (number_t*)first;
                    if (*n >= *n2) return shared->_nil;
                    rest = CDR(rest);

                } while (!NILP(rest));

                return shared->t;
            }

            CALLERROR("primitive procedure '<' some arguments are invalid.");
        }

        obj_t* smaller_than_or_equal(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                return shared->t;
            }

            obj_t* first = CAR(args);
            obj_t* rest  = CDR(args);

            if (NILP(rest)) {
                return shared->t;
            }

            if (NUMBERP(first)) {

                do {
                    const number_t* n = (number_t*)first;
                    first = CAR(rest);
                    if (!NUMBERP(first)) CALLERROR("primitive procedure '<=' some arguments are invalid.");
                    const number_t* n2 = (number_t*)first;
                    if (*n > *n2) return shared->_nil;
                    rest = CDR(rest);

                } while (!NILP(rest));

                return shared->t;
            }

            CALLERROR("primitive procedure '<=' some arguments are invalid.");
        }


        obj_t* bigger_than(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                return shared->t;
            }

            obj_t* first = CAR(args);
            obj_t* rest  = CDR(args);

            if (NILP(rest)) {
                return shared->t;
            }

            if (NUMBERP(first)) {

                do {
                    const number_t* n = (number_t*)first;
                    first = CAR(rest);
                    if (!NUMBERP(first)) CALLERROR("primitive procedure '>' some arguments are invalid.");
                    const number_t* n2 = (number_t*)first;
                    if (*n <= *n2) return shared->_nil;
                    rest = CDR(rest);

                } while (!NILP(rest));

                return shared->t;
            }

            CALLERROR("primitive procedure '>' some arguments are invalid.");
        }

        obj_t* bigger_than_or_equal(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                return shared->t;
            }

            obj_t* first = CAR(args);
            obj_t* rest  = CDR(args);

            if (NILP(rest)) {
                return shared->t;
            }

            if (NUMBERP(first)) {

                do {
                    const number_t* n = (number_t*)first;
                    first = CAR(rest);
                    if (!NUMBERP(first)) CALLERROR("primitive procedure '>=' some arguments are invalid.");
                    const number_t* n2 = (number_t*)first;
                    if (*n < *n2) return shared->_nil;
                    rest = CDR(rest);

                } while (!NILP(rest));

                return shared->t;
            }

            CALLERROR("primitive procedure '>=' some arguments are invalid.");
        }


        obj_t* no(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                CALLERROR("wrong number of arguments 'no' requires 1");
            }

            obj_t* first = CAR(args);
            obj_t* rest  = CDR(args);

            if (!NILP(rest)) {
                CALLERROR("wrong number of arguments 'no' requires 1");
            }

            if (NILP(first)) {
                return shared->t;
            }

            return shared->_nil;
        }

        obj_t* car(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                CALLERROR("wrong number of arguments 'car' requires 1");
            }

            obj_t* first = CAR(args);
            obj_t* rest  = CDR(args);

            if (!NILP(rest)) {
                CALLERROR("wrong number of arguments 'car' requires 1");
            }

            if (NILP(first)) {
                return shared->_nil;
            }

            if (CONSP(first)) {
                return CAR(first);
            }

            CALLERROR("'car' the argument is invalid.");

        }

        obj_t* cdr(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                CALLERROR("wrong number of arguments. 'cdr' expects 1");
            }

            obj_t* first = CAR(args);
            obj_t* rest  = CDR(args);

            if (!NILP(rest)) {
                CALLERROR("wrong number of arguments. 'cdr' expects 1");
            }

            if (NILP(first)) {
                return shared->_nil;
            }

            if (CONSP(first)) {
                return CDR(first);
            }

            CALLERROR("'cdr' the argument is invalid.");

        }

        obj_t* cons(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                CALLERROR("wrong number of arguments. 'cons' expects 2, given 0.");
            }

            obj_t* first    = CAR(args);
            obj_t* rest     = CDR(args);

            if (!NILP(rest)) {
                obj_t* rest2 = CDR(rest);

                if (!NILP(rest2)) {
                    CALLERROR("wrong number of arguments. 'cons' expects 2, given 3 or more.");
                }

                return (new cons_t(first, CAR(rest)));
            }

            CALLERROR("wrong number of arguments. 'cons' expects 2, given 1.");
        }

        obj_t* acons(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                CALLERROR("wrong number of arguments. 'acons' expects 1");

            } else if (!NILP(CDR(args))) {
                CALLERROR("wrong number of arguments. 'acons' expects 1, given 2 or more.");

            }

            return ( CONSP(CAR(args)) ?
                     reinterpret_cast<obj_t*>(shared->t) :
                     reinterpret_cast<obj_t*>(shared->_nil) );
        }


        obj_t* list(obj_t* args, Shared* shared)
        {
            obj_t* ret = shared->_nil;

            while (!NILP(args)) {
                ret   = new cons_t(CAR(args), ret);
                args  = CDR(args);
            }

            obj_t* tmp = shared->_nil;
            obj_t* tmp2;

            while (!NILP(ret)) {
                tmp2 = CDR(ret);
                ((cons_t*)ret)->set_cdr(tmp);
                tmp = ret;
                ret = tmp2;
            }
            ret = tmp;

            return ret;
        }

        obj_t* len(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                CALLERROR("wrong number of arguments. 'len' expects 1");

            } else if (!NILP(CDR(args))) {
                CALLERROR("wrong number of arguments. 'len' expects 1, given 2 or more.");

            }

            obj_t* first = CAR(args);

            if (NILP(first)) return new number_t((int64_t)0);

            if (CONSP(first)) {
                int64_t size = 0;
                obj_t* rest = first;

                while (!NILP(rest)) {
                    if (!CONSP(rest)) {
                        goto error_in_len;
                    }
                    rest = CDR(rest);
                    size++;
                }

                return new number_t(size);
            }

        error_in_len:
            {
                char buf[256] = { 0 };
                first->print(buf, 256);

                CALLERROR("proper list required, but got %s", buf);
            }
        }

        obj_t* load(obj_t* args, Shared* shared)
        {
            if (NILP(args)) {
                CALLERROR("wrong number of arguments 'load' requires 1");
            }

            obj_t* first = CAR(args);
            obj_t* rest  = CDR(args);

            if (!NILP(rest)) {
                CALLERROR("wrong number of arguments 'load' requires 1");
            }

            if(!STRINGP(first))
                CALLERROR("wrong type of argument 'load' requires string");

            const char* fname = ((const string_t*)first)->c_str();

            int fd = open(fname, O_RDONLY);
            if ( fd < 0 ){
                CALLERROR("the file couldn't open (%s)", strerror(errno));
            }

            shared->reader->save_state();
            shared->reader->set_source(fd);

            while(1) {
                obj_t* obj = shared->reader->read();

                if (obj == shared->reader->S_EOF) break;

                shared->evaluator->eval(obj, shared->global_env);
            }

            close(fd);
            shared->reader->resolv_state();

            return shared->undef;
        }

        obj_t* uniq(obj_t* args, Shared* shared)
        {
            if (!NILP(args)) {
                CALLERROR("procedure ar-gensym: expects no arguments. given 1 or more.");
            }

            // logical max is gs18446744073709551615
            char buf[25]; buf[0] = '\0';
            snprintf(buf, 25, "gs%llu", (++(shared->gs_acc)));

            return const_cast<symbol_t*>(shared->symbols->get(buf));
        }

        obj_t* disp(obj_t* args, Shared* shared)
        {
            if (NILP(args))
                return shared->undef;

            /*
            if (!NILP(CDR(args))) {
                // TODO output port.
                CALLERROR("output definition is not surported yet. please wait.");
            }
            */

            obj_t* first = CAR(args);
            obj_t* obj   = first;
            obj_t* rest  = args;

            while (!NILP(rest)) {
                if (!CONSP(rest)) {
                    goto error_in_disp;
                }
                obj = CAR(rest);
                obj->print(stdout);
                fprintf(stdout, " ");
                rest = CDR(rest);
            }

            fprintf(stdout, "\n");

            return shared->undef;

        error_in_disp:
            {
                char buf[256] = { 0 };
                first->print(buf, 256);

                CALLERROR("proper list required, but got %s", buf);
            }

        }

        void setup_primitives( std::vector<const symbol_t*>* variables,
                               std::vector<obj_t*>* values,
                               Shared* shared)
        {
            variables->push_back(shared->symbols->get("eval"));
            values->push_back(new subr_t("eval", (void*)arc_eval));

            variables->push_back(shared->symbols->get("apply"));
            values->push_back(new subr_t("apply", (void*)arc_apply));

            variables->push_back(shared->symbols->get("compile"));
            values->push_back(new subr_t("compile", (void*)compile));

            variables->push_back(shared->symbols->get("+"));
            values->push_back(new subr_t("+", (void*)plus));

            variables->push_back(shared->symbols->get("-"));
            values->push_back(new subr_t("-", (void*)minus));

            variables->push_back(shared->symbols->get("*"));
            values->push_back(new subr_t("*", (void*)multiple));

            variables->push_back(shared->symbols->get("/"));
            values->push_back(new subr_t("/", (void*)devide));

            variables->push_back(shared->symbols->get("is"));
            values->push_back(new subr_t("is", (void*)equal));

            variables->push_back(shared->symbols->get("<"));
            values->push_back(new subr_t("<", (void*)smaller_than));

            variables->push_back(shared->symbols->get("<="));
            values->push_back(new subr_t("<=", (void*)smaller_than_or_equal));

            variables->push_back(shared->symbols->get(">"));
            values->push_back(new subr_t(">", (void*)bigger_than));

            variables->push_back(shared->symbols->get(">="));
            values->push_back(new subr_t(">=", (void*)bigger_than_or_equal));

            variables->push_back(shared->symbols->get("no"));
            values->push_back(new subr_t("no", (void*)no));

            variables->push_back(shared->symbols->get("car"));
            values->push_back(new subr_t("car", (void*)car));

            variables->push_back(shared->symbols->get("cdr"));
            values->push_back(new subr_t("cdr", (void*)cdr));

            variables->push_back(shared->symbols->get("cons"));
            values->push_back(new subr_t("cons", (void*)cons));

            variables->push_back(shared->symbols->get("acons"));
            values->push_back(new subr_t("acons", (void*)acons));

            variables->push_back(shared->symbols->get("list"));
            values->push_back(new subr_t("list", (void*)list));

            variables->push_back(shared->symbols->get("len"));
            values->push_back(new subr_t("len", (void*)len));

            variables->push_back(shared->symbols->get("load"));
            values->push_back(new subr_t("load", (void*)load));

            variables->push_back(shared->symbols->get("uniq"));
            values->push_back(new subr_t("uniq", (void*)uniq));

            variables->push_back(shared->symbols->get("disp"));
            values->push_back(new subr_t("disp", (void*)disp));

            return;
        }
    }
}
