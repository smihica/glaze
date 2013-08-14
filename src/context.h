#ifndef GLAZE__CONTEXT_H_
#define GLAZE__CONTEXT_H_

#include "core.h"
#include "object.h"
#include "symbol_table.h"
// #include "environment.h"

namespace glaze {

    class SymbolTable;
    class Reader;
    class Assembler;
    class VM;

    class Context {
    public:

        SymbolTable *symbolTable;
        Reader      *reader;
        Assembler   *assembler;
        VM          *vm;
        // Environment *env;

        Context() {
            symbolTable = new SymbolTable();
        }

        ~Context() {
            delete symbolTable;
        }
    };
}

#endif // GLAZE__CONTEXT_H_
