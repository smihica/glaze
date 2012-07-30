#include <stdio.h>
#include "version.h"
#include "reader.h"

using namespace glaze;

int main(int argc, char* argv[])
{

    #ifdef REQUIRE_GC_

    GC_INIT();

    #endif

    _program_id_stamp(stdout);

    Reader* reader = new Reader();
    FILE* fp_out = stdout;
    Object* read_result;

retry:
    try {
        while (1) {
            fprintf(fp_out, "> ");
            read_result = reader->read(stdin);
            if (read_result == Reader::S_EOF) break;
            // eval_result = shared.evaluator->eval(read_result, shared.global_env);
            read_result->print(fp_out);
            fprintf(fp_out, "\n");
        }
    } catch (const char* msg) {
        fprintf(stderr, "    -- %s\n", msg);
        goto retry;
    }

    return 0;

}
