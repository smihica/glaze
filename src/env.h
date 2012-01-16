#ifndef GLAZE__ENV_H_
#define GLAZE__ENV_H_

#include "core.h"
#include "object.h"
#include "shared.h"

namespace glaze {

    typedef std::map< const uintptr_t,
                      obj_t*,
                      std::less<uintptr_t>,
                      gc_allocator<std::pair<const uintptr_t, obj_t*> > > ptr_object_table_t;

    class frame_t :
        public gc_cleanup /* using boehmGC */
    {
    public:
        frame_t();
        frame_t(std::vector<const symbol_t*>* variables, std::vector<obj_t*>* values);
        ~frame_t();

        void add_binding(const symbol_t* variable, obj_t* value);
        bool rem_binding(const symbol_t* variable);
        bool change_binding(const symbol_t* variable, obj_t* value);

        obj_t* lookup(const symbol_t* variable);
        void   print();

    private:
        ptr_object_table_t m_table;
    };

    class env_t :
        public gc_cleanup /* using boehmGC */
    {
    public:
        env_t();
        env_t(const env_t& env);
        ~env_t();
        int extend(std::vector<const symbol_t*>* variables, std::vector<obj_t*>* values);
        int extend(obj_t* params, obj_t* args);
        int extend(frame_t* frame);
        int extend();

        void enclose();

        obj_t* lookup(const symbol_t* variable);
        void set(const symbol_t* variable, obj_t* value);
        void define(const symbol_t* variable, obj_t* value);
        void assign(const symbol_t* variable, obj_t* value);
        void unbind(const symbol_t* variable);

        void error(const char* fname, unsigned int line, const char* fmt, ...);

        const std::vector<frame_t*, gc_allocator<frame_t*> >& get_frames() const;

        enum extend_result {
            ARGUMENTS_RIGHT    = 0,
            ARGUMENTS_TOO_FEW  = 1,
            ARGUMENTS_TOO_MANY = 2,
        };

    private:
        std::vector<frame_t*, gc_allocator<frame_t*> > m_frames;
        Shared* shared;

    };

}

#endif // GLAZE__ENV_H_
