#ifndef GLAZE__PRIMITIVES_H_
#define GLAZE__PRIMITIVES_H_

#include "core.h"
#include "object.h"
#include "shared.h"

namespace glaze {
    namespace primitives {

        void primitives_error(const char* fname, unsigned int line, const char* fmt, ...);

        // ---------

        obj_t* arc_eval(obj_t* args, Shared* shared);
        obj_t* arc_apply(obj_t* args, Shared* shared);
        obj_t* plus(obj_t* args, Shared* shared);
        obj_t* minus(obj_t* args, Shared* shared);
        obj_t* multiple(obj_t* args, Shared* shared);
        obj_t* devide(obj_t* args, Shared* shared);
        obj_t* equal(obj_t* args, Shared* shared);
        obj_t* smaller_than(obj_t* args, Shared* shared);
        obj_t* smaller_than_or_equal(obj_t* args, Shared* shared);
        obj_t* bigger_than(obj_t* args, Shared* shared);
        obj_t* bigger_than_or_equal(obj_t* args, Shared* shared);
        obj_t* no(obj_t* args, Shared* shared);
        obj_t* car(obj_t* args, Shared* shared);
        obj_t* cdr(obj_t* args, Shared* shared);
        obj_t* cons(obj_t* args, Shared* shared);
        obj_t* acons(obj_t* args, Shared* shared);
        obj_t* list(obj_t* args, Shared* shared);
        obj_t* len(obj_t* args, Shared* shared);
        obj_t* load(obj_t* args, Shared* shared);
        obj_t* uniq(obj_t* args, Shared* shared);
        obj_t* disp(obj_t* args, Shared* shared);

        void setup_primitives(
            std::vector<const symbol_t*>* variables,
            std::vector<obj_t*>* values,
            Shared* shared);
    }
}

#endif // GLAZE__PRIMITIVES_H_
