#ifndef GLAZE__ASSEMBLER_H_
#define GLAZE__ASSEMBLER_H_

#include "core.h"
#include "object.h"
#include "context.h"
#include "symbol_table.h"

namespace glaze {

    typedef uint64_t Code;

    class Assembler {
    public:
        Assembler(Context* cont)
        {
            symbol_table = cont->symbolTable;
            init();
        }

        ~Assembler();

        void gen(Object* codelist, Code* code, size_t* len);

        static const Code HALT          = 0x00;
        static const Code REFER_LOCAL   = 0x01;
        static const Code REFER_FREE    = 0x02;
        static const Code REFER_GLOBAL  = 0x03;
        static const Code INDIRECT      = 0x04;
        static const Code CONSTANT      = 0x05;
        static const Code CLOSE         = 0x06;
        static const Code BOX           = 0x07;
        static const Code TEST          = 0x08;
        static const Code ASSIGN_GLOBAL = 0x09;
        static const Code ASSIGN_LOCAL  = 0x0A;
        static const Code ASSIGN_FREE   = 0x0B;
        static const Code CONTI         = 0x0C;
        static const Code NUATE         = 0x0D;
        static const Code FRAME         = 0x0E;
        static const Code ARGUMENT      = 0x0F;
        static const Code ARGUMENT_LEN  = 0x10;
        static const Code SHIFT         = 0x11;
        static const Code HINTED_APPLY  = 0x12;
        static const Code APPLY         = 0x13;
        static const Code RETURN        = 0x14;

    private:

        SymbolTable* symbol_table;

        void init() {

#define MKSYM(NAME, STRING) NAME = const_cast<Symbol*>(symbol_table->get(STRING))

            MKSYM(HALT_SYM,          "halt");
            MKSYM(REFER_LOCAL_SYM,   "refer-local");
            MKSYM(REFER_FREE_SYM,    "refer-free");
            MKSYM(REFER_GLOBAL_SYM,  "refer-global");
            MKSYM(INDIRECT_SYM,      "indirect");
            MKSYM(CONSTANT_SYM,      "constant");
            MKSYM(CLOSE_SYM,         "close");
            MKSYM(BOX_SYM,           "box");
            MKSYM(TEST_SYM,          "test");
            MKSYM(ASSIGN_GLOBAL_SYM, "assign-global");
            MKSYM(ASSIGN_LOCAL_SYM,  "assign-local");
            MKSYM(ASSIGN_FREE_SYM,   "assign-free");
            MKSYM(CONTI_SYM,         "conti");
            MKSYM(NUATE_SYM,         "nuate");
            MKSYM(FRAME_SYM,         "frame");
            MKSYM(ARGUMENT_SYM,      "argument");
            MKSYM(ARGUMENT_LEN_SYM,  "argument-length");
            MKSYM(SHIFT_SYM,         "shift");
            MKSYM(HINTED_APPLY_SYM, "tco-hinted-apply");
            MKSYM(APPLY_SYM,         "apply");
            MKSYM(RETURN_SYM,        "return");

        }

        static const Symbol* HALT_SYM;
        static const Symbol* REFER_LOCAL_SYM;
        static const Symbol* REFER_FREE_SYM;
        static const Symbol* REFER_GLOBAL_SYM;
        static const Symbol* INDIRECT_SYM;
        static const Symbol* CONSTANT_SYM;
        static const Symbol* CLOSE_SYM;
        static const Symbol* BOX_SYM;
        static const Symbol* TEST_SYM;
        static const Symbol* ASSIGN_GLOBAL_SYM;
        static const Symbol* ASSIGN_LOCAL_SYM;
        static const Symbol* ASSIGN_FREE_SYM;
        static const Symbol* CONTI_SYM;
        static const Symbol* NUATE_SYM;
        static const Symbol* FRAME_SYM;
        static const Symbol* ARGUMENT_SYM;
        static const Symbol* ARGUMENT_LEN_SYM;
        static const Symbol* SHIFT_SYM;
        static const Symbol* HINTED_APPLY_SYM;
        static const Symbol* APPLY_SYM;
        static const Symbol* RETURN_SYM;

    };
}

#endif
