#include "core.h"
#include "object.h"
//#include "env.h"

namespace glaze {

    const Object Object::undef   = Symbol("undef", 6);
    const Object Object::nil     = Symbol("nil", 4);
    const Object Object::t       = Symbol("t", 2);

    /*
    **  Object
    */

    ssize_t Object::print(FILE* fp) const
    {
        return fprintf(fp, "%s", getObjectString());
    }

    ssize_t Object::print(int fd) const
    {
        return fdprintf(fd, "%s", getObjectString());
    }

    ssize_t
    Object::print(char* target, size_t size) const
    {
        int res = snprintf(target, size, "%s", getObjectString());
        if (res < 0) ERR("snprintf() failed.");
        if (size <= res) return size - 1;
        return res;
    }

    const char*
    Object::getObjectString() const
    {
        return isNil() ? "nil" : isT() ? "t" : isUndef() ? "#<undef>" : "#<object>";
    }

    bool
    Object::isObject() const
    {
        return get_type_id() == Object::type_id;
    }

    bool
    Object::isSymbol() const
    {
        return get_type_id() == Symbol::type_id;
    }

    bool
    Object::isCons() const
    {
        return get_type_id() == Cons::type_id;
    }

    /*
    bool
    Object::isFunction() const
    {
        return (typeid(*this) == typeid(Function) ||
                typeid(*this) == typeid(Subr) ||
                typeid(*this) == typeid(Closure));
    }

    bool
    Object::isSubr() const
    {
        return typeid(*this) == typeid(Subr);
    }

    bool
    Object::isClosure() const
    {
        return typeid(*this) == typeid(Closure);
    }
    */

    bool
    Object::isNumber() const
    {
        return get_type_id() == Number::type_id;
    }

    void
    Object::error(const char* fname, unsigned int line, const char* fmt, ...) const
    {
        char fname_buf[32];
        remove_dir(fname, fname_buf, 32);

        va_list arg;
        va_start(arg, fmt);

        fprintf(stderr, "%s:%u !! ERROR in 'OBJECT' !! -- ", fname_buf, line);
        vfprintf(stderr, fmt, arg);
        fprintf(stderr, "\n\n");

        fflush(stderr);

        va_end(arg);

        throw "OBJECT_ERROR";
    }

    /*
    ** Symbol
    */


    Symbol::Symbol(const char* src, size_t len) : m_name_heap(NULL)
    {
        if (len < SYMBOL_STACK_NAME_LENGTH) {
            memcpy(m_name, src, len);
            m_name[len] = '\0';
            m_name_ptr = m_name;

        } else {
            m_name_heap = (char*)malloc(len+1);
            memcpy(m_name_heap, src, len);
            *(m_name_heap+len) = '\0';
            m_name_ptr = m_name_heap;

        }

        m_len = len;
    }

    Symbol::~Symbol()
    {
        free(m_name_heap);
    }

    const bool
    Symbol::operator==(const Symbol& target) const
    {
        return (strncmp(m_name_ptr, target.name(), m_len+1) == 0);
    }

    ssize_t
    Symbol::print(FILE* fp) const
    {
        int r;
        if(r = fprintf(fp, "%s", m_name_ptr) < 0)
            ERR("fprintf() failed.");

        return r;
    }

    ssize_t
    Symbol::print(int fd) const
    {
        int r;
        if(r = fdprintf(fd, "%s", m_name_ptr) < 0)
            ERR("dprintf() failed.");

        return r;
    }

    ssize_t
    Symbol::print(char* target, size_t size) const
    {
        int res = snprintf(target, size, "%s", m_name_ptr);
        if (res < 0) ERR("snprintf() failed.");
        if (size <= res) return size - 1;

        return res;
    }

    /*
    **  Cons
    */

