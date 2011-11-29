#ifndef GLAZE__GLAZE_H_
#define GLAZE__GLAZE_H_

#include "core.h"
#include "object.h"
#include "env.h"
#include "eval.h"
#include "reader.h"
#include "shared.h"

namespace glaze {

	class Config {};

	class Interpreter
	{
	public:

		Interpreter(Config* conf);
		virtual ~Interpreter();

		int repl();
		int repl(FILE* fp_in, FILE* fp_out);
		int repl(FILE* fp_in, int fd_out);
		int repl(int fd_in, FILE* fp_out);
		int repl(int fd_in, int fd_out);

		obj_t* read();
		obj_t* read(FILE* fp);
		obj_t* read(int fd);
		obj_t* read(const char* target);

		obj_t* eval(obj_t* obj);

		Shared shared;

	private:

		int repl_iter(FILE* fp_out);
		int repl_iter(int fd_out);
	};

}

#endif // GLAZE__GLAZE_H_
