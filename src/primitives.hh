#ifndef PRIMITIVES_H_
#define PRIMITIVES_H_

namespace prim {

	obj_t* plus(obj_t* args);
	obj_t* minus(obj_t* args);
	obj_t* multiple(obj_t* args);
	obj_t* devide(obj_t* args);
	obj_t* equal(obj_t* args);
	obj_t* smaller_than(obj_t* args);
	obj_t* bigger_than(obj_t* args);
	obj_t* _not(obj_t* args);
	obj_t* car(obj_t* args);
	obj_t* cdr(obj_t* args);

	void setup_primitives(std::vector<const symbol_t*>* variables, std::vector<obj_t*>* values);

}

#endif
