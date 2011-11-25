#include "core.h"
#include "object.h"
#include "reader.h"
#include "eval.h"
#include "env.h"
#include "primitives.h"
#include "symbol_table.h"

namespace glaze {
	namespace primitives {

		obj_t* plus(obj_t* args, Shared* shared)
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

		obj_t* minus(obj_t* args, Shared* shared)
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

		obj_t* multiple(obj_t* args, Shared* shared)
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

		obj_t* devide(obj_t* args, Shared* shared)
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

		obj_t* equal(obj_t* args, Shared* shared)
		{
			if (NILP(args)) {
				return shared->t;
			}

			if(!CONSP(args)) throw "invalid arg.";

			obj_t* first = CAR(args);
			obj_t* rest  = CDR(args);

			if (NILP(rest)) {
				return shared->t;
			}

			if (NUMBERP(first)) {

				do {
					const number_t* n = (number_t*)first;
					first = CAR(rest);
					if (!NUMBERP(first)) return shared->nil;
					const number_t* n2 = (number_t*)first;
					if (*n != *n2) return shared->nil;
					rest = CDR(rest);

				} while (!NILP(rest));

				return shared->t;
			}

			if (STRINGP(first)){

				do {
					const string_t* s = (string_t*)first;
					first = CAR(rest);
					if (!STRINGP(first)) return shared->nil;
					const string_t* s2 = (string_t*)first;
					if (*s != *s2) return shared->nil;
					rest = CDR(rest);

				} while (!NILP(rest));

				return shared->t;
			}

			throw "primitive procedure '+' some arguments are invalid.";
		}

		obj_t* smaller_than(obj_t* args, Shared* shared)
		{
			if (NILP(args)) {
				return shared->t;
			}

			if(!CONSP(args)) throw "invalid arg.";

			obj_t* first = CAR(args);
			obj_t* rest  = CDR(args);

			if (NILP(rest)) {
				return shared->t;
			}

			if (NUMBERP(first)) {

				do {
					const number_t* n = (number_t*)first;
					first = CAR(rest);
					if (!NUMBERP(first)) throw "primitive procedure '<' some arguments are invalid.";
					const number_t* n2 = (number_t*)first;
					if (*n >= *n2) return shared->nil;
					rest = CDR(rest);

				} while (!NILP(rest));

				return shared->t;
			}

			throw "primitive procedure '<' some arguments are invalid.";
		}

		obj_t* bigger_than(obj_t* args, Shared* shared)
		{
			if (NILP(args)) {
				return shared->t;
			}

			if(!CONSP(args)) throw "invalid arg.";

			obj_t* first = CAR(args);
			obj_t* rest  = CDR(args);

			if (NILP(rest)) {
				return shared->t;
			}

			if (NUMBERP(first)) {

				do {
					const number_t* n = (number_t*)first;
					first = CAR(rest);
					if (!NUMBERP(first)) throw "primitive procedure '>' some arguments are invalid.";
					const number_t* n2 = (number_t*)first;
					if (*n <= *n2) return shared->nil;
					rest = CDR(rest);

				} while (!NILP(rest));

				return shared->t;
			}

			throw "primitive procedure '>' some arguments are invalid.";
		}

		obj_t* no(obj_t* args, Shared* shared)
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
				return shared->t;
			}

			return shared->nil;
		}

		obj_t* car(obj_t* args, Shared* shared)
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
				return shared->nil;
			}

			if (CONSP(first)) {
				return CAR(first);
			}

			throw "'car' the argument is invalid.";
		}

		obj_t* cdr(obj_t* args, Shared* shared)
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
				return shared->nil;
			}

			if (CONSP(first)) {
				return CDR(first);
			}

			throw "'cdr' the argument is invalid.";
		}

		obj_t* cons(obj_t* args, Shared* shared)
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

		obj_t* list(obj_t* args, Shared* shared)
		{
			obj_t* ret = shared->nil;

			while (!NILP(args)) {
				ret   = new cons_t(CAR(args), ret);
				args  = CDR(args);
			}

			obj_t* tmp = shared->nil;
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

		obj_t* load(obj_t* args, Shared* shared)
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

			reader_t reader = reader_t(shared, fd);

			while(1) {
				obj_t* obj = reader.read_expr();

				if (SYMBOLP(obj) && obj == shared->symbols->get("")) // S_EOF;
					break;

				shared->evaluator->eval(obj, shared->global_env);
			}

			close(fd);

			return shared->undef;
		}

		void setup_primitives(std::vector<const symbol_t*>* variables, std::vector<obj_t*>* values, Shared* shared)
		{
			variables->push_back(shared->symbols->get("+"));
			values->push_back(new subr_t("+", (void*)plus));

			variables->push_back(shared->symbols->get("-"));
			values->push_back(new subr_t("-", (void*)minus));

			variables->push_back(shared->symbols->get("*"));
			values->push_back(new subr_t("*", (void*)multiple));

			variables->push_back(shared->symbols->get("/"));
			values->push_back(new subr_t("/", (void*)devide));

			variables->push_back(shared->symbols->get("is"));
			values->push_back(new subr_t("is", (void*)equal));

			variables->push_back(shared->symbols->get("<"));
			values->push_back(new subr_t("<", (void*)smaller_than));

			variables->push_back(shared->symbols->get(">"));
			values->push_back(new subr_t(">", (void*)bigger_than));

			variables->push_back(shared->symbols->get("no"));
			values->push_back(new subr_t("no", (void*)no));

			variables->push_back(shared->symbols->get("car"));
			values->push_back(new subr_t("car", (void*)car));

			variables->push_back(shared->symbols->get("cdr"));
			values->push_back(new subr_t("cdr", (void*)cdr));

			variables->push_back(shared->symbols->get("cons"));
			values->push_back(new subr_t("cons", (void*)cons));

			variables->push_back(shared->symbols->get("list"));
			values->push_back(new subr_t("list", (void*)list));

			variables->push_back(shared->symbols->get("load"));
			values->push_back(new subr_t("load", (void*)load));

			return;
		}
	}
}
