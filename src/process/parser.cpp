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
	void Parser::error(std::string msg) {
		throw Util::string_format("%s on line %d at pos %d", msg.c_str(), m_current->line, m_current->col);
	}

	static char *copy_lexeme(Token *current) {
		char *id = new char[current->lexeme.size() + 1];
		std::strncpy(id, current->lexeme.c_str(), current->lexeme.size());
		id[current->lexeme.size()] = '\0';
		return id;
	}

	// Returns the sub index
	static size_t add_proc_def_tmp(Environment *env, const char *id) {
		// Does not exist yet, we can just add it
		env->defs.procedures[id].push_back({});
		return env->defs.procedures[id].size() - 1;
	}

	static void verify_proc_def(Environment *env, const char *id, ProcedureDef def) {
		// We must linearly search for a definition
		for(size_t defidx = 0; defidx < env->defs.procedures[id].size() - 1; ++defidx) {
			// Cannot compare different length param count
			if (env->defs.procedures[id][defidx].parameters.size() !=
				def.parameters.size()) {
				continue;
			}

			auto params_it = env->defs.procedures[id][defidx].parameters.begin();
			auto def_params_it = def.parameters.begin();

			bool sameParams = true;
			
			// Similar parameter count, now we compare types
			while(params_it != env->defs.procedures[id][defidx].parameters.end()) {
				if (params_it->second.kind != def_params_it->second.kind) {
					sameParams = false;
					break;
				}

				params_it++;
				def_params_it++;
			}

			// A proc was found with the same parameters
			if (sameParams) {
				throw Util::string_format("Overload for '%s' already exists with current parameters", id);
			}
		}
	}

	// Parser

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
				return (ByteCode)add_literal_to_env({ 
					ValueKind::INT, 
					{ .integer=std::stoi(m_current->lexeme) }, 
					true
				});
			}

			case TokenKind::FLOAT_LIT: {
				return (ByteCode)add_literal_to_env({ 
					ValueKind::STRING,
					{ .floating=std::stof(m_current->lexeme) }, 
					true
				});
			}

			case TokenKind::BOOL_LIT: {
				bool value;

				if (m_current->lexeme == "true") {
					value = true;
				} else if (m_current->lexeme == "false") {
					value = false;
				}
				return (ByteCode)add_literal_to_env({ 
					ValueKind::BOOL,
					{ .boolean=value }, 
					true
				});
			}

			case TokenKind::STRING_LIT: {
				return (ByteCode)add_literal_to_env({ 
					ValueKind::STRING,
					{ .string=copy_lexeme(m_current) }, 
					true
				});
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

			case TokenKind::DUP:		consume(m_current->kind); push_byte(ByteCode::DUPLICATE); break;
			case TokenKind::POP:		consume(m_current->kind); push_byte(ByteCode::DROP); break;
			case TokenKind::SWAP:		consume(m_current->kind); push_byte(ByteCode::SWAP); break;

			case TokenKind::PRINT:		consume(m_current->kind); push_byte(ByteCode::PRINT); break;
			case TokenKind::PRINTLN:	consume(m_current->kind); push_byte(ByteCode::PRINTLN); break;

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

	void Parser::using_statement() {
		consume(m_current->kind);
	}

	void Parser::type_list(const char *id, bool is_proc) {
		// FIXME: This will be geared towards a proc, but will be required for structs
		TokenKind endType = (is_proc) ? TokenKind::RPAREN : TokenKind::RCURLY;
		bool hasMove = false;

		while(m_current->kind != endType) {
			std::vector<std::string> ids;

			// Parse multiple names for a single type
			while (m_current->kind != TokenKind::COLON) {
				ids.push_back(m_current->lexeme);
				consume(TokenKind::ID);

				// Multiple names
				if (m_current->kind == TokenKind::COMMA) {
					consume(m_current->kind);
				}
			}

			consume(TokenKind::COLON);
			bool isMoved = true;

			// Modifier for duplicating data instead of moving
			if (m_current->kind == TokenKind::DUP) {
				if (hasMove) {
					// We cannot move values from within the stack, it must be at the
					// end. We can duplicate them all however.
					error("Cannot use dup modifier after move parameters");
				}

				consume(m_current->kind);
				isMoved = false;
			} else {
				hasMove = true;
			}

			char *type_id = copy_lexeme(m_current);
			consume(TokenKind::TYPEID);

			auto *params = &m_env->defs.procedures[id][m_env->defs.procedures[id].size()-1];
			ProcedureDef procDef = {0};

			// Create each parameter
			for(auto& id : ids) {
				ValueKind kind = kind_from_str(type_id);

				if (kind == ValueKind::VOID) {
					error(Util::string_format("Cannot use type '%s' in parameter list", id));
				}
				procDef.parameters[id] = { kind, hasMove };
				params->parameters[id] = { kind, hasMove };
			}

			verify_proc_def(m_env, id, procDef);

			// Cleanup
			delete[] type_id;

			// Multiple parameters
			if (m_current->kind == TokenKind::COMMA) {
				consume(m_current->kind);

				if (m_current->kind == endType && is_proc) {
					error(Util::string_format("Unexpected character found '%s' after comma", get_token_name(m_current->kind)));
				}
			}
		}
	}

	void Parser::parameter_list(const char *id) {
		consume(TokenKind::LPAREN);
		type_list(id, true);
		consume(TokenKind::RPAREN);
	}

	void Parser::struct_def() {}

	void Parser::procedure_def() {
		consume(TokenKind::PROC);

		// We must copy here since pointing to the c_str gives us a weird result
		char *id = copy_lexeme(m_current);
		consume(TokenKind::ID);

		size_t sub_idx = add_proc_def_tmp(m_env, id);
		m_env->defs.procedures[id][sub_idx].startIdx = (m_env->code.size() == 0) ? 0 : m_env->code.size();

		// TODO: Add proc def and get idx
		parameter_list(id);
		consume(TokenKind::ARROW);
		
		// TODO: Multiple return types
		const char *retid = copy_lexeme(m_current);
		consume(TokenKind::TYPEID);
		m_env->defs.procedures[id][sub_idx].returnTypes.push_back(kind_from_str(retid));

		code_block();

		// Conditional bytes, since main cannot return
		if (std::strcmp(id, "main") != 0) {
			size_t proc_idx = std::distance(m_env->defs.procedures.begin(), m_env->defs.procedures.find(id));
			push_bytes(ByteCode::RETURN, (ByteCode)proc_idx);
			push_byte((ByteCode)sub_idx);
		} else {
			// Check for main def count
			if (m_env->defs.procedures[id].size() > 1) {
				error("Multiple definitions of main");
			}

			// Check main arguments and return
			if (m_env->defs.procedures[id][0].parameters.size() > 0 ||
				m_env->defs.procedures[id][0].returnTypes.size() > 1 ||
				m_env->defs.procedures[id][0].returnTypes[0] != ValueKind::VOID) {
				error("Main must be defined without arguments and return void");
			}

			push_byte(ByteCode::HALT);
		}

		delete[] retid;
		delete[] id;
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