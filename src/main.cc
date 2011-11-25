#include <stdio.h>
#include "glaze.h"

int main(int argc, char* argv[]) {

	glaze::Config conf;

	glaze::Interpreter impl = glaze::Interpreter(&conf);

	return impl.repl();

}