    Cons::Cons(Object* car_ptr, Object* cdr_ptr) {
        m_car_ptr = car_ptr;
        m_cdr_ptr = cdr_ptr;
    }

#define PRINT_LIST_FP_FD(FN, FN_STR, FN_ARG)                    \
    {                                                           \
        ssize_t ret = 0;                                        \
        Object* x    = m_car_ptr;                               \
        Object* next = m_cdr_ptr;                               \
                                                                \
        if (FN(FN_ARG, "(") < 0) ERR(FN_STR"() failed.");         \
        ret++;                                                          \
                                                                        \
        while(1) {                                                      \
                                                                        \
            ret += x->print(FN_ARG);                                    \
                                                                        \
            if (!next->isCons()) {                                      \
                if (!next->isNil()) {                                   \
                    if (FN(FN_ARG, " . ") < 0)                          \
                        ERR(FN_STR"() failed.");                  \
                    ret += (3 + next->print(FN_ARG));                   \
                }                                                       \
                break;                                                  \
            }                                                           \
                                                                        \
            /* isCons */                                                \
            if (FN(FN_ARG, " ") < 0) ERR(FN_STR"() failed.");     \
            ret++;                                                      \
            x = static_cast<Cons*>(next)->car();                        \
            next = static_cast<Cons*>(next)->cdr();                     \
                                                                        \
        }                                                               \
                                                                        \
        if (FN(FN_ARG, ")") < 0) ERR(FN_STR"() failed.");         \
        ret++;                                                          \
                                                                        \
        return ret;                                                     \
    }

    ssize_t Cons::print(FILE* fp) const
        PRINT_LIST_FP_FD(fprintf, "fprintf", fp);

    ssize_t Cons::print(int fd) const
        PRINT_LIST_FP_FD(fdprintf, "fdprintf", fd);

    ssize_t
    Cons::print(char* target, size_t size) const
    {
        ssize_t ret  = 0;
        int     res  = 0;
        Object* x    = m_car_ptr;
        Object* next = m_cdr_ptr;

#define STRING_PRINT_LIST_CHECK()                           \
        {                                                   \
            if (res < 0) ERR("snprintf() failed.");   \
            if (size <= res) return ret + (size - 1);       \
            ret  += res;                                    \
            size -= res;                                    \
        }

        res = snprintf(target+ret, size, "(");
        STRING_PRINT_LIST_CHECK();

        while(1) {

            res = x->print(target+ret, size);
            STRING_PRINT_LIST_CHECK();

            if (!next->isCons()) {
                if (!next->isNil()) {

                    res = snprintf(target+ret, size, " . ");
                    STRING_PRINT_LIST_CHECK();

                    ret = next->print(target+ret, size);
                    STRING_PRINT_LIST_CHECK();

                }
                break;
            }

            /* isCons */
            res = snprintf(target+ret, size, " ");
            STRING_PRINT_LIST_CHECK();

            x = static_cast<Cons*>(next)->car();
            next = static_cast<Cons*>(next)->cdr();

        }

        res = snprintf(target+ret, size, ")");
        STRING_PRINT_LIST_CHECK();

        return ret;
    }

    /*
    ** Subr
    */

    Subr::Subr(const char* name, void* proc) : m_name_heap(NULL)
    {
        unsigned int len = strlen(name);
        if (len < FUNCTION_STACK_NAME_LENGTH) {
            memcpy(m_name, name, len);
            m_name[len] = '\0';
            m_name_ptr = m_name;

        } else {
            m_name_heap = (char*)malloc(len+1);
            memcpy(m_name_heap, name, len);
            *(m_name_heap+len) = '\0';
            m_name_ptr = m_name_heap;

        }

        m_proc = proc;
    }

    Subr::~Subr()
    {
        free(m_name_heap);
    }

    ssize_t
    Subr::print(FILE* fp) const
    {
        return fprintf(fp, "#<subr %s>", m_name);
    }

    ssize_t
    Subr::print(int fd) const
    {
        return fdprintf(fd, "#<subr %s>", m_name);
    }

    ssize_t
    Subr::print(char* target, size_t size) const
    {
        int res = snprintf(target, size, "#<subr %s>", m_name);
        if (res < 0) ERR("snprintf() failed.");
        if (size <= res) return size - 1;

        return res;
    }

    /*
    ** Closure
    */

    Closure::Closure(Cons* compiled_body) : m_name_heap(NULL), m_name_ptr("#f")
    {
        // m_code = new VMCode(compiled_body);
        m_code = NULL;
    }

    Closure::~Closure()
    {
        free(m_name_heap);
    }

    void
    Closure::setName(const char* name)
    {
        unsigned int len = strlen(name);
        if (len < FUNCTION_STACK_NAME_LENGTH) {
            memcpy(m_name, name, len);
            m_name[len] = '\0';
            m_name_ptr = m_name;

        } else {
            m_name_heap = (char*)realloc(m_name_heap, len+1);
            *(m_name_heap+len) = '\0';
            m_name_ptr = m_name_heap;

        }
    }

