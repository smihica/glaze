#include "core.h"
#include "object.h"
#include "symbol_table.h"

namespace glaze {

    void SymbolTable::clear()
    {
/*
        std::map<std::string, const symbol_t*>::iterator it;
        for(it=m_table.begin(); it!=m_table.end(); it++)
        {
            delete (it->second); // gc delete
        }
*/
        m_table.clear();
    }

    const Symbol*
    SymbolTable::get(const char* name)
    {
        const Symbol* sym;
        std::string buf = std::string(name);

        std::pair<std::map<std::string, const Symbol*>::iterator, bool> pib =
            m_table.insert(std::pair<std::string, const Symbol*>(buf, NULL));

        if (!pib.second) {
            sym = pib.first->second;

        } else {
            sym = new Symbol(name, strlen(name));
            pib.first->second = sym;
        }

        return sym;
    }

    void
    SymbolTable::print()
    {
        std::map<std::string, const Symbol*>::iterator it;
        printf("print symbol-table start...\n");

        for(it=m_table.begin(); it!=m_table.end(); it++)
        {
            char buf[256];
            it->second->print(buf, 256);

            printf("    %s : %s ;\n", it->first.c_str(), buf);
        }

        printf("end...\n");
    }
}
