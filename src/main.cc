#include <stdio.h>
#include "version.h"
#include "reader.h"
#include "assembler.h"
#include "vm.h"

using namespace glaze;

int main(int argc, char* argv[])
{

    #ifdef REQUIRE_GC_

    GC_INIT();

    #endif

    _program_id_stamp(stdout);

    Context    context = Context();
    Reader*    reader = new Reader(&context);
    Assembler* assembler = new Assembler(&context);
    VM*        vm = new VM(&context);

    context.reader    = reader;
    context.assembler = assembler;
    context.vm        = vm;

    FILE* fp_out = stdout;
    Object* read_result;

retry:
    try {
        while (1) {
            fprintf(fp_out, "> ");
            read_result = reader->read(stdin);
            if (read_result == Reader::S_EOF) break;
            // eval_result = shared.evaluator->eval(read_result, shared.global_env);
            // Object* nil = const_cast<Object*>(&Object::nil);
            // Object* nil = const_cast<Object*>(&Object::nil);
            Code c[1000];
            size_t len = 0;
            fprintf(fp_out, "read: ");

            read_result->print();
            fprintf(fp_out, "\n");
            fflush(fp_out);

            //std::cout << read_result->isSymbol() << std::endl;
            //std::cout << read_result->isCons()   << std::endl;
            //std::cout << read_result->isObject() << std::endl;

            fflush(fp_out);

            assembler->gen(read_result, c, &len);
            vm->dumpAssembly(fp_out, c, len);
            Object* execute_result = new Number((int64_t)0);
            execute_result = vm->execute(c, 1, execute_result, 0, execute_result, 0);
            execute_result->print();
            std::cout << std::endl;

            /*
            fprintf(stdout, "generated.\n");
            fflush(stdout);
            assembler->dump(fp_out, c, len);
            fprintf(fp_out, "\n");
            */
        }
    } catch (const char* msg) {
        fprintf(stderr, "    -- %s\n", msg);
        goto retry;
    }

    return 0;
}
