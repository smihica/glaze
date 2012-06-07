#ifndef GLAZE__OBJECT_H_
#define GLAZE__OBJECT_H_

#include "core.h"

namespace glaze {

    class env_t;

/*
**  obj_t
*/
    class obj_t :
        public gc_cleanup /* using boehmGC */
    {
    public:
        obj_t();
        ~obj_t();
        ssize_t print() const;
        ssize_t print(FILE* fp) const;
        ssize_t print(int fd) const;
        ssize_t print(char* target, size_t size) const;
        enum typetag {
            OBJECT      = 0,
            UNDEF       = 1,
            NIL         = 2,
            T           = 3,
            CONS        = 4,
            SYMBOL      = 5,
            NUMBER      = 6,
            STRING      = 7,
            SUBR        = 8,
            CLOSURE     = 9,
            MACRO       = 10,
            SYNTAX      = 11,
        };
        int type() const;
    protected:
        typetag m_type;
        virtual ssize_t print_proc(FILE* fp) const;
        virtual ssize_t print_proc(int fd) const;
        virtual ssize_t print_proc(char* target, size_t size) const;
        void error(const char* fname, unsigned int line, const char* fmt, ...) const;
    };

/*
**  undef_t
*/
    class undef_t :
        public obj_t {
    public:
        undef_t();
    private:
        virtual ssize_t print_proc(FILE* fp) const;
        virtual ssize_t print_proc(int fd) const;
        virtual ssize_t print_proc(char* target, size_t size) const;
    };

#define UNDEFP(obj) ((int)((obj)->type()) == (int)(obj_t::UNDEF))


/*
**  nil_t
*/
    class nil_t :
        public obj_t {
    public:
        nil_t();
    private:
        virtual ssize_t print_proc(FILE* fp) const;
        virtual ssize_t print_proc(int fd) const;
        virtual ssize_t print_proc(char* target, size_t size) const;
    };

#define NILP(obj) ((int)((obj)->type()) == (int)(obj_t::NIL))


/*
** cons_t
*/
    class cons_t :
        public obj_t {
    public:
        cons_t(obj_t* car_ptr, obj_t* cdr_ptr);
        obj_t*  car() const;
        obj_t*  cdr() const;
        void    set_car(obj_t* car_ptr);
        void    set_cdr(obj_t* car_ptr);
        ssize_t print_list(FILE* fp, bool top) const;
        ssize_t print_list(int fd, bool top) const;
        ssize_t print_list(char* target, size_t size, bool top) const;
    private:
        obj_t* m_car_ptr;
        obj_t* m_cdr_ptr;
        virtual ssize_t print_proc(FILE* fp) const;
        virtual ssize_t print_proc(int fd) const;
        virtual ssize_t print_proc(char* target, size_t size) const;
    };

#define CAR(obj)                            (((cons_t*)(obj))->car())
#define CDR(obj)                            (((cons_t*)(obj))->cdr())
#define CAAR(obj)                           (CAR(CAR(obj)))
#define CADR(obj)                           (CAR(CDR(obj)))
#define CDAR(obj)                           (CDR(CAR(obj)))
#define CDDR(obj)                           (CDR(CDR(obj)))
#define CADDR(obj)                          (CAR(CDR(CDR(obj))))
#define CADAR(obj)                          (CAR(CDR(CAR(obj))))
#define CDDDR(obj)                          (CDR(CDR(CDR(obj))))
#define CONSP(obj)                          ((int)(obj->type()) == (int)(obj_t::CONS))


/*
** number_t
*/
#define PRINT_NUMBER_BUFFER_SIZE            256

