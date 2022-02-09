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

	void Parser::warning(std::string msg) {
		std::cout << "[Warning] " << Util::string_format("%s on line %d at pos %d", msg.c_str(), m_current->line, m_current->col);
	}

	static std::string copy_lexeme_str(std::shared_ptr<Token> current) {
		return std::string(current->lexeme);
	}

	// Returns the sub index
	static size_t add_proc_def_tmp(std::shared_ptr<Environment> env, const char *id) {
		// Does not exist yet, we can just add it
		env->defs.procedures[id].push_back({});
		return env->defs.procedures[id].size() - 1;
	}

	static void verify_proc_def(std::shared_ptr<Environment> env, const char *id, ProcedureDef def) {
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
				if (params_it->second != def_params_it->second) {
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

	void Parser::push_byte(size_t byte) {
		m_env->code.push_back((ByteCode)byte);
	}

	void Parser::push_bytes(ByteCode byte_a, ByteCode byte_b) {
		m_env->code.push_back(byte_a);
		m_env->code.push_back(byte_b);
	}

	void Parser::push_bytes(ByteCode byte_a, size_t byte_b) {
		m_env->code.push_back(byte_a);
		m_env->code.push_back((ByteCode)byte_b);
	}

	void Parser::consume(TokenKind expected) {
		if (m_current->kind == expected) {
			m_current = m_lexer->get_token();
		} else {
			// TODO: Throw exception
			error(Util::string_format("Expected token '%s' but got '%s' (%s) on line %d at pos %d\n", get_token_name(expected), get_token_name(m_current->kind), m_current->lexeme.c_str(), m_current->line, m_current->col));
		}
	}

	size_t Parser::add_literal_to_env(std::shared_ptr<Value> value) {
		m_env->literals.push_back(value);
		return m_env->literals.size() - 1;
	}

	size_t Parser::add_literal() {
		switch(m_current->kind) {
			case TokenKind::INT_LIT: {
				return add_literal_to_env(create_value(std::stoi(m_current->lexeme)));
			}

			case TokenKind::FLOAT_LIT: {
				return add_literal_to_env(create_value(std::stof(m_current->lexeme)));
			}

			case TokenKind::BOOL_LIT: {
				bool value = (m_current->lexeme == "true") ? true : false;
				return add_literal_to_env(create_value(value));
			}

			case TokenKind::STRING_LIT:	{
				return add_literal_to_env(create_value(copy_lexeme_str(m_current)));
			}
		}
	}

	void Parser::capture_list() {
		consume(TokenKind::CAPTURE);
		size_t capture_count = 0;

		if (m_current->kind == TokenKind::BANG) {
			// Bind values from the stack, dynamicall
			consume(TokenKind::BANG);

			if (m_current->kind != TokenKind::INT_LIT) {
				error("Dynamic capture requires an integer literal");
			}

			// Get the count
			expr();
			capture_count = m_env->literals[m_env->code[m_env->code.size()-1]]->data.integer;

			// Remove push
			m_env->code.pop_back(); m_env->code.pop_back();
		} else {
			// We will bind a series of expressions
			while(m_current->kind != TokenKind::CAPTURE) {
				if (m_current->kind == TokenKind::ENDOFFILE) {
					error("Unterminated capture list");
				}

				expr();
				capture_count++;
			}
		}

		consume(TokenKind::CAPTURE);

		// Note: This has to come last since we will capture values previous
		if (capture_count > 0) {
			push_bytes(ByteCode::CAPTURE, capture_count);
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

			default: {
				error(Util::string_format("Unknown token found '%s'", m_current->lexeme.c_str()));
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

	void Parser::comparison_statement() {
		auto op = m_current->kind;
		consume(op);

		switch(op) {
			case TokenKind::GREATER:		push_byte(ByteCode::GREATER); break;
			case TokenKind::GREATER_EQ:		push_byte(ByteCode::GREATER_EQ); break;
			case TokenKind::LESS:			push_byte(ByteCode::LESS); break;
			case TokenKind::LESS_EQ:		push_byte(ByteCode::LESS_EQ); break;
			case TokenKind::EQUAL:			push_byte(ByteCode::EQUAL); break;
		}
	}

	void Parser::if_statement() {
		consume(TokenKind::IF);

		push_bytes(ByteCode::IF, 0);
		size_t false_idx = m_env->code.size() - 1;

		code_block();

		// Parse the else path
		if (m_current->kind == TokenKind::ELSE) {
			consume(TokenKind::ELSE);

			// Patch a jump to end for true body
			push_bytes(ByteCode::GOTO, 0);
			size_t goto_idx = m_env->code.size() - 1;

			// Patch false path to here
			m_env->code[false_idx] = (ByteCode)(m_env->code.size() - 1);

			code_block();

			// Patch goto path to here
			m_env->code[goto_idx] = (ByteCode)(m_env->code.size() - 1);
		} else {
			// Patch false path to here
			m_env->code[false_idx] = (ByteCode)(m_env->code.size() - 1);
		}

		// Another else block, which is not allowed
		if (m_current->kind == TokenKind::ELSE) {
			error("If block cannot contain several else blocks");
		}
	}

	void Parser::loop_statement() {
		consume(TokenKind::LOOP);

		push_bytes(ByteCode::LOOP, 0);
		size_t false_idx = m_env->code.size() - 1;

		code_block();

		push_bytes(ByteCode::GOTO, false_idx-2);
		m_env->code[false_idx] = (ByteCode)(m_env->code.size() - 1);
	}

	void Parser::proc_call_statement() {
		consume(TokenKind::BANG);

		std::string id = copy_lexeme_str(m_current);
		consume(TokenKind::ID);

		auto proc_it = m_env->defs.procedures.find(id);

		// Trying to use a function that hasn't been defined yet
		if (proc_it == m_env->defs.procedures.end()) {
			error(
				Util::string_format("Trying to call procedure '%s' which has not been defined yet",
				id.c_str()
			));
		}

		// Get the current procedure idx and push it to the proc call
		// Note: We infer which overload to call at run-time
		size_t proc_idx = std::distance(m_env->defs.procedures.begin(), proc_it);
		push_bytes(ByteCode::PROCCALL, proc_idx);
	}

	void Parser::binding_access_statement() {
		std::string binding = copy_lexeme_str(m_current);
		consume(TokenKind::ID);

		m_env->idLiterals.push_back(binding);

		push_bytes(ByteCode::LOAD_BINDING, m_env->idLiterals.size()-1);
	}

	void Parser::statement() {
		switch(m_current->kind) {
			// Arithmetic
			case TokenKind::PLUS: case TokenKind::MINUS:
			case TokenKind::STAR: case TokenKind::SLASH: {
				arithmetic_statement();
				break;
			}

			// Comparison operators
			case TokenKind::GREATER: case TokenKind::GREATER_EQ:
			case TokenKind::LESS: case TokenKind::LESS_EQ: 
			case TokenKind::EQUAL: {
				comparison_statement();
				break;
			}

			// Keywords
			case TokenKind::ID:			binding_access_statement(); break;
			case TokenKind::BANG:		proc_call_statement(); break;
			case TokenKind::IF:			if_statement(); break;
			case TokenKind::LOOP:		loop_statement(); break;
			case TokenKind::DUP:		consume(m_current->kind); push_byte(ByteCode::DUPLICATE); break;
			case TokenKind::POP:		consume(m_current->kind); push_byte(ByteCode::DROP); break;
			case TokenKind::SWAP:		consume(m_current->kind); push_byte(ByteCode::SWAP); break;

			case TokenKind::PRINT:		consume(m_current->kind); push_byte(ByteCode::PRINT); break;
			case TokenKind::PRINTLN:	consume(m_current->kind); push_byte(ByteCode::PRINTLN); break;

			case TokenKind::CAPTURE: {
				capture_list();
				break;
			}

			default: expr(); break;
		}
	}

	void Parser::statement_list() {
		// Consume statements until we are at the end of the block
		while(m_current->kind != TokenKind::ENDOFFILE && m_current->kind != TokenKind::RCURLY) {
			statement();
		}
	}

	void Parser::code_block() {
		consume(TokenKind::LCURLY);
		statement_list();
		consume(TokenKind::RCURLY);
	}

	void Parser::using_statement() {
		consume(TokenKind::USING);

		std::string import = copy_lexeme_str(m_current);
		
		// NOTE: This is like consume, but if we consume, it messes with
		// 		 the order
		if (m_current->kind != TokenKind::STRING_LIT) {
			error("Import expects string literal");
		}

		// TODO: Check if import contains std, to redirect file include
		std::string source = Util::read_file(
			Util::string_format("%s/%s.eso",
				m_base_dir.c_str(),
				import.c_str()
			).c_str()
		);

		size_t hash = Util::hash(source.c_str(), source.size());
		if (m_file_hashes.find(hash) != m_file_hashes.end()) {
			consume(TokenKind::STRING_LIT);

			warning(Util::string_format(
				"Skipping import '%s'",
				import.c_str()
			));
			return;
		}

		m_file_hashes.insert(hash);

		m_lexers.push_back(m_lexer);
		m_lexer = std::make_shared<Lexer>(Lexer(source));
		m_current = m_lexer->get_token();

		m_ignore_main = true;

		program();

		m_lexer.swap(m_lexers.back());
		m_lexers.pop_back();
		m_current = m_lexer->get_token();

		if (m_lexers.size() == 0) {
			m_ignore_main = false;
		}
	}

	void Parser::type_list(const char *id, bool is_proc) {
		// FIXME: This will be geared towards a proc, but will be required for structs
		TokenKind endType = (is_proc) ? TokenKind::RPAREN : TokenKind::RCURLY;

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

			std::string type_id = copy_lexeme_str(m_current);
			consume(TokenKind::TYPEID);

			auto *params = &m_env->defs.procedures[id][m_env->defs.procedures[id].size()-1];
			ProcedureDef procDef = {0};

			// Create each parameter
			for(auto& id : ids) {
				ValueKind kind = kind_from_str(type_id.c_str());

				if (kind == ValueKind::VOID) {
					error(Util::string_format("Cannot use type '%s' in parameter list", id));
				}
				procDef.parameters[id] = { kind };
				params->parameters[id] = { kind };
			}

			verify_proc_def(m_env, id, procDef);

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

	void Parser::struct_def() {
		error("Struct is unimplemented");
	}

	void Parser::procedure_def() {
		consume(TokenKind::PROC);

		// We must copy here since pointing to the c_str gives us a weird result
		std::string id = copy_lexeme_str(m_current);
		consume(TokenKind::ID);

		if (std::strcmp(id.c_str(), "main") == 0 && m_ignore_main) {
			while(m_current->kind != TokenKind::RCURLY) {
				consume(m_current->kind);
			}
			consume(TokenKind::RCURLY);
			return;
		}

		size_t sub_idx = add_proc_def_tmp(m_env, id.c_str());
		m_env->defs.procedures[id][sub_idx].startIdx = (m_env->code.size() == 0) ? 0 : m_env->code.size();

		parameter_list(id.c_str());
		consume(TokenKind::ARROW);
		
		// TODO: Multiple return types
		std::string retid = copy_lexeme_str(m_current);
		consume(TokenKind::TYPEID);

		m_env->defs.procedures[id][sub_idx].returnTypes.push_back(kind_from_str(retid.c_str()));

		// Check each parameter and push a bind opcode with each param
		if (m_env->defs.procedures[id][sub_idx].parameters.size() > 0) {
			push_bytes(ByteCode::BIND_MOVE, m_env->defs.procedures[id][sub_idx].parameters.size());

			for(auto param : m_env->defs.procedures[id][sub_idx].parameters) {
				auto name_it = std::find(m_env->idLiterals.begin(), m_env->idLiterals.end(), param.first);

				if (name_it == m_env->idLiterals.end()) {
					m_env->idLiterals.push_back(param.first);
					push_byte(m_env->idLiterals.size() - 1);
				} else {
					push_byte(std::distance(m_env->idLiterals.begin(), name_it));
				}
			}
		}
		
		// Parse statements within code block
		code_block();

		// Conditional bytes, since main cannot return
		if (std::strcmp(id.c_str(), "main") != 0) {
			push_bytes(ByteCode::RETURN, sub_idx);
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
					error(Util::string_format("Unknown token found '%s'\n", get_token_name(m_current->kind)));
				}
			}
		}
	}


	Parser::Parser(std::string base) :m_base_dir(base) {
		m_env = std::shared_ptr<Environment>(new Environment());
	}

	std::shared_ptr<Environment> Parser::parse(std::string source) {
		m_lexer = std::make_unique<Lexer>(Lexer(source));
		m_current = m_lexer->get_token();

		size_t hash = Util::hash(source.c_str(), source.size());
		m_file_hashes.insert(hash);

		program();
		m_completed = true;

		return m_env;
	}
}