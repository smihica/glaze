#include "core.hh"
#include "object.hh"
#include "eval.hh"
#include "env.hh"
#include "symbol_table.hh"

extern nil_t*		g_obj_nil;
extern undef_t*		g_obj_undef;
extern boolean_t*	g_obj_true;
extern boolean_t*	g_obj_false;
extern symbol_table	g_symbol_table;

#ifdef TRACER
extern int id;
#endif

evaluator_t::evaluator_t()
{
	special_symbols[0] = g_symbol_table.get("quote");
	special_symbols[1] = g_symbol_table.get("set!");
	special_symbols[2] = g_symbol_table.get("if");
	special_symbols[3] = g_symbol_table.get("lambda");
	special_symbols[4] = g_symbol_table.get("define");
	special_symbols[5] = g_symbol_table.get("begin");
	special_symbols[6] = g_symbol_table.get("and");
	special_symbols[7] = g_symbol_table.get("or");
}

obj_t*
evaluator_t::eval(obj_t* exp, env_t* env) {
	evaluator_t::special_t flag;
/*
	printf("eval: ");
	exp->print();
	printf("\n");
	fflush(stdout);
*/

	if (is_self_evaluating(exp))
		return exp;

	else if (is_variable(exp)) {
		return env->lookup((const symbol_t*)exp);
	}

	else if ((flag = is_special(exp)) != evaluator_t::NOT_SPECIAL)
		return eval_special(flag, exp, env);

	else if (is_application(exp)) {
		obj_t* fn = (eval(CAR(exp), env));

		if (!FUNCTIONP(fn)) {
			char buf[1024];
			exp->print(buf, 1024);

			CALLERROR("attempt call non-procedure -- EVAL %s", buf);
		}

		return apply((function_t*)fn,
					 list_of_values(CDR(exp), env));
	}
	else
	{
		char buf[1024];
		exp->print(buf, 1024);

		CALLERROR("unknown expression type -- EVAL %s", buf);
	}

	return NULL;
}

inline bool
evaluator_t::is_self_evaluating(const obj_t* exp)
{
	return (NUMBERP(exp) || STRINGP(exp) || BOOLEANP(exp));
}


inline bool
evaluator_t::is_variable(const obj_t* exp)
{
	return SYMBOLP(exp);
}

evaluator_t::special_t
evaluator_t::is_special(const obj_t* exp)
{

	if (!CONSP(exp)){
		char buf[1024];
		exp->print(buf, 1024);
		CALLERROR("unknown expression type -- IS_SPECIAL %s", buf);
	}

	obj_t* top = CAR(exp);

	if(!(SYMBOLP(top))) return NOT_SPECIAL;

	const symbol_t* s = (symbol_t*)top;

	if ( s == special_symbols[0] )
		return QUOTED;

	else if ( s == special_symbols[1] )
		return ASSIGNMENT;

	else if ( s == special_symbols[2] )
		return IF_STAT;

	else if ( s == special_symbols[3] )
		return LAMBDA_STAT;

	else if ( s == special_symbols[4] )
		return DEFINE_STAT;

	else if ( s == special_symbols[5] )
		return BEGIN_STAT;

	else if ( s == special_symbols[6] )
		return AND_STAT;

	else if ( s == special_symbols[7] )
		return OR_STAT;

	else
		return NOT_SPECIAL;

}

inline bool
evaluator_t::is_true(const obj_t* exp)
{
	return (!(BOOLEANP(exp)) || (((boolean_t*)exp)->get_cond()));
}

inline bool
evaluator_t::is_application(const obj_t* exp)
{
	return CONSP(exp);
}


obj_t*
evaluator_t::list_of_values(obj_t* exps, env_t* env)
{
	if (NILP(exps))
		return g_obj_nil;

	return new cons_t( eval(CAR(exps), env),
					   list_of_values( CDR(exps), env));
}


