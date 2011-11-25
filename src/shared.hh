#ifndef SHARED_H_
#define SHARED_H_

#include "core.hh"
#include "object.hh"

namespace glaze {

	class symbol_table;
	class evaluator_t;

	typedef struct shared_
	{
		nil_t*			nil;
		t_t*			t;
		undef_t*		undef;
		env_t*			global_env;

		symbol_table*	symbols;
		evaluator_t*	evaluator;
	} Shared;

}

#endif // SHARED_H_