    ssize_t
    Closure::print(FILE* fp) const
    {
        return fprintf(fp, "#<closure %s>", m_name_ptr);
    }

    ssize_t
    Closure::print(int fd) const
    {
        return fdprintf(fd, "#<closure %s>", m_name_ptr);
    }

    ssize_t
    Closure::print(char* target, size_t size) const
    {
        int res = snprintf(target, size, "#<closure %s>", m_name_ptr);
        if (res < 0) ERR("snprintf() failed.");
        if (size <= res) return size - 1;

        return res;
    }

    /*
    **  Number
    */

    Number&
    Number::operator=(const Number& n) {
        switch (n.getNumberType()) {
        case FIXNUM:
            return operator=(n.getFixnum());
        case FLOAT:
            return operator=(n.getFloatnum());
        default:
            ERR("unknown number type.");
        }
        return *this;
    }

    Number&
    Number::operator=(const int64_t n) {
        m_number_type  = FIXNUM;
        m_value.fixnum = n;

        return *this;
    }

    Number&
    Number::operator=(const double n) {
        m_number_type    = FLOAT;
        m_value.floatnum = n;

        return *this;
    }

#define NUMBER_CONST_OPERATOR(RETURN_TYPE, RETURN_FN, OP, OPSTR, ERROR_RETURN)  \
    const RETURN_TYPE                                                   \
    Number::operator OP (const Number& n) const                         \
    {                                                                   \
        register char type_set = ((int)m_number_type << 1) | n.getNumberType(); \
                                                                        \
        switch (type_set) {                                             \
        case 0: /* 00 int int */                                        \
            return RETURN_FN (m_value.fixnum OP n.getFixnum());             \
        case 1: /* 01 int float */                                      \
            return RETURN_FN (static_cast<double>(m_value.fixnum) OP n.getFloatnum()); \
        case 2: /* 10 float int */                                      \
            return RETURN_FN (m_value.floatnum OP static_cast<double>(n.getFixnum())); \
        case 3: /* 11 float float */                                    \
            return RETURN_FN (m_value.floatnum OP n.getFloatnum());         \
        default:                                                        \
            ERR("number " OPSTR " is failed.");                   \
        }                                                               \
        return ERROR_RETURN;                                            \
    }

    NUMBER_CONST_OPERATOR(Number, Number, +,  "+",  *this)
    NUMBER_CONST_OPERATOR(Number, Number, -,  "-",  *this)
    NUMBER_CONST_OPERATOR(Number, Number, *,  "*",  *this)
    NUMBER_CONST_OPERATOR(Number, Number, /,  "/",  *this)

    NUMBER_CONST_OPERATOR(bool, , ==, "==", false)
    NUMBER_CONST_OPERATOR(bool, , !=, "!=", true)
    NUMBER_CONST_OPERATOR(bool, , <=, "<=", false)
    NUMBER_CONST_OPERATOR(bool, , >=, ">=", false)
    NUMBER_CONST_OPERATOR(bool, , <,  "<",  false)
    NUMBER_CONST_OPERATOR(bool, , >,  ">",  false)


#define NUMBER_X_EQL_OPERATOR(OP, X)            \
    Number&                                     \
    Number::operator OP (const Number& n)       \
    {                                           \
        operator = ( operator X (n));           \
        return *this;                           \
    }

    NUMBER_X_EQL_OPERATOR(+=, +)
    NUMBER_X_EQL_OPERATOR(-=, -)
    NUMBER_X_EQL_OPERATOR(*=, *)
    NUMBER_X_EQL_OPERATOR(/=, /)

    void
    Number::tostr(char* buf, size_t size) const
    {
        print(buf, size);
    }

    ssize_t
    Number::print(FILE* fp) const
    {
        register int ret;
        switch (m_number_type)
        {
        case FIXNUM:
            if (ret = fprintf(fp, "%lld", m_value.fixnum) < 0)
                ERR("snprintf() failed.");
            return ret;
        case FLOAT:
            if (ret = fprintf(fp, "%g", m_value.floatnum) < 0)
                ERR("snprintf() failed.");
            return ret;
        default:
            ERR("unknown number type.");
        }
        return ret;
    }

