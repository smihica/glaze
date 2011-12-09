#include <stdio.h>
#include "glaze.h"

int main(int argc, char* argv[]) {

    printf( "Glaze-Arc version 0.0.0 compiled at %s %s\n", __DATE__, __TIME__ );

    glaze::Config conf;

    glaze::Interpreter impl = glaze::Interpreter(&conf);

    return impl.repl();

}
