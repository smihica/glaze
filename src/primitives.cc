#include "core.hh"
#include "object.hh"
#include "primitives.hh"
#include "symbol_table.hh"

extern nil_t*		g_obj_nil;
extern obj_t*       g_obj_undef;
extern boolean_t*	g_obj_true;
extern boolean_t*	g_obj_false;
extern symbol_table	g_symbol_table;

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
			throw "wrong number of arguments '=' requires at least 2.";
		}

		if(!CONSP(args)) throw "invalid arg.";

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (NILP(rest)) {
			throw "wrong number of arguments '=' requires at least 2.";
		}

		if (NUMBERP(first)) {

			do {
				const number_t* n = (number_t*)first;
				first = CAR(rest);
				if (!NUMBERP(first)) return g_obj_false;
				const number_t* n2 = (number_t*)first;
				if (*n != *n2) return g_obj_false;
				rest = CDR(rest);

			} while (!NILP(rest));

			return g_obj_true;
		}

		if (STRINGP(first)){

			do {
				const string_t* s = (string_t*)first;
				first = CAR(rest);
				if (!STRINGP(first)) return g_obj_false;
				const string_t* s2 = (string_t*)first;
				if (*s != *s2) return g_obj_false;
				rest = CDR(rest);

			} while (!NILP(rest));

			return g_obj_true;
		}

		throw "primitive procedure '+' some arguments are invalid.";
	}

	obj_t* smaller_than(obj_t* args)
	{
		if (NILP(args)) {
			throw "wrong number of arguments '<' requires at least 2.";
		}

		if(!CONSP(args)) throw "invalid arg.";

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (NILP(rest)) {
			throw "wrong number of arguments '<' requires at least 2.";
		}

		if (NUMBERP(first)) {

			do {
				const number_t* n = (number_t*)first;
				first = CAR(rest);
				if (!NUMBERP(first)) throw "primitive procedure '<' some arguments are invalid.";
				const number_t* n2 = (number_t*)first;
				if (*n >= *n2) return g_obj_false;
				rest = CDR(rest);

			} while (!NILP(rest));

			return g_obj_true;
		}

		throw "primitive procedure '<' some arguments are invalid.";
	}

	obj_t* bigger_than(obj_t* args)
	{
		if (NILP(args)) {
			throw "wrong number of arguments '>' requires at least 2.";
		}

		if(!CONSP(args)) throw "invalid arg.";

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (NILP(rest)) {
			throw "wrong number of arguments '>' requires at least 2.";
		}

		if (NUMBERP(first)) {

			do {
				const number_t* n = (number_t*)first;
				first = CAR(rest);
				if (!NUMBERP(first)) throw "primitive procedure '>' some arguments are invalid.";
				const number_t* n2 = (number_t*)first;
				if (*n <= *n2) return g_obj_false;
				rest = CDR(rest);

			} while (!NILP(rest));

			return g_obj_true;
		}

		throw "primitive procedure '>' some arguments are invalid.";
	}

	obj_t* _not(obj_t* args)
	{
		if (NILP(args)) {
			throw "wrong number of arguments 'not' requires 1";
		}

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (!NILP(rest)) {
			throw "wrong number of arguments 'not' requires 1";
		}

		if ( (!(BOOLEANP(first)) || (((boolean_t*)first)->get_cond())) ) {
			return g_obj_false;
		}

		return g_obj_true;
	}

	obj_t* car(obj_t* args)
	{
		if (NILP(args)) {
			return g_obj_nil;
		}

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (!NILP(rest)) {
			throw "wrong number of arguments 'car' requires 1";
		}

		if (CONSP(first)) {
			return CAR(first);
		}

		throw "'car' the argument is invalid.";
	}

	obj_t* cdr(obj_t* args)
	{
		if (NILP(args)) {
			return g_obj_nil;
		}

		obj_t* first = CAR(args);
		obj_t* rest  = CDR(args);

		if (!NILP(rest)) {
			throw "wrong number of arguments 'cdr' requires 1";
		}

		if (CONSP(first)) {
			return CDR(first);
		}

		throw "'cdr' the argument is invalid.";

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

		variables->push_back(g_symbol_table.get("="));
		values->push_back(new subr_t("=", (void*)equal));

		variables->push_back(g_symbol_table.get("<"));
		values->push_back(new subr_t("<", (void*)smaller_than));

		variables->push_back(g_symbol_table.get(">"));
		values->push_back(new subr_t(">", (void*)bigger_than));

		variables->push_back(g_symbol_table.get("not"));
		values->push_back(new subr_t("not", (void*)_not));

		variables->push_back(g_symbol_table.get("car"));
		values->push_back(new subr_t("car", (void*)car));

		variables->push_back(g_symbol_table.get("cdr"));
		values->push_back(new subr_t("cdr", (void*)cdr));

		return;
	}

}
