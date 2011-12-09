#ifndef GLAZE__EVAL_H_
#define GLAZE__EVAL_H_

#include "core.h"
#include "object.h"
#include "shared.h"

#define READ_STRING_SMALL_BUFFER_SIZE   1024
#define READ_NUMBER_BUFFER_SIZE         256
#define MAX_READ_SYMBOL_LENGTH          256

namespace glaze {

    class evaluator_t {
    public:
        evaluator_t(Shared* sh);

        obj_t* eval(obj_t* exp, env_t* env);
        obj_t* apply(function_t* proc, obj_t* args);

        enum special_t {
            NOT_SPECIAL     = 0,
            QUOTED          = 1,
            ASSIGNMENT      = 2,
            IF_STAT         = 3,
            FN_STAT         = 4,
            DEF_STAT        = 5,
            NLMAC_STAT      = 6,
            MAC_STAT        = 7,
            DO_STAT         = 8,
            AND_STAT        = 9,
            OR_STAT         = 10,
        };

    private:
        Shared* shared;

        const symbol_t* special_symbols[10];
        inline bool is_self_evaluating(const obj_t* exp);
        inline bool is_variable(const obj_t* exp);
        special_t is_special(const obj_t* exp);
        inline bool is_true(const obj_t* exp);
        inline bool is_application(const obj_t* exp);
        obj_t* list_of_values(obj_t* exps, env_t* env);

        obj_t* eval_special(special_t flag, obj_t* exp, env_t* env);
        inline obj_t* eval_quote(obj_t* exp, env_t* env);
        obj_t* eval_assignment(obj_t* exp, env_t* env);
        obj_t* eval_if(obj_t* exp, env_t* env);
        obj_t* eval_nameless_mac(obj_t* exp, env_t* env);
        obj_t* eval_mac(obj_t* exp, env_t* env);
        obj_t* eval_fn(obj_t* exp, env_t* env);
        obj_t* eval_def(obj_t* exp, env_t* env);
        obj_t* eval_do(obj_t* exp, env_t* env);
        obj_t* eval_sequence(obj_t* exps, env_t* env);
        obj_t* eval_and_sequence(obj_t* exps, env_t* env);
        inline obj_t* eval_and(obj_t* exp, env_t* env);
        obj_t* eval_or_sequence(obj_t* exps, env_t* env);
        inline obj_t* eval_or(obj_t* exp, env_t* env);

        // apply
        obj_t* apply_primitive_procedure(subr_t* proc, obj_t* args);
        obj_t* apply_compound_procedure(closure_t* proc, obj_t* args);

        inline bool is_primitive_procedure(const function_t* proc);
        inline bool is_compound_procedure(const function_t* proc);

        // macro
        obj_t* expand_macro(macro_t* macro_fn, obj_t* exp);

        void error(const char* fname, unsigned int line, const char* fmt, ...);
    };

}

#endif // GLAZE__EVAL_H_

