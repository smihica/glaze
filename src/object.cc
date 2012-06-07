#include "core.h"
#include "object.h"
#include "env.h"

namespace glaze {

    /*
    **  obj_t
    */
    obj_t::obj_t() {
        m_type = OBJECT;
    }

    obj_t::~obj_t() {}

    ssize_t obj_t::print() const { return print_proc(stdout); }
    ssize_t obj_t::print(FILE* fp) const { return print_proc(fp); }
    ssize_t obj_t::print(int fd) const { return print_proc(fd); }
    ssize_t obj_t::print(char* target, size_t size) const { return print_proc(target, size); }

    ssize_t
    obj_t::print_proc(FILE* fp) const
    {
        const char* str = "#<standard object>";
        return fprintf(fp, "%s", str);
    }

    ssize_t
    obj_t::print_proc(int fd) const
    {
        const char* str = "#<standard object>";
        return fdprintf(fd, "%s", str);
    }

    ssize_t
    obj_t::print_proc(char* target, size_t size) const
    {
        int res = snprintf(target, size, "#<standard object>");
        if (res < 0) CALLERROR("snprintf() failed.");
        if (size <= res) return size - 1;

        return res;
    }

    int
    obj_t::type() const
    {
        return (int)m_type;
    }

    void obj_t::error(const char* fname, unsigned int line, const char* fmt, ...) const
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
**  undef_t
*/
    undef_t::undef_t() {
        m_type = UNDEF;
    }


    ssize_t
    undef_t::print_proc(FILE* fp) const
    {
        const char* str = "#<undef>";
        return fprintf(fp, "%s", str);
    }

    ssize_t
    undef_t::print_proc(int fd) const
    {
        const char* str = "#<undef>";
        return fdprintf(fd, "%s", str);
    }

    ssize_t
    undef_t::print_proc(char* target, size_t size) const
    {
        int res = snprintf(target, size, "#<undef>");
        if (res < 0) CALLERROR("snprintf() failed.");
        if (size <= res) return size - 1;

        return res;
    }


/*
**  nil_t
*/
    nil_t::nil_t() {
        m_type = NIL;
    }


    ssize_t
    nil_t::print_proc(FILE* fp) const
    {
        const char* str = "nil";
        return fprintf(fp, "%s", str);
    }

    ssize_t
    nil_t::print_proc(int fd) const
    {
        const char* str = "nil";
        return fdprintf(fd, "%s", str);
    }

    ssize_t
    nil_t::print_proc(char* target, size_t size) const
    {
        int res = snprintf(target, size, "nil");
        if (res < 0) CALLERROR("snprintf() failed.");
        if (size <= res) return size - 1;

        return res;
    }

/*
**  cons_t
*/
    cons_t::cons_t(obj_t* car_ptr, obj_t* cdr_ptr) {
        m_car_ptr = car_ptr;
        m_cdr_ptr = cdr_ptr;
        m_type = CONS;
    }

    void
    cons_t::set_car(obj_t* car_ptr) {
        m_car_ptr = car_ptr;
    }

    void
    cons_t::set_cdr(obj_t* cdr_ptr) {
        m_cdr_ptr = cdr_ptr;
    }

    obj_t*
    cons_t::car() const
    {
        return m_car_ptr;
    }

    obj_t*
    cons_t::cdr() const
    {
        return m_cdr_ptr;
    }

    ssize_t
    cons_t::print_list(FILE* fp, bool top) const
    {
        ssize_t ret = 0;

        if (top) {
            if (fprintf(fp, "(") < 0) CALLERROR("fprintf() failed.");
            ret++;
        }

        ret += m_car_ptr->print(fp);

        int next_type = m_cdr_ptr->type();

        if (next_type == (int)CONS)
        {
            if (fprintf(fp, " ") < 0) CALLERROR("fprintf() failed.");
            ret++;
            ret += ((cons_t*)m_cdr_ptr)->print_list(fp, false);
        }
        else if (next_type == (int)NIL)
        {
            // nothing to do.
        }
        else
        {
            if (fprintf(fp, " . ") < 0) CALLERROR("fprintf() failed.");
            ret += 3;

            ret += m_cdr_ptr->print(fp);
        }

        if (top)
        {
            if (fprintf(fp, ")") < 0) CALLERROR("fprintf() failed.");
            ret++;
        }

        return ret;
    }

