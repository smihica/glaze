#include "core.h"
#include "object.h"
#include "env.h"

namespace glaze {

/*
 * frame
 */
    frame_t::frame_t()
    {
    }

    frame_t::frame_t(std::vector<const symbol_t*>* variables, std::vector<obj_t*>* values)
    {
        uintptr_t ptr;
        std::vector<const symbol_t*>::iterator var_it;
        std::vector<obj_t*>::iterator val_it;

        for(var_it = variables->begin(), val_it = values->begin();
            (var_it != variables->end() && val_it != values->end());
             var_it++, val_it++)
        {
            ptr = reinterpret_cast<uintptr_t>(*var_it);
            m_table[ptr] = *val_it;
        }
    }

    frame_t::~frame_t()
    {
        m_table.clear();
    }

    void
    frame_t::add_binding(const symbol_t* variable, obj_t* value)
    {
        uintptr_t ptr = reinterpret_cast<uintptr_t>(variable);
        m_table[ptr] = value;

/*
        uintptr_t ptr_table = reinterpret_cast<uintptr_t>(&m_table);
        variable->print();
        printf("(%u[%u]) <= ", (unsigned int)ptr_table, (unsigned int)ptr);
        value->print();
        printf("\n");
*/

        return;
    }

    bool
    frame_t::rem_binding(const symbol_t* variable)
    {

        uintptr_t ptr = reinterpret_cast<uintptr_t>(variable);
        return (m_table.erase(ptr) > 0);

    }

    bool
    frame_t::change_binding(const symbol_t* variable, obj_t* value)
    {

        uintptr_t ptr = reinterpret_cast<uintptr_t>(variable);
        ptr_object_table_t::iterator it = m_table.find(ptr);

        if (it == m_table.end()) return false;

        it->second = value;
        return true;

    }

    obj_t*
    frame_t::lookup(const symbol_t* variable)
    {
        uintptr_t ptr = reinterpret_cast<uintptr_t>(variable);
        ptr_object_table_t::iterator it = m_table.find(ptr);

        if (it == m_table.end()) return NULL;

        return it->second;

    }

    void
    frame_t::print()
    {
        ptr_object_table_t::iterator it;

        for ( it = m_table.begin(); it != m_table.end(); it++ ) {
            const symbol_t* symb = reinterpret_cast<const symbol_t*>(it->first);
            obj_t* obj  = it->second;

            symb->print();
            printf(" : ");
            obj->print();
            printf("\n");
        }

        fflush(stdout);

        return;
    }

/*
 * env
 */

    env_t::env_t() {}

    env_t::env_t(const env_t& env)
    {
        // use copy constructor;
        m_frames = env.get_frames();
    }

    env_t::~env_t()
    {
        m_frames.clear();
    }

    const std::vector<frame_t*, traceable_allocator<frame_t*> >&
    env_t::get_frames() const
    {
        return m_frames;
    }

    int
    env_t::extend(std::vector<const symbol_t*>* variables, std::vector<obj_t*>* values)
    {
        size_t vars_len = variables->size();
        size_t vals_len = values->size();

        if ( vars_len == vals_len ) {
            frame_t* frame = new frame_t(variables, values);
            m_frames.push_back(frame);
            return ARGUMENTS_RIGHT;
        }

        if ( vars_len < vals_len )
            return ARGUMENTS_TOO_MANY; //CALLERROR("too many arguments supplied -- EXTEND vars_len = %d, vals_len = %d", vars_len, vals_len);

        return ARGUMENTS_TOO_FEW; //CALLERROR("too few arguments supplied -- EXTEND vars_len = %d, vals_len = %d", vars_len, vals_len);

    }

