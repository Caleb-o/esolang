#include <iostream>
#include <exception>
#include <algorithm>
#include <cstring>
#include "parser.hpp"
#include "util.hpp"
#include "../runtime/value.hpp"


using namespace Runtime;

namespace Process {
	// Helpers
	static void error(std::string msg) {
		throw msg;
	}

	void Parser::push_byte(ByteCode byte) {
		m_env->code.push_back(byte);
	}

	void Parser::push_bytes(ByteCode byte_a, ByteCode byte_b) {
		m_env->code.push_back(byte_a);
		m_env->code.push_back(byte_b);
	}

	void Parser::consume(TokenKind expected) {
		if (m_current->kind == expected) {
			delete m_current;
			m_current = m_lexer->get_token();
		} else {
			// TODO: Throw exception
			error(Util::string_format("Expected token '%s' but got '%s' (%s) on line %d at pos %d\n", get_token_name(expected), get_token_name(m_current->kind), m_current->lexeme.c_str(), m_current->line, m_current->col));
			delete m_current;
		}
	}

	size_t Parser::add_literal_to_env(Value value) {
		m_env->literals.push_back(value);
		return m_env->literals.size() - 1;
	}

	ByteCode Parser::add_literal() {
		switch(m_current->kind) {
			case TokenKind::INT_LIT: {
				Value val = { 
					ValueKind::INT, 
					new (ValueData){ .integer=std::stoi(m_current->lexeme.c_str()) }, 
					true
				};
				return (ByteCode)add_literal_to_env(val);
			}
		}
	}

	void Parser::expr() {
		switch(m_current->kind) {
			case TokenKind::INT_LIT: case TokenKind::FLOAT_LIT:
			case TokenKind::BOOL_LIT: case TokenKind::STRING_LIT: {
				push_bytes(ByteCode::PUSH, add_literal());
				consume(m_current->kind);
				break;
			}
		}
	}

	void Parser::arithmetic_statement() {
		TokenKind op = m_current->kind;
		consume(op);

		switch(op) {
			case TokenKind::PLUS:		push_byte(ByteCode::ADD); break;
			case TokenKind::MINUS:		push_byte(ByteCode::SUB); break;
			case TokenKind::STAR:		push_byte(ByteCode::MUL); break;
			case TokenKind::SLASH:		push_byte(ByteCode::DIV); break;
		}
	}

	void Parser::statement() {
		switch(m_current->kind) {
			case TokenKind::PLUS: case TokenKind::MINUS:
			case TokenKind::STAR: case TokenKind::SLASH: {
				arithmetic_statement();
				break;
			}

			default: expr(); break;
		}
	}

	void Parser::statement_list() {
		// Consume statements until we are at the end of the block
		while(m_current->kind != TokenKind::RCURLY) {
			statement();
		}
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

		// We must copy here since pointing to the c_str gives us a weird result
		char *id = new char[m_current->lexeme.size() + 1];
		std::strncpy(id, m_current->lexeme.c_str(), m_current->lexeme.size());
		id[m_current->lexeme.size()] = '\0';

		consume(TokenKind::ID);

		// TODO: Add proc def and get idx

		parameter_list();
		consume(TokenKind::ARROW);
		
		// TODO: Multiple return types
		const char *retid = m_current->lexeme.c_str();
		consume(TokenKind::TYPEID);

		code_block();

		// Conditional bytes, since main cannot return
		if (std::strcmp(id, "main") != 0) {
			push_bytes(ByteCode::RETURN, (ByteCode)0);
		} else {
			push_byte(ByteCode::HALT);
		}
	}

	void Parser::program() {
		std::cout << "Current " << get_token_name(m_current->kind) << std::endl;

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

	Environment *Parser::parse(char *source) {
		m_lexer = new Lexer(source);
		m_current = m_lexer->get_token();

		program();

		delete m_lexer;
		return m_env;
	}
}