    ssize_t
    cons_t::print_list(int fd, bool top) const
    {
        ssize_t ret = 0;

        if (top) {
            if (fdprintf(fd, "(") < 0) CALLERROR("fdprintf() failed.");
            ret++;
        }

        ret += m_car_ptr->print(fd);

        int next_type = m_cdr_ptr->type();

        if (next_type == (int)CONS)
        {
            if (fdprintf(fd, " ") < 0) CALLERROR("fdprintf() failed.");
            ret++;
            ret += ((cons_t*)m_cdr_ptr)->print_list(fd, false);
        }
        else if (next_type == (int)NIL)
        {
            // nothing to do.
        }
        else
        {
            if (fdprintf(fd, " . ") < 0) CALLERROR("fdprintf() failed.");
            ret += 3;

            ret += m_cdr_ptr->print(fd);
        }

        if (top)
        {
            if (fdprintf(fd, ")") < 0) CALLERROR("fdprintf() failed.");
            ret++;
        }

        return ret;
    }

    ssize_t
    cons_t::print_list(char* target, size_t size, bool top) const
    {
        ssize_t ret = 0;
        int res = 0;

        if (top) {
            res = snprintf(target+ret, size, "(");
            if (res < 0) CALLERROR("snprintf() failed.");
            if (size <= res) return ret + (size - 1);
            ret  += res;
            size -= res;
        }

        res = m_car_ptr->print(target+ret, size);
        ret += res;
        size -= res;

        int next_type = m_cdr_ptr->type();

        if (next_type == (int)CONS)
        {
            res = snprintf(target+ret, size, " ");
            if (res < 0) CALLERROR("snrintf() failed.");
            if (size <= res) return ret + (size - 1);
            ret  += res;
            size -= res;

            res = ((cons_t*)m_cdr_ptr)->print_list(target+ret, size, false);
            ret  += res;
            size -= res;
        }
        else if (next_type == (int)NIL)
        {
            // nothing to do.
        }
        else
        {
            res = snprintf(target+ret, size, " . ");
            if (res < 0) CALLERROR("snrintf() failed.");
            if (size <= res) return ret + (size - 1);
            ret  += res;
            size -= res;

            res = m_cdr_ptr->print(target+ret, size);
            ret  += res;
            size -= res;
        }

        if (top)
        {
            res = snprintf(target+ret, size, ")");
            if (res < 0) CALLERROR("snrintf() failed.");
            if (size <= res) return ret + (size - 1);
            ret  += res;
            size -= res;
        }

        return ret;
    }

    ssize_t
    cons_t::print_proc(FILE* fp) const
    {
        return print_list(fp, true);
    }

    ssize_t
    cons_t::print_proc(int fd) const
    {
        return print_list(fd, true);
    }

    ssize_t
    cons_t::print_proc(char* target, size_t size) const
    {
        return print_list(target, size, true);
    }


/*
**  number_t
*/

    number_t::number_t(const number_t* n)   { operator=(n); m_type = NUMBER; }
    number_t::number_t(const int64_t n)     { operator=(n); m_type = NUMBER; }
    number_t::number_t(const double n)      { operator=(n); m_type = NUMBER; }

    number_t&
    number_t::operator=(const number_t& n) {
        switch ((int)(n.get_number_type())) {
        case FIXNUM:
            operator=(n.get_fixnum());
            break;
        case FLOAT:
            operator=(n.get_floatnum());
            break;
        default:
            CALLERROR("unknown number type.");
        }

        return *this;
    }

    number_t&
    number_t::operator=(const int64_t n) {
        m_number_type  = FIXNUM;
        m_value.fixnum = n;

        return *this;
    }

