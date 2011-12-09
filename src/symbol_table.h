#ifndef GLAZE__SYMBOL_TABLE_H_
#define GLAZE__SYMBOL_TABLE_H_

#include "core.h"

namespace glaze {

    class symbol_table {
    public:
        symbol_table() {};
        ~symbol_table();
        const symbol_t* get(const char* name);
        void      clear();
        void      print();

    private:
        std::map<std::string, const symbol_t*> m_table;

    };

}

#endif // GLAZE__SYMBOL_TABLE_H_
