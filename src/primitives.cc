#include "core.hh"
#include "object.hh"
#include "reader.hh"
#include "eval.hh"
#include "env.hh"
#include "primitives.hh"
#include "symbol_table.hh"

extern nil_t*		g_obj_nil;
extern t_t*			g_obj_t;
extern obj_t*       g_obj_undef;
extern symbol_table	g_symbol_table;

extern evaluator_t*	g_evaluator;
extern env_t*		g_global_env;

namespace prim {
	obj_t* plus(obj_t* args)
	{
		if (NILP(args)) {
			int64_t z = 0;
			return new number_t(z);
		}

		if(!CONSP(args)) throw "invalid arg.";

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (NUMBERP(first)) {
			const number_t* n = (number_t*)first;
			number_t* ret = new number_t(*n);

			while (!NILP(rest)) {

				first = CAR(rest);

				if (!NUMBERP(first)) throw "primitive procedure '+' some arguments are invalid.";

				*ret += *((number_t*)first);

				rest = CDR(rest);
			}
			return (obj_t*)ret;
		}

		if (STRINGP(first)){
			const char* src = ((string_t*)first)->c_str();
			string_t* ret = new string_t(src, strlen(src));

			while (!NILP(rest)) {
				first = CAR(rest);

				if (!STRINGP(first)) throw "primitive procedure '+' some arguments are invalid.";

				string_t* p = (string_t*)first;
				*ret += *p;

				rest = CDR(rest);
			}

			return (obj_t*)ret;
		}

		throw "primitive procedure '+' some arguments are invalid.";
	}

	obj_t* minus(obj_t* args)
	{
		if (NILP(args)) {
			int64_t z = 0;
			return new number_t(z);
		}

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (NUMBERP(first)) {
			const number_t* n = (number_t*)first;
			number_t* ret = new number_t(*n);
			while (!NILP(rest)) {
				first = CAR(rest);

				if (!NUMBERP(first)) throw "primitive procedure '-' some arguments are invalid.";

				*ret -= *((number_t*)first);

				rest = CDR(rest);
			}
			return (obj_t*)ret;

		}

		throw "primitive procedure '-' some arguments are invalid.";
	}

	obj_t* multiple(obj_t* args)
	{
		if (NILP(args)) {
			int64_t o = 1;
			return new number_t(o);
		}

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (NUMBERP(first)) {
			const number_t* n = (number_t*)first;
			number_t* ret = new number_t(*n);
			while (!NILP(rest)) {
				first = CAR(rest);

				if (!NUMBERP(first)) throw "primitive procedure '*' some arguments are invalid.";

				*ret *= *((number_t*)first);

				rest = CDR(rest);
			}
			return (obj_t*)ret;
		}

		throw "primitive procedure '*' some arguments are invalid.";
	}

	obj_t* devide(obj_t* args)
	{
		if (NILP(args)) {
			throw "primitive procedure '/' requires at least one argument.";
		}

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (NUMBERP(first)) {
			const number_t* n = (number_t*)first;
			number_t* ret = new number_t(*n);
			while (!NILP(rest)) {
				first = CAR(rest);

				if (!NUMBERP(first)) throw "primitive procedure '/' some arguments are invalid.";

				*ret /= *((number_t*)first);

				rest = CDR(rest);
			}
			return (obj_t*)ret;
		}

		throw "primitive procedure '/' some arguments are invalid.";
	}

	obj_t* equal(obj_t* args)
	{
		if (NILP(args)) {
			return g_obj_t;
		}

		if(!CONSP(args)) throw "invalid arg.";

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (NILP(rest)) {
			return g_obj_t;
		}

		if (NUMBERP(first)) {

			do {
				const number_t* n = (number_t*)first;
				first = CAR(rest);
				if (!NUMBERP(first)) return g_obj_nil;
				const number_t* n2 = (number_t*)first;
				if (*n != *n2) return g_obj_nil;
				rest = CDR(rest);

			} while (!NILP(rest));

			return g_obj_t;
		}

		if (STRINGP(first)){

			do {
				const string_t* s = (string_t*)first;
				first = CAR(rest);
				if (!STRINGP(first)) return g_obj_nil;
				const string_t* s2 = (string_t*)first;
				if (*s != *s2) return g_obj_nil;
				rest = CDR(rest);

			} while (!NILP(rest));

			return g_obj_t;
		}

		throw "primitive procedure '+' some arguments are invalid.";
	}

	obj_t* smaller_than(obj_t* args)
	{
		if (NILP(args)) {
			return g_obj_t;
		}

		if(!CONSP(args)) throw "invalid arg.";

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (NILP(rest)) {
			return g_obj_t;
		}

		if (NUMBERP(first)) {

			do {
				const number_t* n = (number_t*)first;
				first = CAR(rest);
				if (!NUMBERP(first)) throw "primitive procedure '<' some arguments are invalid.";
				const number_t* n2 = (number_t*)first;
				if (*n >= *n2) return g_obj_nil;
				rest = CDR(rest);

			} while (!NILP(rest));

			return g_obj_t;
		}

		throw "primitive procedure '<' some arguments are invalid.";
	}