    number_t&
    number_t::operator=(const double n) {
        m_number_type    = FLOAT;
        m_value.floatnum = n;

        return *this;
    }

    const number_t
    number_t::operator+(const number_t& n) const
    {
        int type_set = ((int)m_number_type << 1) | n.get_number_type();

        switch (type_set) {
        case 0: // 00 int int
            return number_t(m_value.fixnum + n.get_fixnum());
        case 1: // 01 int float
            return number_t(static_cast<double>(m_value.fixnum) + n.get_floatnum());
        case 2: // 10 float int
            return number_t(m_value.floatnum + static_cast<double>(n.get_fixnum()));
        case 3: // 11 float float
            return number_t(m_value.floatnum + n.get_floatnum());
        default:
            CALLERROR("number + is filed.");
        }

        return *this;
    }

    const number_t
    number_t::operator-(const number_t& n) const
    {
        int type_set = ((int)m_number_type << 1) | n.get_number_type();

        switch (type_set) {
        case 0:
            return number_t(m_value.fixnum - n.get_fixnum());
        case 1:
            return number_t(static_cast<double>(m_value.fixnum) - n.get_floatnum());
        case 2:
            return number_t(m_value.floatnum - static_cast<double>(n.get_fixnum()));
        case 3:
            return number_t(m_value.floatnum - n.get_floatnum());
        default:
            CALLERROR("number - is filed.");
        }

        return *this;
    }

    const number_t
    number_t::operator*(const number_t& n) const
    {
        int type_set = ((int)m_number_type << 1) | n.get_number_type();

        switch (type_set) {
        case 0:
            return number_t(m_value.fixnum * n.get_fixnum());
        case 1:
            return number_t(static_cast<double>(m_value.fixnum) * n.get_floatnum());
        case 2:
            return number_t(m_value.floatnum * static_cast<double>(n.get_fixnum()));
        case 3:
            return number_t(m_value.floatnum * n.get_floatnum());
        default:
            CALLERROR("number * is filed.");
        }

        return *this;
    }

    const number_t
    number_t::operator/(const number_t& n) const
    {
        int type_set = ((int)m_number_type << 1) | n.get_number_type();

        switch (type_set) {
        case 0:
            return number_t(m_value.fixnum / n.get_fixnum());
        case 1:
           return number_t(static_cast<double>(m_value.fixnum) / n.get_floatnum());
        case 2:
            return number_t(m_value.floatnum / static_cast<double>(n.get_fixnum()));
        case 3:
            return number_t(m_value.floatnum / n.get_floatnum());
        default:
            CALLERROR("number / is filed.");
        }

        return *this;
    }

    const bool
    number_t::operator==(const number_t& n) const
    {
        return ((m_number_type == n.get_number_type()) &&
                ((m_number_type == FIXNUM) ? (m_value.fixnum == n.get_fixnum()) : (m_value.floatnum == n.get_floatnum())));
    }

    const bool
    number_t::operator!=(const number_t& n) const
    {
        return !operator==(n);
    }

    const bool
    number_t::operator<=(const number_t& n) const
    {
        int type_set = ((int)m_number_type << 1) | n.get_number_type();

        switch (type_set) {
        case 0:
            return (m_value.fixnum <= n.get_fixnum());
        case 1:
            return (static_cast<double>(m_value.fixnum) <= n.get_floatnum());
        case 2:
            return (m_value.floatnum <= static_cast<double>(n.get_fixnum()));
        case 3:
            return (m_value.floatnum <= n.get_floatnum());
        default:
            CALLERROR("number <= is filed.");
        }

        return false;
    }

    const bool
    number_t::operator>=(const number_t& n) const
    {
        int type_set = ((int)m_number_type << 1) | n.get_number_type();

        switch (type_set) {
        case 0:
            return (m_value.fixnum >= n.get_fixnum());
        case 1:
            return (static_cast<double>(m_value.fixnum) >= n.get_floatnum());
        case 2:
            return (m_value.floatnum >= static_cast<double>(n.get_fixnum()));
        case 3:
            return (m_value.floatnum >= n.get_floatnum());
        default:
            CALLERROR("number >= is filed.");
        }

        return false;
    }

