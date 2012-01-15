#include "core.h"
#include "object.h"
#include "shared.h"
#include "eval.h"
#include "env.h"
#include "symbol_table.h"

namespace glaze {

    evaluator_t::evaluator_t(Shared* sh)
    {
        shared = sh;

        special_symbols[0] = shared->symbols->get("quote");
        special_symbols[1] = shared->symbols->get("set");
        special_symbols[2] = shared->symbols->get("if");
        special_symbols[3] = shared->symbols->get("fn");
        special_symbols[4] = shared->symbols->get("def");
        special_symbols[5] = shared->symbols->get("%nameless_mac");
        special_symbols[6] = shared->symbols->get("mac");
        special_symbols[7] = shared->symbols->get("do");
        special_symbols[8] = shared->symbols->get("and");
        special_symbols[9] = shared->symbols->get("or");
    }

    obj_t*
    evaluator_t::eval(obj_t* exp, env_t* env) {
        evaluator_t::special_t flag;

        if (is_self_evaluating(exp))
            return exp;

        else if (is_variable(exp)) {
            return env->lookup((const symbol_t*)exp);
        }

        else if ((flag = is_special(exp)) != evaluator_t::NOT_SPECIAL)
            return eval_special(flag, exp, env);

        else if (is_application(exp)) {
            obj_t* top = (eval(CAR(exp), env));

            if (FUNCTIONP(top)) {
                return apply((function_t*)top,
                             list_of_values(CDR(exp), env));
            }

            if (MACROP(top)) {
                obj_t* expr = expand_macro((macro_t*)top, CDR(exp));
                return eval(expr, env);
            }

            char buf[1024];
            top->print(buf, 1024);

            CALLERROR("attempt call non-procedure -- EVAL %s", buf);
        }
        else
        {
            char buf[1024];
            exp->print(buf, 1024);

            CALLERROR("unknown expression type -- EVAL %s", buf);
        }

        return NULL;
    }

    inline bool
    evaluator_t::is_self_evaluating(const obj_t* exp)
    {
        return (NILP(exp) || NUMBERP(exp) || STRINGP(exp) || TP(exp) || UNDEFP(exp));
    }


    inline bool
    evaluator_t::is_variable(const obj_t* exp)
    {
        return SYMBOLP(exp);
    }

    evaluator_t::special_t
    evaluator_t::is_special(const obj_t* exp)
    {

        if (!CONSP(exp)){
            char buf[1024];
            exp->print(buf, 1024);
            CALLERROR("unknown expression type -- IS_SPECIAL %s", buf);
        }

        obj_t* top = CAR(exp);

        if(!(SYMBOLP(top))) return NOT_SPECIAL;

        const symbol_t* s = (symbol_t*)top;

        if ( s == special_symbols[0] )
            return QUOTED;

        else if ( s == special_symbols[1] )
            return ASSIGNMENT;

        else if ( s == special_symbols[2] )
            return IF_STAT;

        else if ( s == special_symbols[3] )
            return FN_STAT;

        else if ( s == special_symbols[4] )
            return DEF_STAT;

        else if ( s == special_symbols[5] )
            return NLMAC_STAT;

        else if ( s == special_symbols[6] )
            return MAC_STAT;

        else if ( s == special_symbols[7] )
            return DO_STAT;

        else if ( s == special_symbols[8] )
            return AND_STAT;

        else if ( s == special_symbols[9] )
            return OR_STAT;

        else
            return NOT_SPECIAL;

    }

    inline bool
    evaluator_t::is_true(const obj_t* exp)
    {
        return !NILP(exp);
    }

    inline bool
    evaluator_t::is_application(const obj_t* exp)
    {
        return CONSP(exp);
    }


    obj_t*
    evaluator_t::list_of_values(obj_t* exps, env_t* env)
    {
        if (NILP(exps))
            return shared->_nil;

        if (CONSP(exps))
            return new cons_t( eval(CAR(exps), env),
                               list_of_values( CDR(exps), env));
        {
            char buf[1024];
            exps->print(buf, 1024);

            CALLERROR("arguments must be a proper list -- LIST_OF_VALUES %s", buf);
        }
    }