obj_t*
evaluator_t::eval_special(evaluator_t::special_t flag, obj_t* exp, env_t* env)
{
	switch(flag) {
	case QUOTED:
		return eval_quote(exp, env);
	case ASSIGNMENT:
		return eval_assignment(exp, env);
	case IF_STAT:
		return eval_if(exp, env);
	case LAMBDA_STAT:
		return eval_lambda(exp, env);
	case DEFINE_STAT:
		return eval_definition(exp, env);
	case BEGIN_STAT:
		return eval_begin(exp, env);
	case AND_STAT:
		return eval_and(exp, env);
	case OR_STAT:
		return eval_or(exp, env);
	default: {
		char buf[1024];
		exp->print(buf, 1024);

		CALLERROR("This is not a special expression -- EVAL_SPECIAL %s", buf);
	}
	}

	return g_obj_undef;
}

inline obj_t*
evaluator_t::eval_quote(obj_t* exp, env_t* env)
{
	return CADR(exp);
}

obj_t*
evaluator_t::eval_assignment(obj_t* exp, env_t* env)
{
	obj_t* second = NULL;
	obj_t* third = NULL;
	obj_t* ret;

	second =  CADR(exp);
	if (NILP(second) || (!is_variable(second))) goto error_eval_assignment;

	third = CADDR(exp);
	if (NILP(third)) goto error_eval_assignment;

	ret	= eval(third, env);

	env->set( (const symbol_t*)second, ret );

	return ret;

error_eval_assignment:
	char buf[1024];
	exp->print(buf, 1024);

	CALLERROR("error in set! : Expected variable and single expression -- EVAL_ASSIGNMENT %s", buf);

	return g_obj_undef;
}

obj_t*
evaluator_t::eval_if(obj_t* exp, env_t* env)
{
	obj_t* second = CADR(exp);

	if (NILP(second)) goto error_eval_if;

	if (is_true(eval(second, env))) {
		obj_t* third = CADDR(exp);
		if (NILP(third)) goto error_eval_if;

		return eval(third, env);

	} else {
		obj_t* fourth = CADDR(CDR(exp));
		if (NILP(fourth))
		{
			return g_obj_undef;
		}
		return eval(fourth, env);
	}

error_eval_if:
	char buf[1024];
	exp->print(buf, 1024);

	CALLERROR("error in if : Expected 2 or 3 expressions -- EVAL_IF %s", buf);

	return g_obj_undef;
}

/*
  (lambda)
  (lambda (a))
  (lambda a b)
  (lambda "ab" c)
 */
obj_t*
evaluator_t::eval_lambda(obj_t* exp, env_t* env)
{
	obj_t* second	= NULL;
	obj_t* rest		= NULL;
	closure_t* ret;

	second = CADR(exp);
	if (NILP(second)) goto error_eval_lambda;
	rest = CDDR(exp);
	if (NILP(rest))  goto error_eval_lambda;

	ret = new closure_t(second, rest, env);

	char pos[32];
	snprintf(pos, 31, "#%p", ret);
	ret->set_name(pos);

	return ret;

error_eval_lambda:
	char buf[1024];
	exp->print(buf, 1024);

	CALLERROR("error in lambda : Expected formals and body -- EVAL_LAMBDA %s", buf);

	return g_obj_undef;
}

/*
  (define)
  (define (a))
  (define (a) b)
  (define a b)
  (define "ab" c)

 */
obj_t*
evaluator_t::eval_definition(obj_t* exp, env_t* env)
{
	obj_t* second = CADR(exp);

	if (NILP(second)) {
		char buf[1024];
		exp->print(buf, 1024);

		CALLERROR("expected 1 or 2, but no clause given -- EVAL_DEFINITION %s", buf);
	}

	obj_t* definition_value;
	obj_t* definition_variable;

	obj_t* obj;

	if (is_variable(second)) {
		definition_variable = second;

		obj_t* third = CADDR(exp);
		if (NILP(third)) {
			definition_value = g_obj_undef;
		} else {
			definition_value = third;
		}

	} else if (CONSP(second)) {
		definition_variable = CAR(second);

		if (!is_variable(definition_variable)) goto error_eval_definition;

		definition_value = new cons_t( const_cast<symbol_t*>(g_symbol_table.get("lambda")),
									   new cons_t(CDR(second), CDDR(exp)));

	} else goto error_eval_definition;

	obj = eval(definition_value, env);

	if (CLOSUREP(obj)) {
		((closure_t*)obj)->set_name(((const symbol_t*)definition_variable)->name());
	}

	env->define( (const symbol_t*)definition_variable, obj );

	return obj;

error_eval_definition:
	char buf[1024];
	exp->print(buf, 1024);

	CALLERROR("expected symbol or cons for first clause -- EVAL_DEFINITION %s", buf);

	return g_obj_undef;
}

