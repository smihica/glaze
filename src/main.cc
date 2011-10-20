#include "core.hh"
#include "object.hh"
#include "reader.hh"
#include "eval.hh"
#include "env.hh"
#include "primitives.hh"
#include "symbol_table.hh"

symbol_table	g_symbol_table;

nil_t*			g_obj_nil;
undef_t*		g_obj_undef;
boolean_t*		g_obj_true;
boolean_t*		g_obj_false;

#ifdef TRACER
int id;
#endif

int main() {

	fprintf(stderr,"hello\n");
	fflush(stderr);

	g_obj_nil   = new nil_t();
	g_obj_true  = new boolean_t(true);
	g_obj_false = new boolean_t(false);
	g_obj_undef = new undef_t();

	fprintf(stderr,"hellohello\n");
	fflush(stderr);

	g_symbol_table = *(new symbol_table());

	reader_t	r = reader_t();
	evaluator_t e = evaluator_t();
	env_t  		env = env_t();

	std::vector<const symbol_t*>*	primitive_variables = new std::vector<const symbol_t*>();
	std::vector<obj_t*>*			primitive_values	= new std::vector<obj_t*>();

	prim::setup_primitives(primitive_variables, primitive_values);

	primitive_variables->push_back(g_symbol_table.get("#t"));
	primitive_variables->push_back(g_symbol_table.get("#f"));
	primitive_values->push_back((obj_t*)g_obj_true);
	primitive_values->push_back((obj_t*)g_obj_false);

	env.extend(primitive_variables, primitive_values);

//	g_symbol_table.print();

retry:
	try {
		while (1) {
			printf(">");
			obj_t* result;
			obj_t* obj = r.read_expr();
#ifdef TRACER
			id = 0;
			printf("start evaluate : ");
			obj->print();
			printf("\n");
			g_symbol_table.print();
#endif
			result = e.eval(obj, &env);
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

	env.enclose();
	return 0;
}

