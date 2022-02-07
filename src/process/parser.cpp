#include <iostream>
#include <exception>
#include "parser.hpp"
#include "util.hpp"


namespace Process {
	// Helpers
	static void error(std::string msg) {
		throw msg;
	}

	void Parser::push_byte(ByteCode byte) {

	}

	void Parser::push_bytes(ByteCode byte_a, ByteCode byte_b) {}

	void Parser::consume(TokenKind expected) {
		if (m_current->kind == expected) {
			delete m_current;
			m_current = m_lexer->get_token();

			std::cout << "Current token: " << m_current->lexeme << ", " << get_token_name(m_current->kind) << std::endl;
		} else {
			// TODO: Throw exception
			error(Util::string_format("Expected token '%s' but got '%s' (%s) on line %d at pos %d\n", get_token_name(expected), get_token_name(m_current->kind), m_current->lexeme.c_str(), m_current->line, m_current->col));
			delete m_current;
		}
	}

	void Parser::expr() {}

	void Parser::statement() {}

	void Parser::statement_list() {

	}

	void Parser::code_block() {
		consume(TokenKind::LCURLY);
		statement_list();
		consume(TokenKind::RCURLY);
	}

	void Parser::using_statement() {}

	void Parser::type_list() {

	}

	void Parser::parameter_list() {
		consume(TokenKind::LPAREN);
		type_list();
		consume(TokenKind::RPAREN);
	}

	void Parser::struct_def() {}

	void Parser::procedure_def() {
		consume(TokenKind::PROC);

		const char *id = m_current->lexeme.c_str();
		consume(TokenKind::ID);

		// TODO: Add proc def and get idx

		parameter_list();
		consume(TokenKind::ARROW);
		
		// TODO: Multiple return types
		const char *retid = m_current->lexeme.c_str();
		consume(TokenKind::TYPEID);

		code_block();

		push_bytes(ByteCode::RETURN, (ByteCode)0);
	}

	void Parser::program() {
		while(m_current->kind != TokenKind::ENDOFFILE) {
			switch(m_current->kind) {
				case TokenKind::USING: {
					using_statement();
					break;
				}

				case TokenKind::STRUCT: {
					struct_def();
					break;
				}

				case TokenKind::PROC: {
					procedure_def();
					break;
				}

				default: {
					// TODO: Throw exception
					error(Util::string_format("Unknown token found '%s'\n", get_token_name(m_current->kind)));
				}
			}
		}
	}


	Parser::Parser() {
		m_env = new Environment();
	}

	Environment *Parser::parse(std::string source) {
		m_lexer = new Lexer(source);
		m_current = m_lexer->get_token();

		program();

		delete m_lexer;
		return m_env;
	}
}