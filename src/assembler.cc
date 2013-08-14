#include "core.h"
#include "object.h"
#include "assembler.h"

namespace glaze {

    const Symbol* Assembler::HALT_SYM;
    const Symbol* Assembler::REFER_LOCAL_SYM;
    const Symbol* Assembler::REFER_FREE_SYM;
    const Symbol* Assembler::REFER_GLOBAL_SYM;
    const Symbol* Assembler::INDIRECT_SYM;
    const Symbol* Assembler::CONSTANT_SYM;
    const Symbol* Assembler::CLOSE_SYM;
    const Symbol* Assembler::BOX_SYM;
    const Symbol* Assembler::TEST_SYM;
    const Symbol* Assembler::ASSIGN_GLOBAL_SYM;
    const Symbol* Assembler::ASSIGN_LOCAL_SYM;
    const Symbol* Assembler::ASSIGN_FREE_SYM;
    const Symbol* Assembler::CONTI_SYM;
    const Symbol* Assembler::NUATE_SYM;
    const Symbol* Assembler::FRAME_SYM;
    const Symbol* Assembler::ARGUMENT_SYM;
    const Symbol* Assembler::ARGUMENT_LEN_SYM;
    const Symbol* Assembler::SHIFT_SYM;
    const Symbol* Assembler::HINTED_APPLY_SYM;
    const Symbol* Assembler::APPLY_SYM;
    const Symbol* Assembler::RETURN_SYM;

    void Assembler::gen(Object* codelist, Code* code, size_t* len) {

        size_t pc = 0;
        Object* instruction;
        Object* first;


        for (;;) {

            instruction = CAR(codelist);
            first = CAR(instruction);

            if (first->isSymbol()) {

                Symbol* s = (Symbol*)first;

                if (s == HALT_SYM) {
                    code[pc++] = HALT;
                    break;

                } else if (s == REFER_LOCAL_SYM) {
                    code[pc++] = REFER_LOCAL;
                    code[pc++] = ((Number*)(CADR(instruction)))->getFixnum();

                } else if (s == REFER_FREE_SYM) {
                    code[pc++] = REFER_FREE;
                    code[pc++] = ((Number*)(CADR(instruction)))->getFixnum();

                } else if (s == REFER_GLOBAL_SYM) {
                    code[pc++] = REFER_GLOBAL;
                    code[pc++] = (intptr_t)((Symbol*)(CADR(instruction)));

                } else if (s == INDIRECT_SYM) {
                    code[pc++] = INDIRECT;

                } else if (s == CONSTANT_SYM) {
                    code[pc++] = CONSTANT;
                    code[pc++] = (intptr_t)(CADR(instruction));

                } else if (s == CLOSE_SYM) {
                    code[pc++] = CLOSE;
                    code[pc++] = ((Number*)(CADR(instruction)))->getFixnum();
                    code[pc++] = ((Number*)(CADDR(instruction)))->getFixnum();
                    code[pc++] = ((Number*)(CADDR(CDR(instruction))))->getFixnum();

                } else if (s == BOX_SYM) {
                    // TODO
                    code[pc++] = BOX;
                    code[pc++] = ((Number*)(CADR(instruction)))->getFixnum();

                } else if (s == TEST_SYM) {
                    code[pc++] = TEST;
                    code[pc++] = ((Number*)(CADR(instruction)))->getFixnum();

                } else if (s == ASSIGN_GLOBAL_SYM) {
                    code[pc++] = ASSIGN_GLOBAL;
                    code[pc++] = (intptr_t)((Symbol*)(CADR(instruction)));

                } else if (s == ASSIGN_LOCAL_SYM) {
                    code[pc++] = ASSIGN_LOCAL;
                    code[pc++] = ((Number*)(CADR(instruction)))->getFixnum();

                } else if (s == ASSIGN_FREE_SYM) {
                    code[pc++] = ASSIGN_FREE;
                    code[pc++] = ((Number*)(CADR(instruction)))->getFixnum();

                } else if (s == CONTI_SYM) {
                    code[pc++] = CONTI;
                    code[pc++] = ((Number*)(CADR(instruction)))->getFixnum();

                } else if (s == NUATE_SYM) {

                    // TODO;

                } else if (s == FRAME_SYM) {
                    code[pc++] = FRAME;
                    code[pc++] = ((Number*)(CADR(instruction)))->getFixnum();

                } else if (s == ARGUMENT_SYM) {
                    code[pc++] = ARGUMENT;

                } else if (s == ARGUMENT_LEN_SYM) {
                    code[pc++] = ARGUMENT_LEN;
                    code[pc++] = ((Number*)(CADR(instruction)))->getFixnum();

                } else if (s == SHIFT_SYM) {
                    code[pc++] = SHIFT;
                    code[pc++] = ((Number*)(CADR(instruction)))->getFixnum();

                } else if (s == HINTED_APPLY_SYM) {
                    code[pc++] = HINTED_APPLY;

                } else if (s == APPLY_SYM) {
                    code[pc++] = APPLY;

                } else if (s == RETURN_SYM) {
                    code[pc++] = RETURN;

                } else {
                    fprintf(stderr, "Unknown Instruction -> ");
                    s->print(stderr);
                    fprintf(stderr, "\n");
                    break;
                }
            }

            codelist = CDR(codelist);

            if (codelist->isNil()) break;
        }

        *len = pc;
    }

}