obj_t*
evaluator_t::eval_sequence(obj_t* exps, env_t* env)
{

	obj_t* rest = CDR(exps);

	if (NILP(rest)) return eval(CAR(exps), env);

	eval(CAR(exps), env);

	return eval_sequence(rest, env);
}

obj_t*
evaluator_t::eval_begin(obj_t* exp, env_t* env)
{
	obj_t* rest = CDR(exp);

	if (NILP(rest))
		return g_obj_undef;

	return eval_sequence(rest, env);
}

obj_t*
evaluator_t::eval_and_sequence(obj_t* exps, env_t* env)
{

	if (NILP(exps)) return g_obj_true;

	if (is_true(eval(CAR(exps), env)))
		return eval_and_sequence(CDR(exps), env);

	return g_obj_false;
}

inline obj_t*
evaluator_t::eval_and(obj_t* exp, env_t* env)
{
	return eval_and_sequence(CDR(exp), env);
}

obj_t*
evaluator_t::eval_or_sequence(obj_t* exps, env_t* env)
{

	if (NILP(exps)) return g_obj_false;

	if (is_true(eval(CAR(exps), env)))
		return g_obj_true;

	return eval_or_sequence(CDR(exps), env);
}

inline obj_t*
evaluator_t::eval_or(obj_t* exp, env_t* env)
{
	return eval_or_sequence(CDR(exp), env);
}


// apply
obj_t*
evaluator_t::apply(function_t* proc, obj_t* args)
{
	if (is_primitive_procedure(proc)) {
		try {
			return apply_primitive_procedure((subr_t*)proc, args);
		} catch (const char* message) {
			CALLERROR("APPLY : in primitive procedure -- %s", message);
		}

	} else if (is_compound_procedure(proc)) {
		return apply_compound_procedure((closure_t*)proc, args);

	} else {
		char buf[1024];
		proc->print(buf, 1024);

		CALLERROR("unknown procedure type -- APPLY %s", buf);
	}

	return g_obj_undef;
}

obj_t*
evaluator_t::apply_primitive_procedure(subr_t* proc, obj_t* args)
{
	obj_t* (*func)(obj_t* arg);
	func = (obj_t* (*)(obj_t*))proc->func();

	return (*func)(args);
}

inline bool
evaluator_t::is_primitive_procedure(const function_t* proc)
{
	return SUBRP(proc);
}

obj_t*
evaluator_t::apply_compound_procedure(closure_t* proc, obj_t* args)
{
#ifdef TRACER
	id++;
	for(int i=0; i<id; i++){ printf("  "); }
	printf("running id:%d --- ", id);
	proc->body()->print();
	printf(" with bindings ");
	proc->param()->print();
	printf(":");
	args->print();
	printf("\n");
	usleep(100000); // 0.1 sec
#endif
	proc->env()->extend(proc->param(), args);
	obj_t* ret = eval_sequence(proc->body(), proc->env());
	proc->env()->enclose();
	
#ifdef TRACER
	for(int i=0; i<id; i++){ printf("  "); }
	printf("the result of id:%d is --- ", id);
	ret->print();
	printf("\n");
	id--;
	usleep(100000); // 0.1 sec
#endif

	return ret;
}

inline bool
evaluator_t::is_compound_procedure(const function_t* proc)
{
	return CLOSUREP(proc);
}

void evaluator_t::error(const char* fname, unsigned int line, const char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	fprintf(stderr, "%s:%u !! ERROR in 'EVALUATOR' !! -- ", fname, line);
	vfprintf(stderr, fmt, arg);
	fprintf(stderr, "\n\n");

	fflush(stderr);

	va_end(arg);

	throw "EVALUATOR_ERROR";
}
