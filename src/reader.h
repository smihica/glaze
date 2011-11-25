#ifndef READER_H_
#define READER_H_

#include "core.h"
#include "object.h"
#include "shared.h"

namespace glaze {

	class reader_t {
	public:
		reader_t(Shared* sh);
		reader_t(Shared* sh, int fd);
		reader_t(Shared* sh, FILE* fp);
		reader_t(Shared* sh, const char* src);
		obj_t* read_expr();

		obj_t* S_EOF;
	private:

		// values
		Shared* shared;

		int m_fd;
		FILE* m_fp;
		const char* m_src;

		int m_mode;
		size_t m_read;

		bool m_ungetbuf_valid;
		int  m_ungetbuf;

		static bool s_char_map_ready;
		static uint8_t s_char_map[128];
		static void make_char_map();


		//funcs
		void init();
		bool whitespace_p(int c);
		bool delimited(int c);

		int get();
		int lookahead();
		void unget();

		// number
		size_t read_thing(char* buf, size_t size);
		obj_t* make_number(const char* buf, size_t len);
		obj_t* read_number();

		// string
		obj_t* make_string(const char* buf, size_t len);
		obj_t* read_string();

		// list
		obj_t* make_list_2items(obj_t* first, obj_t* second);
		obj_t* reverse_list(obj_t* lst, obj_t* tail);
		obj_t* read_list(bool bracketed);

		// symbol
		obj_t* make_symbol(const char* buf);
		obj_t* read_symbol();

		obj_t* read_token();
		void error(const char* fname, unsigned int line, const char* fmt, ...);

		// boolean

		obj_t* S_DOT;
		obj_t* S_LPAREN;
		obj_t* S_RPAREN;
		obj_t* S_LBRACK;
		obj_t* S_RBRACK;
		obj_t* S_QUOTE;
		obj_t* S_QUASIQUOTE;
		obj_t* S_UNQUOTE;
		obj_t* S_UNQUOTE_SPLICING;

	};
}

#endif
