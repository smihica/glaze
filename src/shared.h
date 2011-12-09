#ifndef GLAZE__SHARED_H_
#define GLAZE__SHARED_H_

#include "core.h"
#include "object.h"

namespace glaze {

    class env_t;
    class symbol_table;
    class evaluator_t;
    class reader_t;

    typedef struct shared_
    {
        nil_t*          _nil;
        t_t*            t;
        undef_t*        undef;
        env_t*          global_env;

        symbol_table*   symbols;
        evaluator_t*    evaluator;
        reader_t*       reader;
    } Shared;

}

#endif // GLAZE__SHARED_H_
