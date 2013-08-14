#ifndef GLAZE__ENVIRONMENT_H_
#define GLAZE__ENVIRONMENT_H_

#include "core.h"
#include "object.h"

namespace glaze {

    typedef std::map< const uintptr_t,
                      Object*,

    #ifdef REQUIRE_GC_
                      std::less<uintptr_t>,
                      traceable_allocator<std::pair<const uintptr_t, Object*> >
    #endif
                      > ptr_object_table_t;

    class Environment

#ifdef REQUIRE_GC_
        : public gc_cleanup /* using boehmGC */
#endif

    {
    public:
        Environment();
        Environment(std::vector<const Symbol*>* variables, std::vector<Object*>* values);
        ~Environment();

        inline Object* get(const Symbol* variable);
        inline void    set(const Symbol* variable, Object* value);
        inline bool    remove(const Symbol* variable);
        void           print();

    private:
        ptr_object_table_t m_table;
    };
}

#endif // GLAZE__ENVIRONMENT_H_