    ssize_t
    Number::print(int fd) const
    {
        register int ret;
        switch (m_number_type)
        {
        case FIXNUM:
            if (ret = fdprintf(fd, "%lld", m_value.fixnum) < 0)
                ERR("snprintf() failed.");
            return ret;
        case FLOAT:
            if (ret = fdprintf(fd, "%g", m_value.floatnum) < 0)
                ERR("snprintf() failed.");
            return ret;
        default:
            ERR("unknown number type.");
        }
        return ret;
    }

    ssize_t
    Number::print(char* target, size_t size) const
    {
        register int ret;
        switch (m_number_type)
        {
        case FIXNUM:
            if (ret = snprintf(target, size, "%lld", m_value.fixnum) < 0)
                ERR("snprintf() failed.");
            return ret;
        case FLOAT:
            if (ret = snprintf(target, size, "%g", m_value.floatnum) < 0)
                ERR("snprintf() failed.");
            return ret;
        default:
            ERR("unknown number type.");
        }
        return ret;
    }


    /*
    ** String
    */

    Str::Str(const char* src, size_t len){
        m_ptr = (char*)malloc(len+1);
        memcpy(m_ptr, src, len);
        *(m_ptr+len) = '\0';

        m_len = strlen(m_ptr);
    }

    Str::~Str() {
        free(m_ptr);
    }

    const Str Str::operator+(const Str& str) const
    {
        // todo ::
        // up efficiency.
        // full memory copy occur every time.

        size_t len = m_len + str.len();
        char buf[len+1];

        memcpy(buf, m_ptr, m_len);
        memcpy(buf+m_len, str.c_str(), str.len());

        *(buf+len) = '\0';

        return Str(buf, len);
    }

    Str& Str::operator+=(const Str& str)
    {
        *this = operator+(str);
        return *this;
    }

    const bool
    Str::operator==(const Str& str) const
    {
        return (m_len != str.len()) ? false : (strncmp(m_ptr, str.c_str(), m_len+1) == 0);
    }

    const bool
    Str::operator!=(const Str& str) const
    {
        return !operator==(str);
    }

    void Str::update(const char* src, size_t len) {
        // todo ::
        // up efficiency.
        // full memory copy occur every time.

        m_ptr = (char*)realloc(m_ptr, len+1);
        memcpy(m_ptr, src, len);
        *(m_ptr+len) = '\0';

        m_len = strlen(m_ptr);
    }

    ssize_t Str::display(FILE* fp) const
    {
        ssize_t i;
        size_t len = strlen(m_ptr);
        bool escape = false;

        for (i=0; i<len; i++) {
            char c = *(m_ptr+i);
            if (c == '\\') {
                if (escape) {
                    fprintf(fp, "%c", '\\');
                } else {
                    escape = true;
                }
                continue;
            }
            if (escape) {
                switch (c) {
                case 'n':
                    fprintf(fp, "%c", '\n');
                    break;
                case 't':
                    fprintf(fp, "%c", '\t');
                    break;
                }
            } else {
                fprintf(fp, "%c", c);
            }
            escape = false;
        }

        return i;
    }

    ssize_t Str::print(FILE* fp) const
    {
        ssize_t s = 0;
        if(fprintf(fp, "\"") < 0)
            ERR("fprintf() failed.");
        s++;

        if(fprintf(fp, "%s", m_ptr) < 0)
            ERR("fprintf() failed.");
        s+=m_len;

        if(fprintf(fp, "\"") < 0)
            ERR("fprintf() failed.");
        s++;

        return s;
    }

    ssize_t Str::print(int fd) const
    {
        ssize_t s = 0;
        if(fdprintf(fd, "\"") < 0)
            ERR("dprintf() failed.");
        s++;

        if(fdprintf(fd, m_ptr) < 0)
            ERR("dprintf() failed.");
        s+=m_len;

        if(fdprintf(fd, "\"") < 0)
            ERR("dprintf() failed.");
        s++;

        return s;

    }

    ssize_t Str::print(char* target, size_t size) const
    {

        int res = snprintf(target, size, "\"%s\"", m_ptr);
        if (res < 0) ERR("snprintf() failed.");
        if (size <= res) return size - 1;

        return res;
    }

}
