#include "core.h"
#include "object.h"
#include "vm.h"

namespace glaze {

    Object* VM::execute(Code* code, size_t len, Object* a, size_t f, Object* c, size_t s)
    {
        size_t pc = 0;
        size_t top = pc;
        FILE* fp = stdout;

        try {

            for (;;) {
                top = pc;
                switch (code[pc++]) {
                case Assembler::HALT: {
                    return a;
                }
                case Assembler::REFER_LOCAL: {
                    int n = code[pc++];
                    a = index(f, n);
                    fprintf(fp, "REFER_LOCL %d\t;;; %zd\n", n, top);
                    break;
                }
                case Assembler::ASSIGN_LOCAL: {
                    int n = code[pc++];
                    index_set(f, n, a);
                    a = const_cast<Object*>(&Object::undef);
                    fprintf(fp, "ASSIGN_LCL %d\t;;; %zd\n", n, top);
                    break;
                }
                    /*
                case Assembler::REFER_FREE: {
                    int n = code[pc++];
                    a = c[n+1];
                    fprintf(fp, "REFER_FREE %d\t;;; %zd\n", n, top);
                    break;
                }
                case Assembler::ASSIGN_FREE: {
                    int n = code[pc++];
                    c[n+1] = a;
                    fprintf(fp, "ASSIGN_FRE %d\t;;; %zd\n", n, top);
                    break;
                }

                case Assembler::REFER_GLOBAL: {
                    Symbol* s = (Symbol*)code[pc++];
                    a = context->env.get(s);
                    fprintf(fp, "REFER_GLOB %s\t;;; %zd\n", s->name(), top);
                    break;
                }
                case Assembler::ASSIGN_GLOBAL: {
                    Symbol* s = (Symbol*)code[pc++];
                    context->env.set(s, a);
                    fprintf(fp, "ASSIGN_GLB %s\t;;; %zd\n", s->name(), top);
                    break;
                }

                case Assembler::BOX: {
                    int n = code[pc++];
                    // index_set(s, n, index(s, n)->box());
                    // fprintf(fp, "BOX        %d\t;;; %zd\n", n, top);
                    break;
                }
                case Assembler::INDIRECT: {
                    // a = a->indirect();
                    // fprintf(fp, "INDIRECT     \t;;; %zd\n", top);
                    break;
                }
                    */
                case Assembler::CONSTANT: {
                    Object* o = (Object*)code[pc++];
                    o->print();
                    a = o;
                    break;
                }
                    /*
                case Assembler::CLOSE: {
                    int n = code[pc++];
                    int dotpos = code[pc++];
                    int range = code[pc++];
                    a = makeClosure(code+top, range, n, s, dotpos);
                    s -= n;
                    pc = top + range;
                    fprintf(fp, "CLOSE   %d %d %d\t;;; %zd\n", n, dotpos, range, top);
                    break;
                }
                case Assembler::TEST: {
                    int n = code[pc++];
                    if (!isTrue(a)) pc = top + n;
                    fprintf(fp, "TEST       %d\t;;; %zd\n", n, top);
                    break;
                }
                    */
                case Assembler::CONTI: {
                    /*
                      int n = code[pc++];
                      fprintf(fp, "CONTI      %d\t;;; %zd\n", n, top);
                    */
                    break;
                }
                case Assembler::NUATE: {
                    /*
                      int n = code[pc++];
                      fprintf(fp, "NUATE      %d\t;;; %zd\n", n, pc);
                      break;
                    */
                    break;
                }
                case Assembler::FRAME: {
                    int n = code[pc++];
                    s = push((intptr_t)(code+(top+n)), push((intptr_t)f, push((intptr_t)c, s)));
                    fprintf(fp, "FRAME ;;; %zd\n", top);
                    break;
                }
                case Assembler::ARGUMENT: {
                    s = push((intptr_t)a, s);
                    fprintf(fp, "ARGUMENT ;;; %zd\n", top);
                    break;
                }
                case Assembler::ARGUMENT_LEN: {
                    int n = code[pc++];
                    s = push(n, s);
                    fprintf(fp, "ARGLEN %d;;; %zd\n", n, top);
                    break;
                }
                case Assembler::SHIFT: {
                    int n = code[pc++];
                    // Getting the number of arguments in the last call
                    int n_args = index_int(s, n);
                    s = shift_args(n, n_args, s);
                    fprintf(fp, "SHIFT      %d\t;;; %zd\n", n, top);
                    break;
                }
                    /*
                case Assembler::HINTED_APPLY: {
                    fprintf(fp, "HINTED_APPLY \t;;; %zd\n", top);
                    goto apply;
                }
                case Assembler::APPLY: {
                    fprintf(fp, "APPLY        \t;;; %zd\n", top);
                    apply:
                    int n_args = index_int(s, -1);
                    if (a->isClosure())
                    {
                        Closure* func = (Closure*)a;
                        code = func->body;
                        int dotpos = func->dotpos;
                        if (dotpos >= 0) {
                            Object* ls = const_cast<Object*>(&Object::nil);
                            for (int i=n_args-1; i>dotpos-1; i--) {
                                ls = new Cons((Object*)index(s, i), ls);
                            }
                            if (dotpos >= n_args) {
                                for (int i=-1; i<n_args; i++) {
                                    index_set(s, i-1, index(s, i));
                                }
                                s++;
                                index_set(s, -1, index(s, -1) + 1);
                            }
                            index_set(s, dotpos, ls);
                        }
                        f = s;
                        c = a;
                    }
                    else if(a->isSubr())
                    {
                        Subr* func = (Subr*)a;
                        Object* args[n_args];

                        for (int i=0; i<n_args; i++)
                            args[i] = (Object*)index(s, i);

                        Object* result = func->call(args, n_args);

                        a = result;
                        goto _return;
                    }
                    else
                    {
                        char obj[100];
                        a->print(obj, 100);
                        ERR("%s is not a function.", obj);
                    }
                    break;
                }
                case Assembler::RETURN: {
                    fprintf(fp, "RETURN       \t;;; %zd\n", top);
                    _return:
                    int n_args = index(s, -1);
                    size_t ss = s - n_args;
                    code = (Code*)index(ss, 0);
                    f    = (size_t)index(ss, 1);
                    c    = (Object*)index(ss, 2);
                    s    = ss-3-1;
                    break;
                }
                    */
                default: {
                    fprintf(fp, "UNKNOWN      \t;;; %zd\n", top);
                    break;
                }
                }

                {
                    char buf[10];
                    buf[0] = 'p';

                instruction:

                    switch (buf[0]) {
                    case 'n':
                        continue;
                    case 'p': {
                        a->print(buf, 10);
                        fprintf(fp, "a: %s, ", buf);
                        fprintf(fp, "pc: %zd, ", pc);
                        fprintf(fp, "f: %zd, ", f);
                        c->print(buf, 10);
                        fprintf(fp, "c: %s, ", buf);
                        fprintf(fp, "s: %zd\n", s);
                        break;
                    }
                    case 's':
                        dumpAssembly(fp, code+pc, len-pc);
                        break;
                    }

                    read(0, buf, 10);

                    goto instruction;

                }
            }

        } catch (std::bad_cast b) {
            std::cout << "ダウンキャストに失敗しました" << std::endl;
            return const_cast<Object*>(&Object::nil);
        } catch (const char* e) {
            return const_cast<Object*>(&Object::nil);
        }
    }

