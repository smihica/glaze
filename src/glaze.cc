#include "core.h"
#include "object.h"
#include "shared.h"
#include "reader.h"
#include "eval.h"
#include "env.h"
#include "primitives.h"
#include "symbol_table.h"
#include "glaze.h"

namespace glaze {

	Interpreter::Interpreter(Config* conf)
	{
		GC_INIT();

		shared._nil		= new nil_t();
		shared.t		= new t_t();
		shared.undef	= new undef_t();

		// Important this order.
		// Because g_symbol_table is refered in
		// the reader and the evaluator and the env.
		shared.symbols		= new symbol_table();
		shared.evaluator	= new evaluator_t(&shared);
		shared.global_env	= new env_t(&shared);
		shared.reader		= new reader_t(&shared);

		std::vector<const symbol_t*>*	primitive_variables = new std::vector<const symbol_t*>();
		std::vector<obj_t*>*			primitive_values	= new std::vector<obj_t*>();

		primitives::setup_primitives(primitive_variables, primitive_values, &shared);

		primitive_variables->push_back(shared.symbols->get("nil"));
		primitive_values->push_back((obj_t*)shared._nil);
		primitive_variables->push_back(shared.symbols->get("t"));
		primitive_values->push_back((obj_t*)shared.t);

		shared.global_env->extend(primitive_variables, primitive_values);
	}

	Interpreter::~Interpreter()
	{
		shared.global_env->enclose();
	}

	int Interpreter::repl()
	{
		return repl(stdin, stdout);
	}

	int Interpreter::repl(FILE* fp_in, FILE* fp_out)
	{
		shared.reader->set_source(fp_in);
		return repl_iter(fp_out);
	}

	int Interpreter::repl(FILE* fp_in, int fd_out)
	{
		shared.reader->set_source(fp_in);
		return repl_iter(fd_out);
	}

	int Interpreter::repl(int fd_in, FILE* fp_out)
	{
		shared.reader->set_source(fd_in);
		return repl_iter(fp_out);
	}

	int Interpreter::repl(int fd_in, int fd_out)
	{
		shared.reader->set_source(fd_in);
		return repl_iter(fd_out);
	}

	int Interpreter::repl_iter(FILE* fp_out)
	{
		obj_t* read_result;
		obj_t* eval_result;

	retry:
		try {
			while (1) {
				fprintf(fp_out, "> ");
				read_result = shared.reader->read();
				if (read_result == shared.reader->S_EOF) break;
				eval_result = shared.evaluator->eval(read_result, shared.global_env);
				eval_result->print(fp_out);
				fprintf(fp_out, "\n");
			}
		} catch (const char* msg) {
			fprintf(stderr, "    -- %s\n", msg);
			goto retry;
		}

		return 0;

	}

	int Interpreter::repl_iter(int fd_out){
		obj_t* read_result;
		obj_t* eval_result;

	retry:
		try {
			while (1) {
				fdprintf(fd_out, "> ");
				read_result = shared.reader->read();
				if (read_result == shared.reader->S_EOF) break;
				eval_result = shared.evaluator->eval(read_result, shared.global_env);
				eval_result->print(fd_out);
				fdprintf(fd_out, "\n");
			}
		} catch (const char* msg) {
			fdprintf(fd_out, "    -- %s\n", msg);
			goto retry;
		}

		return 0;
	}

	obj_t* Interpreter::read()
	{
		return read(stdin);
	}

	obj_t* Interpreter::read(FILE* fp)
	{
		return shared.reader->read(fp);
	}

	obj_t* Interpreter::read(int fd)
	{
		return shared.reader->read(fd);
	}

	obj_t* Interpreter::read(const char* target)
	{
		return shared.reader->read(target);
	}

	obj_t* Interpreter::eval(obj_t* obj)
	{
		return shared.evaluator->eval(obj, shared.global_env);
	}

}
