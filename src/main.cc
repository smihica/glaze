#include "core.hh"
#include "object.hh"
#include "reader.hh"
#include "eval.hh"
#include "env.hh"
#include "primitives.hh"
#include "symbol_table.hh"

symbol_table	g_symbol_table;

nil_t*			g_obj_nil;
t_t*			g_obj_t;
undef_t*		g_obj_undef;

reader_t*		g_reader;
evaluator_t*	g_evaluator;
env_t*			g_global_env;

#ifdef TRACER
int id;
#endif

int main() {
	GC_INIT();

	g_obj_nil   = new nil_t();
	g_obj_t		= new t_t();
	g_obj_undef = new undef_t();

	// Important this order.
	// Because g_symbol_table is refered in
	// the reader and the evaluator and the env.
	g_symbol_table = *(new symbol_table());

	g_reader		= new reader_t();
	g_evaluator		= new evaluator_t();
	g_global_env	= new env_t();

	std::vector<const symbol_t*>*	primitive_variables = new std::vector<const symbol_t*>();
	std::vector<obj_t*>*			primitive_values	= new std::vector<obj_t*>();

	prim::setup_primitives(primitive_variables, primitive_values);

	primitive_variables->push_back(g_symbol_table.get("nil"));
	primitive_values->push_back((obj_t*)g_obj_nil);
	primitive_variables->push_back(g_symbol_table.get("t"));
	primitive_values->push_back((obj_t*)g_obj_t);

	g_global_env->extend(primitive_variables, primitive_values);

retry:
	try {
		while (1) {
			printf(">");
			obj_t* result;
			obj_t* obj = g_reader->read_expr();
#ifdef TRACER
			id = 0;
			printf("start evaluate : ");
			obj->print();
			printf("\n");
			g_symbol_table.print();
#endif
            if (obj == g_reader->S_EOF)
                break;
			result = g_evaluator->eval(obj, g_global_env);
#ifdef TRACER
			printf("last result is : ");
#endif
			result->print();
			printf("\n");
		}
	} catch (const char* msg) {
		fprintf(stderr, "    -- %s\n", msg);
		goto retry;
	}

	g_global_env->enclose();
	return 0;
}

