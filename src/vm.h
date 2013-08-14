#ifndef GLAZE__VM_H_
#define GLAZE__VM_H_

#include "core.h"
#include "object.h"
#include "symbol_table.h"
#include "context.h"
#include "assembler.h"

#define STACK_SIZE 10000
#define OVERFLOW_CHECKING

namespace glaze {
    class VM {
    public:
        VM(Context* cont) {
            context = cont;
        }
        ~VM();

        Object* execute(Code* code, size_t len, Object* a, size_t f, Object* c, size_t s);
        void dumpAssembly(FILE* fp, Code* code, size_t len);
        void error(const char* fname, unsigned int line, const char* fmt, ...);

    private:

        Context* context;

        inline void index_set(size_t s, int i, Object* v) {
            intptr_t ptr = reinterpret_cast<intptr_t>(v);
            stack[s-i-2] = ptr;
        }
        inline Object* index(size_t s, int i) {
            intptr_t ptr = stack[s-i-2];
            return reinterpret_cast<Object*>(ptr);
        }
        inline intptr_t index_int(size_t s, int i) {
            intptr_t ptr = stack[s-i-2];
            return ptr;
        }
        inline size_t push(Object* v, size_t s) {
            intptr_t ptr = reinterpret_cast<intptr_t>(v);
            stack[s] = ptr;
            return s+1;
        }
        inline size_t push(intptr_t v, size_t s) {
            stack[s] = v;
            return s+1;
        }
        inline size_t shift_args(int n, int n_args, size_t s) {
            int m = n_args+1;
            for (int i=n-1; i>-2; i--) // shift 'ARGLEN x' too.
                index_set(s, i+m, index(s, i));
            return s - m;
        }
        intptr_t stack[STACK_SIZE];
    };
}

#endif // GLAZE__VM_H_
