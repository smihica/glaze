#ifndef GLAZE_H_
#define GLAZE_H_

#include "core.hh"
#include "object.hh"
#include "env.hh"
#include "eval.hh"
#include "shared.hh"

namespace glaze {

	class reader_t;

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

	private:
		Shared shared;

		int repl_iter(reader_t* r, FILE* fp_out);
		int repl_iter(reader_t* r, int fd_out);
	};

}

#endif // GLAZE_H_