    const bool
    number_t::operator<(const number_t& n) const
    {
        int type_set = ((int)m_number_type << 1) | n.get_number_type();

        switch (type_set) {
        case 0:
            return (m_value.fixnum < n.get_fixnum());
        case 1:
            return (static_cast<double>(m_value.fixnum) < n.get_floatnum());
        case 2:
            return (m_value.floatnum < static_cast<double>(n.get_fixnum()));
        case 3:
            return (m_value.floatnum < n.get_floatnum());
        default:
            CALLERROR("number < is filed.");
        }

        return false;
    }

    const bool
    number_t::operator>(const number_t& n) const
    {
        int type_set = ((int)m_number_type << 1) | n.get_number_type();

        switch (type_set) {
        case 0:
            return (m_value.fixnum > n.get_fixnum());
        case 1:
            return (static_cast<double>(m_value.fixnum) > n.get_floatnum());
        case 2:
            return (m_value.floatnum > static_cast<double>(n.get_fixnum()));
        case 3:
            return (m_value.floatnum > n.get_floatnum());
        default:
            CALLERROR("number > is filed.");
        }

        return false;
    }

    number_t&
    number_t::operator+=(const number_t& n)
    {
        *this = operator+(n);
        return *this;
    }

    number_t&
    number_t::operator-=(const number_t& n)
    {
        *this = operator-(n);
        return *this;
    }

    number_t&
    number_t::operator*=(const number_t& n)
    {
        *this = operator*(n);
        return *this;
    }

    number_t&
    number_t::operator/=(const number_t& n)
    {
        *this = operator/(n);
        return *this;
    }

    void
    number_t::to_str(char* buf, size_t size) const
    {
        switch (m_number_type) 
        {
        case FIXNUM:
            if (snprintf(buf, size, "%lld", m_value.fixnum) < 0)
                CALLERROR("snprintf() failed.");
            break;
        case FLOAT:
            if (snprintf(buf, size, "%g", m_value.floatnum) < 0)
                CALLERROR("snprintf() failed.");
            break;
        default:
            CALLERROR("unknown number type.");
        }
    }

    ssize_t
    number_t::print_proc(FILE* fp) const
    {
        char buf[PRINT_NUMBER_BUFFER_SIZE];
        to_str(buf, PRINT_NUMBER_BUFFER_SIZE);

        if(fprintf(fp, "%s", buf) < 0)
            CALLERROR("fprintf() failed.");

        return strlen(buf);
    }

    ssize_t
    number_t::print_proc(int fd) const
    {
        char buf[PRINT_NUMBER_BUFFER_SIZE];
        to_str(buf, PRINT_NUMBER_BUFFER_SIZE);

        if (fdprintf(fd, buf) < 0)
            CALLERROR("dprintf() failed.");

        return strlen(buf);
    }

    ssize_t
    number_t::print_proc(char* target, size_t size) const
    {
        to_str(target, size);

        return strlen(target);
    }


/*
**  string_t
*/
    string_t::string_t(const char* src, size_t len){
        m_ptr = (char*)malloc(len+1);
        memcpy(m_ptr, src, len);
        *(m_ptr+len) = '\0';

        m_len = strlen(m_ptr);
        m_type = STRING;
    }

    string_t::~string_t() {
        free(m_ptr);
    }

    const string_t string_t::operator+(const string_t& str) const
    {
        // todo ::
        // up efficiency.
        // full memory copy occur every time.

        size_t len = m_len + str.len();
        char buf[len+1];

        memcpy(buf, m_ptr, m_len);
        memcpy(buf+m_len, str.c_str(), str.len());

        *(buf+len) = '\0';

        return string_t(buf, len);
    }

    string_t& string_t::operator+=(const string_t& str)
    {
        *this = operator+(str);
        return *this;
    }