    int
    env_t::extend(obj_t* params, obj_t* args)
    {
        obj_t* var;
        obj_t* val;
        frame_t* frame = new frame_t();

        while (!NILP(params)) {

            if (SYMBOLP(params)) {
                frame->add_binding((const symbol_t*)params, args);
                break;
            }

            if (!CONSP(params)) goto error_extend_obj_t;

            if (NILP(args)) {
                delete frame;
                return ARGUMENTS_TOO_FEW;
                // CALLERROR("too few arguments supplied -- EXTEND");
            }

            var = CAR(params);
            val = CAR(args);

            if (!SYMBOLP(var)) {
                char buf[1024];
                params->print(buf, 1024);
                delete frame;

                CALLERROR("param must be a SYMBOL -- EXTEND params = %s", buf);
            }

            frame->add_binding((const symbol_t*)var, val);

            params = CDR(params);
            args   = CDR(args);

        }

    params_is_nil:

        if (NILP(params) && !NILP(args)) {
            delete frame;
            return ARGUMENTS_TOO_MANY;
            // CALLERROR("too many arguments supplied -- EXTEND");
        }

        // success !!
        m_frames.push_back(frame);

        return ARGUMENTS_RIGHT;

    error_extend_obj_t:
        delete frame;
        char buf1[1024];
        char buf2[1024];
        params->print(buf1, 1024);
        args->print(buf2, 1024);

        CALLERROR("params and args must be SYMOBL or NIL or CONS -- EXTEND params = %s, args = %s", buf1, buf2);
    }

    int
    env_t::extend(frame_t* n_frame)
    {
        m_frames.push_back(n_frame);
        return ARGUMENTS_RIGHT;
    }

    int
    env_t::extend()
    {
        frame_t* frame = new frame_t();
        m_frames.push_back(frame);
        return ARGUMENTS_RIGHT;
    }

    void
    env_t::enclose()
    {
        if (m_frames.empty()) return;
        frame_t* frame = m_frames.back();
        m_frames.pop_back();

        // don't delete the frame, because there may be closures having it.

        return;
    }

    obj_t*
    env_t::lookup(const symbol_t* variable)
    {
//        fprintf(stderr, "lookup called!\n");
//        fflush(stderr);

        std::vector<frame_t*, traceable_allocator<frame_t*> >::reverse_iterator it;
        frame_t* f;
        obj_t* obj;

//        fprintf(stderr, "0");
//        fflush(stderr);

        for(it = m_frames.rbegin(); it != m_frames.rend(); it++)
        {
//            fprintf(stderr, "1");
//            fflush(stderr);

            f = (*it);

//            fprintf(stderr, "2");
//            fflush(stderr);

            obj = f->lookup(variable);
            if (obj != NULL) return obj;
        }



        char buf[1024];
        variable->print(buf, 1024);

        CALLERROR("unbound variable. : LOOKUP %s", buf);

        return NULL;
    }

    void
    env_t::set(const symbol_t* variable, obj_t* value)
    {
        std::vector<frame_t*, traceable_allocator<frame_t*> >::reverse_iterator it;
        frame_t* f;

        for(it = m_frames.rbegin(); it != m_frames.rend(); it++)
        {
            f = (*it);
            if (f->change_binding(variable, value)) return;
        }

        char buf[1024];
        variable->print(buf, 1024);

        CALLERROR("unbound variable. -- SET %s", buf);
    }

    void
    env_t::assign(const symbol_t* variable, obj_t* value)
    {
        std::vector<frame_t*, traceable_allocator<frame_t*> >::reverse_iterator it;
        frame_t* f;

        for(it = m_frames.rbegin(); it != m_frames.rend(); it++)
        {
            f = (*it);
            if (f->change_binding(variable, value)) return;
        }

        f = m_frames.back();

        f->add_binding(variable, value);

        return;
    }

    void
    env_t::unbind(const symbol_t* variable)
    {
        std::vector<frame_t*, traceable_allocator<frame_t*> >::reverse_iterator it;
        frame_t* f;

        for(it = m_frames.rbegin(); it != m_frames.rend(); it++)
        {
            f = (*it);
            if (f->rem_binding(variable)) return;
        }

        char buf[1024];
        variable->print(buf, 1024);

        CALLERROR("unbound variable. -- UNBIND %s", buf);
    }

    void
    env_t::define(const symbol_t* variable, obj_t* value)
    {

        frame_t* f = m_frames.back();

        f->add_binding(variable, value);

        return;
    }

    void env_t::error(const char* fname, unsigned int line, const char* fmt, ...)
    {
        char fname_buf[32];
        remove_dir(fname, fname_buf, 32);

        va_list arg;
        va_start(arg, fmt);

        fprintf(stderr, "%s:%u !! ERROR in 'ENVIRONMENT' !! -- ", fname_buf, line);
        vfprintf(stderr, fmt, arg);
        fprintf(stderr, "\n\n");

        fflush(stderr);

        va_end(arg);

        throw "ENVIRONMENT_ERROR";
    }

}
