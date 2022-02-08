#pragma once
#include <string>
#include <memory>
#include "token.hpp"

namespace Process {
	class Lexer {
		int m_line = { 1 }, m_col = { 1 };
		size_t m_ip = { 0 };
		std::string m_source;

	private:
		// Helper methods
		void error(std::string);
		void skip_whitespace();
		char peek(size_t);

		std::shared_ptr<Token> make_identifier();
		std::shared_ptr<Token> make_string();
		std::shared_ptr<Token> make_number();
		std::shared_ptr<Token> make_single(TokenKind);

	public:
		Lexer(std::string);
		~Lexer() {}

		std::shared_ptr<Token> get_token();
	};
}