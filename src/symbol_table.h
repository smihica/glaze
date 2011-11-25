#ifndef SYMBOL_TABLE_H_
#define SYMBOL_TABLE_H_

#include <map>
#include <string>

namespace glaze {

	class symbol_table {
	public:
		symbol_table() {};
		~symbol_table();
		const symbol_t* get(const char* name);
		void      clear();
		void	  print();

	private:
		std::map<std::string, const symbol_t*> m_table;

	};

}

#endif
