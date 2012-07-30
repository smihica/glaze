#ifndef GLAZE__SYMBOL_TABLE_H_
#define GLAZE__SYMBOL_TABLE_H_

#include "core.h"

namespace glaze {

    class SymbolTable {
    public:
        SymbolTable() {};
        ~SymbolTable() { clear(); };
        const Symbol* get(const char* name);
        void clear();
        void print();

    private:
        std::map<std::string, const Symbol*> m_table;
    };

}

#endif // GLAZE__SYMBOL_TABLE_H_