    class number_t :
        public obj_t {
    public:
        number_t(const number_t* n);
        number_t(const int64_t n = 0);
        number_t(const double n);

        number_t& operator=(const number_t& n);
        number_t& operator=(const int64_t n);
        number_t& operator=(const double n);
        const number_t operator+(const number_t& n) const;
        const number_t operator-(const number_t& n) const;
        const number_t operator*(const number_t& n) const;
        const number_t operator/(const number_t& n) const;

        const bool operator==(const number_t& n) const;
        const bool operator!=(const number_t& n) const;
        const bool operator<=(const number_t& n) const;
        const bool operator>=(const number_t& n) const;
        const bool operator<(const number_t& n) const;
        const bool operator>(const number_t& n) const;

        number_t& operator+=(const number_t& n);
        number_t& operator-=(const number_t& n);
        number_t& operator*=(const number_t& n);
        number_t& operator/=(const number_t& n);

        enum number_type {
            FIXNUM = 0, FLOAT = 1
        };

        const inline int get_number_type() const { return (int)m_number_type; }
        const inline int64_t get_fixnum() const { return m_value.fixnum; }
        const inline double  get_floatnum() const { return m_value.floatnum; }

    private:
        union value {
            int64_t  fixnum;
            double   floatnum;
        };

        value m_value;
        number_type m_number_type;
        void to_str(char* buf, size_t size) const;
        virtual ssize_t print_proc(FILE* fp) const;
        virtual ssize_t print_proc(int fd) const;
        virtual ssize_t print_proc(char* target, size_t size) const;
    };

#define NUMBERP(obj) ((int)((obj)->type()) == (int)(obj_t::NUMBER))

/*
** string_t
*/
    class string_t :
        public obj_t {
    public:
        string_t(const char* src, size_t len);
        ~string_t();

        string_t& operator=(const string_t& str) {
            return operator=(str.c_str());
        }

        string_t& operator=(const char* src) {
            update(src, strlen(src));
            return *this;
        }

        const string_t operator+(const string_t& str) const;
        string_t& operator+=(const string_t& str);

        const bool operator==(const string_t& str) const;
        const bool operator!=(const string_t& str) const;

        inline const char* c_str() const { return m_ptr; }
        inline const size_t len() const { return m_len; }

    private:
        char* m_ptr;
        size_t m_len;

        void update(const char* src, size_t len);

        virtual ssize_t print_proc(FILE* fp) const;
        virtual ssize_t print_proc(int fd) const;
        virtual ssize_t print_proc(char* target, size_t size) const;
    };

#define STRINGP(obj) ((int)((obj)->type()) == (int)(obj_t::STRING))

/*
** symbol_t
*/
    class symbol_t :
        public obj_t {
    public:
        symbol_t(const char* src, size_t len);
        ~symbol_t();

        inline const char* name() const { return m_name; };
        const bool operator==(const symbol_t& target) const;

    private:
        char* m_name;
        size_t m_len;

        virtual ssize_t print_proc(FILE* fp) const;
        virtual ssize_t print_proc(int fd) const;
        virtual ssize_t print_proc(char* target, size_t size) const;
    };

#define SYMBOLP(obj) ((int)((obj)->type()) == (int)(obj_t::SYMBOL))

/*
** t_t
*/
    class t_t :
        public obj_t {
    public:
        t_t();

    private:
        virtual ssize_t print_proc(FILE* fp) const;
        virtual ssize_t print_proc(int fd) const;
        virtual ssize_t print_proc(char* target, size_t size) const;
    };

#define TP(obj) ((int)((obj)->type()) == (int)(obj_t::T))

/*
** function_t (virtual)
*/
    class function_t : public obj_t {};

#define FUNCTIONP(obj) (SUBRP(obj)||CLOSUREP(obj))

/*
** subr_t
*/
    class subr_t :
        public function_t {
    public:
        subr_t(const char* name, void* proc);
        ~subr_t();

        inline void* func() { return m_proc; }
        inline const char* name() const { return m_name == NULL ? "#f" : m_name; };

    private:
        void* m_proc;
        char* m_name;

        virtual ssize_t print_proc(FILE* fp) const;
        virtual ssize_t print_proc(int fd) const;
        virtual ssize_t print_proc(char* target, size_t size) const;
    };

#define SUBRP(obj) ((int)(obj->type()) == (int)(obj_t::SUBR))

/*
** closure_t
*/
    class closure_t :
        public function_t {
    public:
        closure_t(obj_t* parameters, obj_t* body, env_t* env);
        ~closure_t();

        inline obj_t* param() const { return m_parameters; };
        inline obj_t* body() const { return m_body; };
        inline env_t* env()  const { return m_env; };
        inline const char* name() const { return m_name == NULL ? "#f" : m_name; };

        void set_name(const char* name);

    private:
        obj_t* m_parameters;
        obj_t* m_body;
        env_t* m_env;
        char* m_name;

        virtual ssize_t print_proc(FILE* fp) const;
        virtual ssize_t print_proc(int fd) const;
        virtual ssize_t print_proc(char* target, size_t size) const;
    };

#define CLOSUREP(obj) ((int)(obj->type()) == (int)(obj_t::CLOSURE))

/*
** macro_t
*/
    class macro_t :
        public obj_t {
    public:
        macro_t(obj_t* parameters, obj_t* body, env_t* env);
        ~macro_t();

        inline obj_t* param() const { return m_parameters; };
        inline obj_t* body() const { return m_body; };
        inline env_t* env()  const { return m_env; };
        inline const char* name() const { return m_name == NULL ? "#f" : m_name; };

        void set_name(const char* name);

    private:
        obj_t* m_parameters;
        obj_t* m_body;
        env_t* m_env;
        char* m_name;

        virtual ssize_t print_proc(FILE* fp) const;
        virtual ssize_t print_proc(int fd) const;
        virtual ssize_t print_proc(char* target, size_t size) const;
    };

#define MACROP(obj) ((int)(obj->type()) == (int)(obj_t::MACRO))

/*
** syntax_t
*/
    class syntax_t :
        public obj_t {
    public:
        syntax_t(const char* name);
        ~syntax_t();

        inline const char* name() const { return m_name; };

    private:
        char* m_name;

        virtual ssize_t print_proc(FILE* fp) const;
        virtual ssize_t print_proc(int fd) const;
        virtual ssize_t print_proc(char* target, size_t size) const;
    };

#define SYNTAXP(obj) ((int)(obj->type()) == (int)(obj_t::SYNTAX))

}

#endif // GLAZE__OBJECT_H_
