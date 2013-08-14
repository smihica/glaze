#include "core.h"
#include "object.h"
#include "environment.h"

namespace glaze {

    Environment::Environment()
    {
    }

    Environment::Environment(std::vector<const Symbol*>* variables, std::vector<Object*>* values)
    {
        uintptr_t ptr;
        std::vector<const Symbol*>::iterator var_it;
        std::vector<Object*>::iterator val_it;

        for(var_it = variables->begin(), val_it = values->begin();
            (var_it != variables->end() && val_it != values->end());
             var_it++, val_it++)
        {
            ptr = reinterpret_cast<uintptr_t>(*var_it);
            m_table[ptr] = *val_it;
        }
    }

    Environment::~Environment()
    {
        m_table.clear();
    }

    Object*
    Environment::get(const Symbol* variable)
    {
        uintptr_t ptr = reinterpret_cast<uintptr_t>(variable);
        ptr_object_table_t::iterator it = m_table.find(ptr);

        return (it == m_table.end()) ? NULL : it->second;
    }

    void
    Environment::set(const Symbol* variable, Object* value)
    {
        uintptr_t ptr = reinterpret_cast<uintptr_t>(variable);
        m_table[ptr] = value;
    }

    bool
    Environment::remove(const Symbol* variable)
    {
        uintptr_t ptr = reinterpret_cast<uintptr_t>(variable);
        return (m_table.erase(ptr) > 0);
    }

    void
    Environment::print()
    {
        ptr_object_table_t::iterator it;

        for ( it = m_table.begin(); it != m_table.end(); it++ ) {
            const Symbol* symb = reinterpret_cast<const Symbol*>(it->first);
            Object* obj = it->second;

            symb->print();
            printf(" : ");
            obj->print();
            printf("\n");
        }

        fflush(stdout);

        return;
    }
}
