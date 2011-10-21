#include "core.hh"
#include "object.hh"
#include "env.hh"

extern undef_t*		g_obj_undef;

// frame
frame_t::frame_t()
{
	m_variables	= new std::vector<const symbol_t*>();
	m_values	= new std::vector<obj_t*>();
}

frame_t::frame_t(std::vector<const symbol_t*>* variables, std::vector<obj_t*>* values)
{
	m_variables	= variables;
	m_values	= values;
}

frame_t::~frame_t()
{
	m_variables->clear();
	m_values->clear();
	delete m_variables;
	delete m_values;
}

std::vector<const symbol_t*>*
frame_t::variables()
{
	return m_variables;
}

std::vector<obj_t*>*
frame_t::values()
{
	return m_values;
}

void
frame_t::add_binding(const symbol_t* variable, obj_t* value)
{
	m_variables->push_back(variable);
	m_values->push_back(value);
	return;
}

bool
frame_t::rem_binding(const symbol_t* variable)
{

	std::vector<const symbol_t*>::iterator var_it;
	std::vector<obj_t*>::iterator val_it;

	for( var_it = m_variables->begin(),
		 val_it = m_values->begin();

		 (var_it != m_variables->end() &&
		  val_it != m_values->end());

		 var_it++, val_it++)
	{
		if(*var_it == variable) {
			m_variables->erase(var_it);
			m_values->erase(val_it);

			return true;
		}
	}

	return false;
}

bool
frame_t::change_binding(const symbol_t* variable, obj_t* value)
{
	std::vector<const symbol_t*>::iterator var_it;
	std::vector<obj_t*>::iterator val_it;

	for( var_it = m_variables->begin(),
		 val_it = m_values->begin();

		 (var_it != m_variables->end() &&
		  val_it != m_values->end());

		 var_it++, val_it++)
	{
		if(*var_it == variable) {
			(*val_it) = value;
			return true;
		}
	}

	return false;
}

obj_t*
frame_t::lookup(const symbol_t* variable)
{
	size_t lim = m_variables->size();

	for (size_t i = 0; i < lim; i++) {
		if ((m_variables->at(i)) == variable) {
			return m_values->at(i);
		}
	}

	return NULL;
}

void
frame_t::print()
{
	std::vector<const symbol_t*>::iterator var_it;
	std::vector<obj_t*>::iterator val_it;

	for( var_it = m_variables->begin(),
		 val_it = m_values->begin();

		 (var_it != m_variables->end() &&
		  val_it != m_values->end());

		 var_it++, val_it++)
	{
		(*var_it)->print();
		printf(" : ");
		(*val_it)->print();
		printf("\n");
	}

	fflush(stdout);
	return;
}


// env
env_t::env_t(const env_t& env)
{
	// use copy constructor;
	m_frames = env.get_frames();
}

env_t::~env_t()
{
/*
	std::vector<frame_t*>::reverse_iterator it;
	frame_t* f;

	for(it = m_frames.rbegin(); it != m_frames.rend(); it++)
	{
		f = (*it);
		delete f;
	}
*/
	m_frames.clear();
}

const std::vector<frame_t*>&
env_t::get_frames() const
{
	return m_frames;
}

void
env_t::extend(std::vector<const symbol_t*>* variables, std::vector<obj_t*>* values)
{
	size_t vars_len = variables->size();
	size_t vals_len = values->size();

	if ( vars_len == vals_len ) {
		frame_t* frame = new frame_t(variables, values);
		m_frames.push_back(frame);
		return;
	}

	if ( vars_len < vals_len )
		CALLERROR("too many arguments supplied -- EXTEND vars_len = %d, vals_len = %d", vars_len, vals_len);

	CALLERROR("too few arguments supplied -- EXTEND vars_len = %d, vals_len = %d", vars_len, vals_len);
}

void
env_t::extend(obj_t* params, obj_t* args)
{
	std::vector<const symbol_t*>*	vars = new std::vector<const symbol_t*>();
	std::vector<obj_t*>*			vals = new std::vector<obj_t*>();
	obj_t* var;
	obj_t* val;

	while (!NILP(params)) {

		if (SYMBOLP(params)) {
			vars->push_back((const symbol_t*)params);
			vals->push_back(args);
			break;
		}

		if (!CONSP(params)) goto error_extend_obj_t;

		if (NILP(args)) {
			CALLERROR("too few arguments supplied -- EXTEND");
		}

		var = CAR(params);
		val = CAR(args);

		if (!SYMBOLP(var)) {
			char buf[1024];
			params->print(buf, 1024);

			CALLERROR("param must be a SYMBOL -- EXTEND params = %s", buf);
		}

		vars->push_back((const symbol_t*)var);
		vals->push_back(val);

		params = CDR(params);
		args   = CDR(args);
	}

	if (NILP(params) && !NILP(args)) {
		CALLERROR("too many arguments supplied -- EXTEND");
	}

	extend(vars, vals);
	return;

error_extend_obj_t:
	char buf1[1024];
	char buf2[1024];
	params->print(buf1, 1024);
	args->print(buf2, 1024);

	CALLERROR("params and args must be SYMOBL or NIL or CONS -- EXTEND params = %s, args = %s", buf1, buf2);
}

void
env_t::extend(frame_t* n_frame)
{
	m_frames.push_back(n_frame);
	return;
}

void
env_t::extend()
{
	frame_t* frame = new frame_t();
	m_frames.push_back(frame);
	return;
}

void
env_t::enclose()
{
	if (m_frames.empty()) return;

	frame_t* frame = m_frames.back();
	m_frames.pop_back();
	// delete frame;

	return;
}

obj_t*
env_t::lookup(const symbol_t* variable)
{
	std::vector<frame_t*>::reverse_iterator it;
	frame_t* f;
	obj_t* obj;

	for(it = m_frames.rbegin(); it != m_frames.rend(); it++)
	{
		f = (*it);
		obj = f->lookup(variable);
		if (obj != NULL) return obj;
	}

	char buf[1024];
	variable->print(buf, 1024);

	CALLERROR("unbound variable. : LOOKUP %s", buf);

	return g_obj_undef;
}

void
env_t::set(const symbol_t* variable, obj_t* value)
{
	std::vector<frame_t*>::reverse_iterator it;

	for(it = m_frames.rbegin(); it != m_frames.rend(); it++)
	{
		frame_t* f = (*it);
		if (f->change_binding(variable, value)) return;
	}

	char buf[1024];
	variable->print(buf, 1024);

	CALLERROR("unbound variable. -- SET %s", buf);
}

void
env_t::unbind(const symbol_t* variable)
{
	std::vector<frame_t*>::reverse_iterator it;

	for(it = m_frames.rbegin(); it != m_frames.rend(); it++)
	{
		frame_t* f = (*it);
		if (f->rem_binding(variable)) return;
	}

	char buf[1024];
	variable->print(buf, 1024);

	CALLERROR("unbound variable. -- UNBIND %s", buf);
}

void
env_t::define(const symbol_t* variable, obj_t* value)
{

	frame_t* f = m_frames.back();

	if (f->change_binding(variable, value)) return;

	f->add_binding(variable, value);

	return;
}

void env_t::error(const char* fname, unsigned int line, const char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	fprintf(stderr, "%s:%u !! ERROR in 'ENVIRONMENT' !! -- ", fname, line);
	vfprintf(stderr, fmt, arg);
	fprintf(stderr, "\n\n");

	fflush(stderr);

	va_end(arg);

	throw "ENVIRONMENT_ERROR";
}
