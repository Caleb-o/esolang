#pragma once
#include <string>
#include "token.hpp"

namespace Process {
	class Lexer {
		int m_line = { 1 }, m_col = { 1 };
		size_t m_ip = { 0 };
		std::string m_source;

	private:
		// Helper methods
		void skip_whitespace();
		char peek(size_t);

		Token *make_identifier();
		Token *make_string();
		Token *make_number();
		Token *make_single(TokenKind);

	public:
		Lexer(std::string);

		Token *get_token();
	};
}