	obj_t* bigger_than(obj_t* args)
	{
		if (NILP(args)) {
			return g_obj_t;
		}

		if(!CONSP(args)) throw "invalid arg.";

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (NILP(rest)) {
			return g_obj_t;
		}

		if (NUMBERP(first)) {

			do {
				const number_t* n = (number_t*)first;
				first = CAR(rest);
				if (!NUMBERP(first)) throw "primitive procedure '>' some arguments are invalid.";
				const number_t* n2 = (number_t*)first;
				if (*n <= *n2) return g_obj_nil;
				rest = CDR(rest);

			} while (!NILP(rest));

			return g_obj_t;
		}

		throw "primitive procedure '>' some arguments are invalid.";
	}

	obj_t* no(obj_t* args)
	{
		if (NILP(args)) {
			throw "wrong number of arguments 'no' requires 1";
		}

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (!NILP(rest)) {
			throw "wrong number of arguments 'no' requires 1";
		}

		if (NILP(first)) {
			return g_obj_t;
		}

		return g_obj_nil;
	}

	obj_t* car(obj_t* args)
	{
		if (NILP(args)) {
			throw "wrong number of arguments 'car' requires 1";
		}

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (!NILP(rest)) {
			throw "wrong number of arguments 'car' requires 1";
		}

		if (NILP(first)) {
			return g_obj_nil;
		}

		if (CONSP(first)) {
			return CAR(first);
		}

		throw "'car' the argument is invalid.";
	}

	obj_t* cdr(obj_t* args)
	{
		if (NILP(args)) {
			throw "wrong number of arguments. 'cdr' expects 1";
		}

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (!NILP(rest)) {
			throw "wrong number of arguments. 'cdr' expects 1";
		}

		if (NILP(first)) {
			return g_obj_nil;
		}

		if (CONSP(first)) {
			return CDR(first);
		}

		throw "'cdr' the argument is invalid.";
	}

	obj_t* cons(obj_t* args)
	{
		if (NILP(args)) {
			throw "wrong number of arguments. 'cons' expects 2, given 0.";
		}

		obj_t* first	= CAR(args);
		obj_t* rest		= CDR(args);

		if (!NILP(rest)) {
			obj_t* rest2 = CDR(rest);

			if (!NILP(rest2)) {
				throw "wrong number of arguments. 'cons' expects 2, given 3 or more.";
			}

			return (new cons_t(first, CAR(rest)));
		}

		throw "wrong number of arguments. 'cons' expects 2, given 1.";
	}

	obj_t* list(obj_t* args)
	{
		obj_t* ret = g_obj_nil;

		while (!NILP(args)) {
			ret   = new cons_t(CAR(args), ret);
			args  = CDR(args);
		}

		obj_t* tmp = g_obj_nil;
		obj_t* tmp2;

		while (!NILP(ret)) {
			tmp2 = CDR(ret);
			((cons_t*)ret)->set_cdr(tmp);
			tmp = ret;
			ret = tmp2;
		}
		ret = tmp;

		return ret;
	}

	obj_t* load(obj_t* args)
	{
		if (NILP(args)) {
			throw "wrong number of arguments 'load' requires 1";
		}

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (!NILP(rest)) {
			throw "wrong number of arguments 'load' requires 1";
		}

		if(!STRINGP(first))
			throw "wrong type of argument 'load' requires string";

		const char* fname = ((const string_t*)first)->c_str();

		int fd = open(fname, O_RDONLY);
		if ( fd < 0 ){
			// TODO !!! memory leak !!!!
			char* buf = (char*)malloc(64); *buf = 0;
			snprintf(buf, 64, "the file couldn't open (%s)", strerror(errno));

			throw buf;
		}

		reader_t reader = reader_t(fd);

		while(1) {
			obj_t* obj = reader.read_expr();

			if (SYMBOLP(obj) && obj == g_symbol_table.get("")) // S_EOF;
				break;

			g_evaluator->eval(obj, g_global_env);
		}

		close(fd);

		return g_obj_undef;
	}

	void setup_primitives(std::vector<const symbol_t*>* variables, std::vector<obj_t*>* values)
	{
		variables->push_back(g_symbol_table.get("+"));
		values->push_back(new subr_t("+", (void*)plus));

		variables->push_back(g_symbol_table.get("-"));
		values->push_back(new subr_t("-", (void*)minus));

		variables->push_back(g_symbol_table.get("*"));
		values->push_back(new subr_t("*", (void*)multiple));

		variables->push_back(g_symbol_table.get("/"));
		values->push_back(new subr_t("/", (void*)devide));

		variables->push_back(g_symbol_table.get("is"));
		values->push_back(new subr_t("is", (void*)equal));

		variables->push_back(g_symbol_table.get("<"));
		values->push_back(new subr_t("<", (void*)smaller_than));

		variables->push_back(g_symbol_table.get(">"));
		values->push_back(new subr_t(">", (void*)bigger_than));

		variables->push_back(g_symbol_table.get("no"));
		values->push_back(new subr_t("no", (void*)no));

		variables->push_back(g_symbol_table.get("car"));
		values->push_back(new subr_t("car", (void*)car));

		variables->push_back(g_symbol_table.get("cdr"));
		values->push_back(new subr_t("cdr", (void*)cdr));

		variables->push_back(g_symbol_table.get("cons"));
		values->push_back(new subr_t("cons", (void*)cons));

		variables->push_back(g_symbol_table.get("list"));
		values->push_back(new subr_t("list", (void*)list));

		variables->push_back(g_symbol_table.get("load"));
		values->push_back(new subr_t("load", (void*)load));

		return;
	}

}