    obj_t*
    evaluator_t::eval_special(evaluator_t::special_t flag, obj_t* exp, env_t* env)
    {
        switch(flag) {
        case QUOTED:
            return eval_quote(exp, env);
        case ASSIGNMENT:
            return eval_assignment(exp, env);
        case IF_STAT:
            return eval_if(exp, env);
        case FN_STAT:
            return eval_fn(exp, env);
        case DEF_STAT:
            return eval_def(exp, env);
        case NLMAC_STAT:
            return eval_nameless_mac(exp, env);
        case MAC_STAT:
            return eval_mac(exp, env);
        case DO_STAT:
            return eval_do(exp, env);
        case AND_STAT:
            return eval_and(exp, env);
        case OR_STAT:
            return eval_or(exp, env);
        default: {
            char buf[1024];
            exp->print(buf, 1024);

            CALLERROR("This is not a special expression -- EVAL_SPECIAL %s", buf);
        }
        }

        return shared->undef;
    }

    inline obj_t*
    evaluator_t::eval_quote(obj_t* exp, env_t* env)
    {
        if (NILP(CDR(exp)) || !NILP(CDDR(exp))) {
            char buf[1024];
            exp->print(buf, 1024);

            CALLERROR("Bad syntax -- EVAL_QUOTE %s", buf);
        }

        return CADR(exp);
    }

    obj_t*
    evaluator_t::eval_assignment(obj_t* exp, env_t* env)
    {
        obj_t* rest;
        obj_t* second;
        obj_t* third;
        obj_t* ret;
        const symbol_t* sym;

        rest = CDR(exp);
        if (NILP(rest) || !CONSP(rest)) goto error_eval_assignment;

        second =  CAR(rest);
        if (!is_variable(second)) goto error_eval_assignment;

        rest = CDR(rest);
        if (NILP(rest) || !CONSP(rest)) goto error_eval_assignment;

        third = CAR(rest);
        ret = eval(third, env);
        sym = (const symbol_t*)second;

        if (CLOSUREP(ret))
            ((closure_t*)ret)->set_name(sym->name());
        else if (MACROP(ret))
            ((macro_t*)ret)->set_name(sym->name());

        env->assign(sym, ret);

        return ret;

    error_eval_assignment:
        char buf[1024];
        exp->print(buf, 1024);

        CALLERROR("error in set! : Expected variable and single expression -- EVAL_ASSIGNMENT %s", buf);

        return shared->undef;
    }

