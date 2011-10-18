#ifndef EVAL_H_
#define EVAL_H_

#include "core.hh"

#define READ_STRING_SMALL_BUFFER_SIZE	1024
#define READ_NUMBER_BUFFER_SIZE			256
#define MAX_READ_SYMBOL_LENGTH			256

class evaluator_t {
public:
	evaluator_t();

	obj_t* eval(obj_t* exp, env_t* env);
	obj_t* apply(function_t* proc, obj_t* args);

	enum special_t {
		NOT_SPECIAL		= 0,
		QUOTED			= 1,
		ASSIGNMENT		= 2,
		IF_STAT			= 3,
		LAMBDA_STAT		= 4,
		DEFINE_STAT		= 5,
		BEGIN_STAT		= 6,
		AND_STAT		= 7,
		OR_STAT			= 8,
	};

private:
	const symbol_t* special_symbols[8]; 
	inline bool is_self_evaluating(const obj_t* exp);
	inline bool is_variable(const obj_t* exp);
	special_t is_special(const obj_t* exp);
	inline bool is_true(const obj_t* exp);
	inline bool is_application(const obj_t* exp);
	obj_t* list_of_values(obj_t* exps, env_t* env);

	obj_t* eval_special(special_t flag, obj_t* exp, env_t* env);
	inline obj_t* eval_quote(obj_t* exp, env_t* env);
	obj_t* eval_assignment(obj_t* exp, env_t* env);
	obj_t* eval_if(obj_t* exp, env_t* env);
	obj_t* eval_lambda(obj_t* exp, env_t* env);
	obj_t* eval_definition(obj_t* exp, env_t* env);
	obj_t* eval_begin(obj_t* exp, env_t* env);
	obj_t* eval_sequence(obj_t* exps, env_t* env);
	obj_t* eval_and_sequence(obj_t* exps, env_t* env);
	inline obj_t* eval_and(obj_t* exp, env_t* env);
	obj_t* eval_or_sequence(obj_t* exps, env_t* env);
	inline obj_t* eval_or(obj_t* exp, env_t* env);

	// apply
	obj_t* apply_primitive_procedure(subr_t* proc, obj_t* args);
	obj_t* apply_compound_procedure(closure_t* proc, obj_t* args);

	inline bool is_primitive_procedure(const function_t* proc);
	inline bool is_compound_procedure(const function_t* proc);

	void error(const char* fname, unsigned int line, const char* fmt, ...);
};

#endif