    void VM::dumpAssembly(FILE* fp, Code* code, size_t len) {

        size_t pc = 0;
        size_t top = pc;

        for (;;) {
            top = pc;
            switch (code[pc++]) {
            case Assembler::HALT: {
                fprintf(fp, ";;; %04zd HALT\n", top);
                goto end;
            }
            case Assembler::REFER_LOCAL: {
                int n = code[pc++];
                fprintf(fp, ";;; %04zd REFLCL\t\t%d\n", top, n);
                break;
            }
            case Assembler::REFER_FREE: {
                int n = code[pc++];
                fprintf(fp, ";;; %04zd REFFRE\t\t%d\n", top, n);
                break;
            }
            case Assembler::REFER_GLOBAL: {
                Symbol* s = (Symbol*)code[pc++];
                fprintf(fp, ";;; %04zd REFGLB\t\t%s\n", top, s->name());
                break;
            }
            case Assembler::INDIRECT: {
                fprintf(fp, ";;; %04zd INDIRECT\n", top);
                break;
            }
            case Assembler::CONSTANT: {
                Object* o = (Object*)code[pc++];
                char c[64];
                o->print(c, 64);
                fprintf(fp, ";;; %04zd CONST\t\t%s\n", top, c);
                break;
            }
            case Assembler::CLOSE: {
                int n = code[pc++];
                int dotpos = code[pc++];
                int range = code[pc++];
                fprintf(fp, ";;; %04zd CLOSE\t\t%d\t%d\t%d\n", top, n, dotpos, range);
                break;
            }
            case Assembler::BOX: {
                int n = code[pc++];
                fprintf(fp, ";;; %04zd BOXING\t\t%d\n", top, n);
                break;
            }
            case Assembler::TEST: {
                int n = code[pc++];
                fprintf(fp, ";;; %04zd TEST\t\t%d\n", top, n);
                break;
            }
            case Assembler::ASSIGN_GLOBAL: {
                Symbol* s = (Symbol*)code[pc++];
                fprintf(fp, ";;; %04zd ASNGLB\t\t%s\n", top, s->name());
                break;
            }
            case Assembler::ASSIGN_LOCAL: {
                int n = code[pc++];
                fprintf(fp, ";;; %04zd ASNLCL\t\t%d\n", top, n);
                break;
            }
            case Assembler::ASSIGN_FREE: {
                int n = code[pc++];
                fprintf(fp, ";;; %04zd ASNFRE\t\t%d\n", top, n);
                break;
            }
            case Assembler::CONTI: {
                int n = code[pc++];
                fprintf(fp, ";;; %04zd CONTI\t\t%d\n", top, n);
                break;
            }
            case Assembler::NUATE: {
                /*
                int n = code[pc++];
                fprintf(fp, ";;; %04zd NUATE      %d\n", n, pc);
                break;
                */
                break;
            }
            case Assembler::FRAME: {
                int n = code[pc++];
                fprintf(fp, ";;; %04zd FRAME\t\t%d\n", top, n);
                break;
            }
            case Assembler::ARGUMENT: {
                fprintf(fp, ";;; %04zd ARGMNT\n", top);
                break;
            }
            case Assembler::ARGUMENT_LEN: {
                int n = code[pc++];
                fprintf(fp, ";;; %04zd ARGLEN\t\t%d\n", top, n);
                break;
            }
            case Assembler::SHIFT: {
                int n = code[pc++];
                fprintf(fp, ";;; %04zd SHIFT\t\t%d\n", top, n);
                break;
            }
            case Assembler::HINTED_APPLY: {
                fprintf(fp, ";;; %04zd HAPPLY\n", top);
                break;
            }
            case Assembler::APPLY: {
                fprintf(fp, ";;; %04zd APPLY\n", top);
                break;
            }
            case Assembler::RETURN: {
                fprintf(fp, ";;; %04zd RETURN\n", top);
                break;
            }
            default: {
                fprintf(fp, ";;; %04zd UNKNOWN\n", top);
                break;
            }
            }
            if (pc > len) break;
        }

    end:
        return;

    }

    void VM::error(const char* fname, unsigned int line, const char* fmt, ...)
    {
        char fname_buf[32];
        remove_dir(fname, fname_buf, 32);

        va_list arg;
        va_start(arg, fmt);

        fprintf(stderr, "ERROR on (%s:%u) -- ", fname_buf, line);
        vfprintf(stderr, fmt, arg);
        fprintf(stderr, "\n\n");

        fflush(stderr);

        va_end(arg);

        throw "VM_ERROR";
    }
}