    obj_t*
    evaluator_t::eval_if(obj_t* exp, env_t* env)
    {
        size_t  index = 0;
        bool    skip = true;
        obj_t*  rest;
        obj_t*  first;

        rest = CDR(exp);
        if (NILP(rest)) return shared->_nil;

        index++;

        while (!NILP(rest)) {
            first = CAR(rest);
            if ((index % 2) != 0) {

                if (NILP(CDR(rest))) // last item. this is not a condition.
                    return eval(first, env);

                // condition
                if (is_true(eval(first, env))) {
                    skip = false;
                }
            } else {
                if (!skip) return eval(first, env);
            }

            index++;
            rest = CDR(rest);
        }

        return shared->_nil;
    }

/*
  (%nameless_mac)
  (%nameless_mac a)
  (%nameless_mac a b)
  (%nameless_mac "ab" c)
*/
    obj_t*
    evaluator_t::eval_nameless_mac(obj_t* exp, env_t* env)
    {
        obj_t* second;
        obj_t* rest;
        macro_t* ret;

        if (NILP(CDR(exp))) goto error_eval_nameless_mac;

        second = CADR(exp);
        if (NILP(second)) goto error_eval_nameless_mac;
        rest = CDDR(exp);

        ret = new macro_t(second, rest, env);

        char pos[32];
        snprintf(pos, 31, "#%p", ret);
        ret->set_name(pos);

        return ret;

    error_eval_nameless_mac:
        char buf[1024];
        exp->print(buf, 1024);

        CALLERROR("error in nameless_mac : Expected formals and body -- EVAL_NAMELESS_MAC %s", buf);

        return shared->undef;
    }

/*
  (mac)
  (mac a)
  (mac a b)
  (mac a b)
  (mac "ab" c)
*/
    obj_t*
    evaluator_t::eval_mac(obj_t* exp, env_t* env)
    {
        obj_t* second;
        obj_t* third;
        obj_t* rest;
        obj_t* definition_value;
        obj_t* obj;

        const symbol_t* definition_variable;

        if (NILP(CDR(exp))) goto error_eval_mac1;

        second = CADR(exp);
        if (NILP(second)) goto error_eval_mac1;

        if (NILP(CDDR(exp))) goto error_eval_mac1;

        third = CADDR(exp);
        if (!(SYMBOLP(third) || CONSP(third))) {
            char buf[1024];
            exp->print(buf, 1024);

            CALLERROR("mac: expected symbol or cons for second clause -- EVAL_MAC %s", buf);
        }

        rest = CDDDR(exp);

        if (!is_variable(second)) goto error_eval_mac2;

        definition_variable = (const symbol_t*)second;
        definition_value = new cons_t(const_cast<symbol_t*>(shared->symbols->get("%nameless_mac")),
                                      new cons_t(third, rest));

        obj = eval(definition_value, env);

        if (MACROP(obj)) {
            ((macro_t*)obj)->set_name(definition_variable->name());
        }

        env->define(definition_variable, obj);

        return obj;

    error_eval_mac1:
        char buf1[1024];
        exp->print(buf1, 1024);

        CALLERROR("mac: expected at least 2 arguments -- EVAL_MAC %s", buf1);

    error_eval_mac2:
        char buf2[1024];
        exp->print(buf2, 1024);

        CALLERROR("mac: expected symbol for first clause -- EVAL_MAC %s", buf2);

        return shared->undef;
    }

/*
  (fn)        -> NG
  (fn a)      -> OK
  (fn a b)    -> OK
  (fn "ab" c) -> NG
*/
    obj_t*
    evaluator_t::eval_fn(obj_t* exp, env_t* env)
    {
        obj_t* second;
        obj_t* rest;
        closure_t* ret;

        if (NILP(CDR(exp))) {
            CALLERROR("Error: fn expects at least 1 argument. given (fn)");
        }

        second = CADR(exp);
        rest = CDDR(exp);

        if (!(CONSP(second) || NILP(second) || SYMBOLP(second))) {
            CALLERROR("Error: the argument of fn must be able to understand list.");
        }

        ret = new closure_t(second, rest, env);

        char pos[32];
        snprintf(pos, 31, "#%p", ret);
        ret->set_name(pos);

        return ret;

    error_eval_fn:
        char buf[1024];
        exp->print(buf, 1024);

        CALLERROR("error in fn : Expected formals and body -- EVAL_FN %s", buf);

        return shared->undef;
    }

/*
  (def)   -> error
  (def a) -> error
  (def a b)
  (def a b)
  (def "ab" c)
*/
    obj_t*
    evaluator_t::eval_def(obj_t* exp, env_t* env)
    {
        obj_t* second;
        obj_t* third;
        obj_t* rest;
        obj_t* definition_value;
        obj_t* obj;
        const symbol_t* definition_variable;

        if (NILP(CDR(exp))) goto error_eval_def1;

        second = CADR(exp);
        if (NILP(second)) goto error_eval_def1;

        if (NILP(CDDR(exp))) goto error_eval_def1;

        third = CADDR(exp);
        if (!(CONSP(third) || NILP(third) || SYMBOLP(third))) {
            char buf[1024];
            exp->print(buf, 1024);

            CALLERROR("def: expected symbol or cons for second clause -- EVAL_DEF %s", buf);

        }

        rest = CDDDR(exp);

        if (!is_variable(second)) goto error_eval_def2;

        definition_variable = (const symbol_t*)second;
        definition_value = new cons_t(const_cast<symbol_t*>(shared->symbols->get("fn")),
                                      new cons_t(third, rest));

        obj = eval(definition_value, env);

        if (CLOSUREP(obj)) {
            ((closure_t*)obj)->set_name(definition_variable->name());
        }

        env->define(definition_variable, obj);

        return obj;

    error_eval_def1:
        char buf1[1024];
        exp->print(buf1, 1024);

        CALLERROR("def: expected at least 2 arguments -- EVAL_DEF %s", buf1);

    error_eval_def2:
        char buf2[1024];
        exp->print(buf2, 1024);

        CALLERROR("def: expected symbol for first clause -- EVAL_DEF %s", buf2);

        return shared->undef;
    }

    obj_t*
    evaluator_t::eval_sequence(obj_t* exps, env_t* env)
    {
        if (NILP(exps)) return shared->_nil;

        obj_t* rest = CDR(exps);

        if (NILP(rest)) return eval(CAR(exps), env);

        eval(CAR(exps), env);

        return eval_sequence(rest, env);
    }