    const bool
    string_t::operator==(const string_t& str) const
    {
        return (m_len != str.len()) ? false : (strncmp(m_ptr, str.c_str(), m_len+1) == 0);
    }

    const bool
    string_t::operator!=(const string_t& str) const
    {
        return !operator==(str);
    }

    void string_t::update(const char* src, size_t len) {
        // todo ::
        // up efficiency.
        // full memory copy occur every time.

        m_ptr = (char*)realloc(m_ptr, len+1);
        memcpy(m_ptr, src, len);
        *(m_ptr+len) = '\0';

        m_len = strlen(m_ptr);
    }

    ssize_t
    string_t::print_proc(FILE* fp) const
    {
        ssize_t s = 0;
        if(fprintf(fp, "\"") < 0)
            CALLERROR("fprintf() failed.");
        s++;

        if(fprintf(fp, "%s", m_ptr) < 0)
            CALLERROR("fprintf() failed.");
        s+=m_len;

        if(fprintf(fp, "\"") < 0)
            CALLERROR("fprintf() failed.");
        s++;

        return s;
    }

    ssize_t
    string_t::print_proc(int fd) const
    {
        ssize_t s = 0;
        if(fdprintf(fd, "\"") < 0)
            CALLERROR("dprintf() failed.");
        s++;

        if(fdprintf(fd, m_ptr) < 0)
            CALLERROR("dprintf() failed.");
        s+=m_len;

        if(fdprintf(fd, "\"") < 0)
            CALLERROR("dprintf() failed.");
        s++;

        return s;

    }

    ssize_t
    string_t::print_proc(char* target, size_t size) const
    {

        int res = snprintf(target, size, "\"%s\"", m_ptr);
        if (res < 0) CALLERROR("snprintf() failed.");
        if (size <= res) return size - 1;

        return res;
    }


/*
**  symbol_t
*/
    symbol_t::symbol_t(const char* src, size_t len){
        m_name = (char*)malloc(len+1);
        memcpy(m_name, src, len);
        *(m_name+len) = '\0';

        m_len = strlen(m_name);
        m_type = SYMBOL;
    }

    symbol_t::~symbol_t() {
        free(m_name);
    }

    const bool
    symbol_t::operator==(const symbol_t& target) const
    {
        return (strncmp(m_name, target.name(), m_len+1) == 0);
    }

    ssize_t
    symbol_t::print_proc(FILE* fp) const
    {
        if(fprintf(fp, "%s", m_name) < 0)
            CALLERROR("fprintf() failed.");

        return m_len;
    }

    ssize_t
    symbol_t::print_proc(int fd) const
    {
        if(fdprintf(fd, "%s", m_name) < 0)
            CALLERROR("dprintf() failed.");

        return m_len;
    }

    ssize_t
    symbol_t::print_proc(char* target, size_t size) const
    {
        int res = snprintf(target, size, "%s", m_name);
        if (res < 0) CALLERROR("snprintf() failed.");
        if (size <= res) return size - 1;

        return res;
    }


/*
** t_t
*/
    t_t::t_t() {
        m_type = T;
    }

    ssize_t
    t_t::print_proc(FILE* fp) const
    {
        if(fprintf(fp, "t") < 0)
            CALLERROR("fprintf() failed.");

        return 1;
    }

    ssize_t
    t_t::print_proc(int fd) const
    {
        if(fdprintf(fd, "t") < 0)
            CALLERROR("dprintf() failed.");

        return 1;
    }

    ssize_t
    t_t::print_proc(char* target, size_t size) const
    {
        int res = snprintf(target, size, "t");
        if (res < 0) CALLERROR("snprintf() failed.");
        if (size <= res) return size - 1;

        return res;
    }

/*
** subr_t
*/
    subr_t::subr_t(const char* name, void* proc)
    {
        m_type = SUBR;
        m_proc = proc;
        m_name = (char*)malloc(strlen(name)+1);
        memcpy(m_name, name, strlen(name)+1);
    }

