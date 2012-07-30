#ifndef GLAZE__OBJECT_H_
#define GLAZE__OBJECT_H_

#include <typeinfo>
#include <iostream>
#include "core.h"

namespace glaze {

   /*
   **  Object
   */

    class Object

#ifdef REQUIRE_GC_
        : public gc_cleanup
#endif

    {
    public:
        Object() {}
        ~Object() {}

        static const Object undef;
        static const Object nil;
        static const Object t;

        virtual ssize_t print() const { return print(stdout); }
        virtual ssize_t print(FILE* fp) const;
        virtual ssize_t print(int fd) const;
        virtual ssize_t print(char* target, size_t size) const;

        inline bool isUndef()  const { return this == &undef; }
        inline bool isNil()    const { return this == &nil; }
        inline bool isT()      const { return this == &t; }

        bool isObject()   const;
        bool isSymbol()   const;
        bool isCons()     const;
        bool isFunction() const;
        bool isSubr()     const;
        bool isClosure()  const;
        //bool isMacro()    const;
        bool isSyntax()   const;
        bool isNumber()   const;
        //bool isCharacters() const;
        bool isString()   const;
        //bool isVector()   const;
        //bool isTable()    const;
        //bool isTagged()  const;

        //bool isThread()   const;
        //bool isSocket()   const;
        //bool isInput()    const;
        //bool isOutput()   const;
        //bool isException() const;



    protected:
        void error(const char* fname, unsigned int line, const char* fmt, ...) const;

    private:
        inline const char* getObjectString() const;

    };

    /*
    ** Symbol
    */

#define SYMBOL_STACK_NAME_LENGTH 32

    class Symbol : public Object
    {
    public:
        Symbol(const char* src, size_t len);
        ~Symbol();

        inline const char* name() const { return m_name_ptr; };
        const bool operator==(const Symbol& target) const;

        virtual ssize_t print(FILE* fp) const;
        virtual ssize_t print(int fd) const;
        virtual ssize_t print(char* target, size_t size) const;

    private:
        char* m_name_heap;
        char  m_name[SYMBOL_STACK_NAME_LENGTH];
        const char* m_name_ptr;
        size_t m_len;
    };

    /*
    ** Cons
    */

    class Cons : public Object
    {
    public:
        Cons(Object* car_ptr, Object* cdr_ptr);
        inline Object* car() const { return m_car_ptr; }
        inline Object* cdr() const { return m_cdr_ptr; }
        inline Object* setCar(Object* car_ptr) { m_car_ptr = car_ptr; return this; }
        inline Object* setCdr(Object* cdr_ptr) { m_cdr_ptr = cdr_ptr; return this; }

        virtual ssize_t print(FILE* fp) const;
        virtual ssize_t print(int fd) const;
        virtual ssize_t print(char* target, size_t size) const;

    private:
        Object* m_car_ptr;
        Object* m_cdr_ptr;
    };

#define CAR(obj)   (((Cons*)(obj))->car())
#define CDR(obj)   (((Cons*)(obj))->cdr())
#define CAAR(obj)  (CAR(CAR(obj)))
#define CADR(obj)  (CAR(CDR(obj)))
#define CDAR(obj)  (CDR(CAR(obj)))
#define CDDR(obj)  (CDR(CDR(obj)))
#define CADDR(obj) (CAR(CDR(CDR(obj))))
#define CADAR(obj) (CAR(CDR(CAR(obj))))
#define CDDDR(obj) (CDR(CDR(CDR(obj))))

    /*
    ** Function (virtual)
    */

    class Function : public Object {};

#define FUNCTION_STACK_NAME_LENGTH 16

    /*
    ** Subr
    */

    class Subr : public Function
    {
    public:
        Subr(const char* name, void* proc);
        ~Subr();

        inline void* func() { return m_proc; }
        inline const char* name() const { return m_name_ptr; }

        virtual ssize_t print(FILE* fp) const;
        virtual ssize_t print(int fd) const;
        virtual ssize_t print(char* target, size_t size) const;

    private:
        void* m_proc;
        char* m_name_heap;
        char  m_name[FUNCTION_STACK_NAME_LENGTH];
        const char* m_name_ptr;

    };

    // temp
    class VMCode;

    /*
    ** Closure
    */

    class Closure : public Function
    {
    public:
        Closure(Cons* compiled_body);
        ~Closure();

        inline VMCode* code()     const { return m_code; };
        inline const char* name() const { return m_name_ptr; };

        void setName(const char* name);

        virtual ssize_t print(FILE* fp) const;
        virtual ssize_t print(int fd) const;
        virtual ssize_t print(char* target, size_t size) const;

    private:
        VMCode* m_code;
        char*   m_name_heap;
        char    m_name[FUNCTION_STACK_NAME_LENGTH];
        const char* m_name_ptr;

    };

    /*
    ** Number
    */

    class Number : public Object
    {
    public:
        Number()                {operator=((int64_t)0);}
        Number(const Number* n) {operator=(n);}
        Number(const int64_t n) {operator=(n);}
        Number(const double n)  {operator=(n);}

        virtual ssize_t print(FILE* fp) const;
        virtual ssize_t print(int fd) const;
        virtual ssize_t print(char* target, size_t size) const;

        Number& operator=(const Number& n);
        Number& operator=(const int64_t n);
        Number& operator=(const double n);
        const Number operator+(const Number& n) const;
        const Number operator-(const Number& n) const;
        const Number operator*(const Number& n) const;
        const Number operator/(const Number& n) const;
        const bool operator==(const Number& n) const;
        const bool operator!=(const Number& n) const;
        const bool operator<=(const Number& n) const;
        const bool operator>=(const Number& n) const;
        const bool operator<(const Number& n) const;
        const bool operator>(const Number& n) const;

        Number& operator+=(const Number& n);
        Number& operator-=(const Number& n);
        Number& operator*=(const Number& n);
        Number& operator/=(const Number& n);

        enum numberType {
            FIXNUM = 0, FLOAT = 1
        };

        const inline int     getNumberType() const { return (int)m_number_type; }
        const inline int64_t getFixnum()     const { return m_value.fixnum; }
        const inline double  getFloatnum()   const { return m_value.floatnum; }

    private:
        union value {
            int64_t  fixnum;
            double   floatnum;
        };

        value m_value;
        numberType m_number_type;
        void tostr(char* buf, size_t size) const;

    };

    /*
    ** String
    */

    class Str : public Object {
    public:
        Str(const char* src, size_t len);
        ~Str();

        virtual ssize_t print(FILE* fp) const;
        virtual ssize_t print(int fd) const;
        virtual ssize_t print(char* target, size_t size) const;

        Str& operator=(const Str& str) {
            return operator=(str.c_str());
        }

        Str& operator=(const char* src) {
            update(src, strlen(src));
            return *this;
        }

        const Str operator+(const Str& str) const;
        Str& operator+=(const Str& str);

        const bool operator==(const Str& str) const;
        const bool operator!=(const Str& str) const;

        inline const char* c_str() const { return m_ptr; }
        inline const size_t len() const { return m_len; }

        ssize_t display(FILE* fp) const;

    private:
        char* m_ptr;
        size_t m_len;

        void update(const char* src, size_t len);
    };

}

#endif // GLAZE__OBJECT_H_