    obj_t*
    evaluator_t::eval_do(obj_t* exp, env_t* env)
    {
        obj_t* rest = CDR(exp);

        if (NILP(rest))
            return shared->_nil;

        return eval_sequence(rest, env);
    }

    obj_t*
    evaluator_t::eval_and_sequence(obj_t* exps, env_t* env)
    {
        obj_t* last = shared->t;

        if (CONSP(exps)) {
            while (!NILP(exps)) {

                obj_t* evaled = eval(CAR(exps), env);

                if(!is_true(evaled))
                    return shared->_nil;

                last = evaled;
                exps = CDR(exps);
            }
        }

        return last;
    }

    inline obj_t*
    evaluator_t::eval_and(obj_t* exp, env_t* env)
    {
        return eval_and_sequence(CDR(exp), env);
    }

    obj_t*
    evaluator_t::eval_or_sequence(obj_t* exps, env_t* env)
    {

        if (CONSP(exps)) {
            while (!NILP(exps)) {

                obj_t* evaled = eval(CAR(exps), env);

                if(is_true(evaled)) return evaled;

                exps = CDR(exps);
            }
        }

        return shared->_nil;
    }

    inline obj_t*
    evaluator_t::eval_or(obj_t* exp, env_t* env)
    {
        return eval_or_sequence(CDR(exp), env);
    }


// apply
    obj_t*
    evaluator_t::apply(function_t* proc, obj_t* args)
    {

        if (is_primitive_procedure(proc)) {
            try {
                return apply_primitive_procedure((subr_t*)proc, args);
            } catch (char* message) {
                CALLERROR("APPLY\n---- in primitive procedure '%s' -- %s", ((subr_t*)proc)->name(), message);
                free(message);
            }

        } else if (is_compound_procedure(proc)) {
            return apply_compound_procedure((closure_t*)proc, args);

        } else {
            char buf[1024];
            proc->print(buf, 1024);

            CALLERROR("unknown procedure type -- APPLY %s", buf);
        }

        return shared->undef;
    }

    obj_t*
    evaluator_t::apply_primitive_procedure(subr_t* proc, obj_t* args)
    {
        obj_t* (*func)(obj_t* arg, Shared* shared);
        func = (obj_t* (*)(obj_t*, Shared*))proc->func();

        return (*func)(args, shared);
    }

    inline bool
    evaluator_t::is_primitive_procedure(const function_t* proc)
    {
        return SUBRP(proc);
    }

    obj_t*
    evaluator_t::apply_compound_procedure(closure_t* proc, obj_t* args)
    {
        int extend_result = proc->env()->extend(proc->param(), args);
        if (extend_result != 0) {
            char buf[1024];
            proc->param()->print(buf, 1024);

            CALLERROR("too %s arguments supplied -- EXTEND : proc \"%s\" requires %s",
                      (extend_result == 1) ? "few" : "many",
                      proc->name(),
                      buf);
        }
        obj_t* ret = eval_sequence(proc->body(), proc->env());
        proc->env()->enclose();

        return ret;
    }

    inline bool
    evaluator_t::is_compound_procedure(const function_t* proc)
    {
        return CLOSUREP(proc);
    }


// macro
    obj_t*
    evaluator_t::expand_macro(macro_t* macro_fn, obj_t* exp)
    {
        int extend_result = macro_fn->env()->extend(macro_fn->param(), exp);
        if (extend_result != 0) {
            char buf[1024];
            macro_fn->param()->print(buf, 1024);

            CALLERROR("too %s arguments supplied -- EXTEND : macro \"%s\" requires %s",
                      (extend_result == 1) ? "few" : "many",
                      macro_fn->name(),
                      buf);
        }

        obj_t* ret = eval_sequence(macro_fn->body(), macro_fn->env());
        macro_fn->env()->enclose();

        return ret;
    }

    void evaluator_t::error(const char* fname, unsigned int line, const char* fmt, ...)
    {
        char fname_buf[32];
        remove_dir(fname, fname_buf, 32);

        va_list arg;
        va_start(arg, fmt);

        fprintf(stderr, "%s:%u !! ERROR in 'EVALUATOR' !! -- ", fname_buf, line);
        vfprintf(stderr, fmt, arg);
        fprintf(stderr, "\n\n");

        fflush(stderr);

        va_end(arg);

        throw "EVALUATOR_ERROR";
    }

}