    subr_t::~subr_t()
    {
        free(m_name);
    }

    ssize_t
    subr_t::print_proc(FILE* fp) const
    {
        return fprintf(fp, "#<subr %s>", m_name);
    }

    ssize_t
    subr_t::print_proc(int fd) const
    {
        return fdprintf(fd, "#<subr %s>", m_name);
    }

    ssize_t
    subr_t::print_proc(char* target, size_t size) const
    {
        int res = snprintf(target, size, "#<subr %s>", m_name);
        if (res < 0) CALLERROR("snprintf() failed.");
        if (size <= res) return size - 1;

        return res;
    }

/*
** closure_t
*/
    closure_t::closure_t(obj_t* parameters, obj_t* body, env_t* env)
    {
        m_type = CLOSURE;
        m_parameters = parameters;
        m_body = body;
        m_env = new env_t(*env);

        m_name = NULL;
    }

    closure_t::~closure_t()
    {
        free(m_name);
        delete m_env;
    }

    void
    closure_t::set_name(const char* name)
    {
        m_name = (char*)realloc(m_name, strlen(name)+1);
        memcpy(m_name, name, strlen(name)+1);
    }

    ssize_t
    closure_t::print_proc(FILE* fp) const
    {
        return fprintf(fp, "#<closure %s>", m_name == NULL ? "#f" : m_name);
    }

    ssize_t
    closure_t::print_proc(int fd) const
    {
        return fdprintf(fd, "#<closure %s>", m_name == NULL ? "#f" : m_name);
    }

    ssize_t
    closure_t::print_proc(char* target, size_t size) const
    {
        int res = snprintf(target, size, "#<closure %s>", m_name == NULL ? "#f" : m_name);
        if (res < 0) CALLERROR("snprintf() failed.");
        if (size <= res) return size - 1;

        return res;
    }


/*
** macro_t
*/
    macro_t::macro_t(obj_t* parameters, obj_t* body, env_t* env)
    {
        m_type = MACRO;
        m_parameters = parameters;
        m_body = body;
        m_env = new env_t(*env);

        m_name = NULL;
    }

    macro_t::~macro_t()
    {
        free(m_name);
        delete m_env;
    }

    void
    macro_t::set_name(const char* name)
    {
        m_name = (char*)realloc(m_name, strlen(name)+1);
        memcpy(m_name, name, strlen(name)+1);
    }

    ssize_t
    macro_t::print_proc(FILE* fp) const
    {
        return fprintf(fp, "#<macro %s>", m_name == NULL ? "#f" : m_name);
    }

    ssize_t
    macro_t::print_proc(int fd) const
    {
        return fdprintf(fd, "#<macro %s>", m_name == NULL ? "#f" : m_name);
    }

    ssize_t
    macro_t::print_proc(char* target, size_t size) const
    {
        int res = snprintf(target, size, "#<macro %s>", m_name == NULL ? "#f" : m_name);
        if (res < 0) CALLERROR("snprintf() failed.");
        if (size <= res) return size - 1;

        return res;
    }

/*
** syntax_t
*/
    syntax_t::syntax_t(const char* name)
    {
        m_type = SYNTAX;
        m_name = (char*)realloc(m_name, strlen(name)+1);
        memcpy(m_name, name, strlen(name)+1);
    }

    syntax_t::~syntax_t()
    {
        free(m_name);
    }

    ssize_t
    syntax_t::print_proc(FILE* fp) const
    {
        return fprintf(fp, "#<special-syntax %s>", m_name);
    }

    ssize_t
    syntax_t::print_proc(int fd) const
    {
        return fdprintf(fd, "#<special-syntax %s>", m_name);
    }

    ssize_t
    syntax_t::print_proc(char* target, size_t size) const
    {
        int res = snprintf(target, size, "#<special-syntax %s>", m_name);
        if (res < 0) CALLERROR("snprintf() failed.");
        if (size <= res) return size - 1;

        return res;
    }

}
