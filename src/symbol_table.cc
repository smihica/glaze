#include "core.h"
#include "object.h"
#include "symbol_table.h"

namespace glaze {

    symbol_table::~symbol_table()
    {
        clear();
    }

    void symbol_table::clear()
    {
        std::map<std::string, const symbol_t*>::iterator it;
/*
        for(it=m_table.begin(); it!=m_table.end(); it++)
        {
            delete (it->second); // gc delete
        }
*/

        m_table.clear();
    }

    const symbol_t*
    symbol_table::get(const char* name)
    {
        const symbol_t* sym;
        std::string buf = std::string(name);

        std::pair<std::map<std::string, const symbol_t*>::iterator, bool> pib = m_table.insert(std::pair<std::string, const symbol_t*>(buf, NULL));

        if (!pib.second) {
            sym = pib.first->second;

        } else {
            sym = new symbol_t(name, strlen(name));
            pib.first->second = sym;
        }

        return sym;
    }

    void
    symbol_table::print()
    {
        std::map<std::string, const symbol_t*>::iterator it;
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
