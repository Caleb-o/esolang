#include <iostream>
#include <exception>
#include <algorithm>
#include <cstring>
#include "parser.hpp"
#include "util.hpp"
#include "native.hpp"
#include "../runtime/value.hpp"


using namespace Runtime;

namespace Process {
	// Helpers
	void Parser::error(std::string msg) {
		throw Util::string_format("%s on line %d at pos %d", msg.c_str(), m_current->line, m_current->col);
	}

	void Parser::warning(std::string msg) {
		std::cout << "[Warning] " << Util::string_format("%s on line %d at pos %d", msg.c_str(), m_current->line, m_current->col) << std::endl;
	}

	static std::string copy_lexeme_str(std::shared_ptr<Token> current) {
		return std::string(current->lexeme);
	}

	// Returns the sub index
	static size_t add_proc_def_tmp(std::shared_ptr<Environment> env, const char *id) {
		// Does not exist yet, we can just add it
		size_t idx = get_proc_idx(env, id);

		if (idx >= env->defs.procedures.size()) {
			env->defs.procedures.push_back(std::make_pair(std::string(id), std::vector<ProcedureDef>()));
			env->defs.procedures[env->defs.procedures.size()-1].second.push_back({});
			return env->defs.procedures[env->defs.procedures.size()-1].second.size() - 1;
		} else {
			env->defs.procedures[idx].second.push_back({});
			return env->defs.procedures[idx].second.size() - 1;
		}
	}

	static void verify_proc_def(std::shared_ptr<Environment> env, const char *id, ProcedureDef def) {
		// We must linearly search for a definition
		size_t idx = get_proc_idx(env, id);

		for(size_t defidx = 0; defidx < env->defs.procedures[idx].second.size() - 1; ++defidx) {
			// Cannot compare different length param count
			if (env->defs.procedures[idx].second[defidx].parameters.size() !=
				def.parameters.size()) {
				continue;
			}

			auto params_it = env->defs.procedures[idx].second[defidx].parameters.begin();
			auto def_params_it = def.parameters.begin();

			bool sameParams = true;
			
			// Similar parameter count, now we compare types
			while(params_it != env->defs.procedures[idx].second[defidx].parameters.end()) {
				if (params_it->kind != def_params_it->kind) {
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
		push_byte((ByteCode)byte);
	}

	void Parser::push_bytes(ByteCode byte_a, ByteCode byte_b) {
		push_byte(byte_a); push_byte(byte_b);
	}

	void Parser::push_bytes(ByteCode byte_a, size_t byte_b) {
		push_byte(byte_a); push_byte((ByteCode)byte_b);
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
				return add_literal_to_env(create_value(std::stoll(m_current->lexeme)));
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

		// TODO: Support '#' which captures *all* items on the stack, might need another opcode "CAPTURE_ALL"

		if (m_current->kind == TokenKind::BANG) {
			// Bind values from the stack, dynamicall
			consume(TokenKind::BANG);

			if (m_current->kind != TokenKind::INT_LIT) {
				error("Dynamic capture requires an integer literal");
			}

			capture_count = std::stoi(m_current->lexeme);
			consume(TokenKind::INT_LIT);
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
		push_bytes(ByteCode::CAPTURE, capture_count);
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
				error(Util::string_format("Unknown expr token found '%s'", m_current->lexeme.c_str()));
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
			case TokenKind::MOD:		push_byte(ByteCode::MOD); break;
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
			case TokenKind::OR:				push_byte(ByteCode::OR); break;
			case TokenKind::AND:			push_byte(ByteCode::AND); break;
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
		
		size_t proc_idx = get_proc_idx(m_env, id.c_str());

		// Trying to use a function that hasn't been defined yet
		if (proc_idx >= m_env->defs.procedures.size()) {
			error(
				Util::string_format("Trying to call procedure '%s' which has not been defined yet",
				id.c_str()
			));
		}

		// Get the current procedure idx and push it to the proc call
		// Note: We infer which overload to call at run-time
		push_bytes(ByteCode::PROCCALL, proc_idx);
	}

	void Parser::native_call_statement() {
		consume(TokenKind::AT);

		std::string id = copy_lexeme_str(m_current);
		consume(TokenKind::ID);

		size_t native_idx = get_native_idx(m_env, id.c_str());
		auto *native_it = &m_env->defs.native_procs[native_idx];

		if (native_it == &m_env->defs.native_procs.back()) {
			error(Util::string_format(
				"Unkown native procedure found '%s'",
				id.c_str()
			));
		}

		push_bytes(ByteCode::NATIVECALL, native_idx);
	}

	void Parser::binding_access_statement() {
		std::string binding = copy_lexeme_str(m_current);
		consume(TokenKind::ID);

		push_byte(ByteCode::LOAD_BINDING);

		auto name_it = std::find(m_env->idLiterals.begin(), m_env->idLiterals.end(), binding);

		if (name_it == m_env->idLiterals.end()) {
			m_env->idLiterals.push_back(binding);
			push_byte(m_env->idLiterals.size() - 1);
		} else {
			push_byte(std::distance(m_env->idLiterals.begin(), name_it));
		}
	}

	void Parser::bind_statement(BindFlag flag, bool unbind) {
		TokenKind kind;
		ByteCode byte;

		if (!unbind) {
			switch(flag) {
				case BindFlag::PLAIN: {
					kind = TokenKind::BIND;
					byte = ByteCode::BIND;
					break;
				}

				case BindFlag::STRICT: {
					kind = TokenKind::BIND_STRICT;
					byte = ByteCode::BIND_STRICT;
					break;
				}

				case BindFlag::PARAM: {
					kind = TokenKind::BIND_PARAM;
					byte = ByteCode::BIND_PARAM;
					break;
				}
			}
		} else {
			kind = TokenKind::UNBIND;
			byte = ByteCode::UNBIND;
		}

		consume(kind);

		push_bytes(byte, 0);
		size_t bind_len = (m_is_top_level) ? m_top_level.size() - 1 : m_env->code.size() - 1;
		size_t bind_count = 0;
		std::vector<size_t> bindidx;

		consume(TokenKind::CAPTURE);

		while(m_current->kind != TokenKind::CAPTURE) {
			std::string id = copy_lexeme_str(m_current);
			consume(TokenKind::ID);

			auto name_it = std::find(m_env->idLiterals.begin(), m_env->idLiterals.end(), id);

			if (name_it == m_env->idLiterals.end()) {
				m_env->idLiterals.push_back(id);
				size_t idx = m_env->idLiterals.size() - 1;
				bindidx.push_back(idx);
				push_byte(idx);
			} else {
				size_t idx = std::distance(m_env->idLiterals.begin(), name_it);
				bindidx.push_back(idx);
				push_byte(idx);
			}

			bind_count++;

			// Multiple binds
			if (m_current->kind == TokenKind::COMMA) {
				consume(TokenKind::COMMA);
			}
		}

		consume(TokenKind::CAPTURE);

		// Get count of bindings
		m_env->code[bind_len] = (ByteCode)bind_count;

		// "Scope" bound bindings
		if (m_current->kind == TokenKind::LCURLY) {
			code_block();

			push_bytes(ByteCode::UNBIND, bind_count);
			for(auto idx : bindidx) {
				push_byte(idx);
			}
		}
	}

	void Parser::statement() {
		switch(m_current->kind) {
			// Arithmetic
			case TokenKind::PLUS: case TokenKind::MINUS:
			case TokenKind::STAR: case TokenKind::SLASH:
			case TokenKind::MOD: {
				arithmetic_statement();
				break;
			}

			// Comparison operators
			case TokenKind::GREATER: case TokenKind::GREATER_EQ:
			case TokenKind::LESS: case TokenKind::LESS_EQ: 
			case TokenKind::EQUAL: case TokenKind::OR:
			case TokenKind::AND: {
				comparison_statement();
				break;
			}

			// Keywords
			case TokenKind::BIND:			bind_statement(BindFlag::PLAIN, false); break;
			case TokenKind::BIND_STRICT:	bind_statement(BindFlag::STRICT, false); break;
			case TokenKind::UNBIND:			bind_statement(BindFlag::PLAIN, true); break;
			case TokenKind::ID:				binding_access_statement(); break;
			case TokenKind::BANG:			proc_call_statement(); break;
			case TokenKind::AT:				native_call_statement(); break;
			case TokenKind::IF:				if_statement(); break;
			case TokenKind::LOOP:			loop_statement(); break;
			case TokenKind::DUP:			consume(m_current->kind); push_byte(ByteCode::DUPLICATE); break;
			case TokenKind::ROT:			consume(m_current->kind); push_byte(ByteCode::ROTATE); break;
			case TokenKind::POP:			consume(m_current->kind); push_byte(ByteCode::DROP); break;
			case TokenKind::SWAP:			consume(m_current->kind); push_byte(ByteCode::SWAP); break;
			case TokenKind::NOT: 			consume(m_current->kind); push_byte(ByteCode::NOT); break;

			case TokenKind::PRINT:			consume(m_current->kind); push_byte(ByteCode::PRINT); break;
			case TokenKind::PRINTLN:		consume(m_current->kind); push_byte(ByteCode::PRINTLN); break;

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
		bool is_std = false;
		
		// NOTE: This is like consume, but if we consume, it messes with
		// 		 the order
		if (m_current->kind != TokenKind::STRING_LIT) {
			error("Import expects string literal");
		}

		if (import.size() > 4 && std::strncmp(import.c_str(), "std:", 4) == 0) {
			is_std = true;
			import = import.substr(4);
		}


		// TODO: Check if import contains std, to redirect file 
		std::string final_file = Util::string_format("%s/%s.eso",
			(is_std) ? "std" : m_base_dir.c_str(),
			import.c_str()
		);

		std::ifstream file(final_file.c_str());

		if (!file.good()) {
			error(Util::string_format(
				"File does not exist '%s'",
				import.c_str()
			));
		}
		file.close();

		std::string source = Util::read_file(final_file.c_str());

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
		ProcedureDef procDef = {0};
		size_t proc_idx = get_proc_idx(m_env, id);

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

			// Cannot use capture as an argument, only return
			if (type_id == "capture") {
				error("Cannot use 'capture' as a parameter type. Captures can only be returne");
			}

			auto *params = &m_env->defs.procedures[proc_idx].second[m_env->defs.procedures[proc_idx].second.size()-1];
			

			// Create each parameter
			for(auto& id : ids) {
				ValueKind kind = kind_from_str(type_id.c_str());

				if (kind == ValueKind::VOID) {
					error(Util::string_format("Cannot use type '%s' in parameter list", id));
				}
				procDef.parameters.push_back({ id, kind });
				params->parameters.push_back({ id, kind });

				if (std::find(m_env->idLiterals.begin(), m_env->idLiterals.end(), id) == m_env->idLiterals.end()) {
					m_env->idLiterals.push_back(id);
				}
			}

			// Multiple parameters
			if (m_current->kind == TokenKind::COMMA) {
				consume(m_current->kind);

				if (m_current->kind == endType && is_proc) {
					error(Util::string_format("Unexpected character found '%s' after comma", get_token_name(m_current->kind)));
				}
			}
		}
		verify_proc_def(m_env, id, procDef);
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
		size_t proc_idx = get_proc_idx(m_env, id.c_str());
		m_env->defs.procedures[proc_idx].second[sub_idx].startIdx = (m_env->code.size() == 0) ? 0 : m_env->code.size();

		parameter_list(id.c_str());
		consume(TokenKind::ARROW);
		
		// TODO: Allow for capture as a return type
		std::string retid = copy_lexeme_str(m_current);
		consume(TokenKind::TYPEID);

		bool using_capture = false;
		m_env->defs.procedures[proc_idx].second[sub_idx].returnTypes.push_back(kind_from_str(retid.c_str()));

		if (std::strcmp(retid.c_str(), "capture") == 0) {
			using_capture = true;
		}

		if (m_current->kind == TokenKind::COMMA) {
			if (std::strcmp(retid.c_str(), "void") == 0) {
				error("Cannot use void in a return list");
			}
			
			consume(TokenKind::COMMA);

			while(m_current->kind != TokenKind::LCURLY) {
				std::string retid = copy_lexeme_str(m_current);
				consume(TokenKind::TYPEID);

				if (std::strcmp(retid.c_str(), "capture") == 0) {
					using_capture = true;
				}

				if (using_capture) {
					error("Captures can only be used as a single return type");
				}

				m_env->defs.procedures[proc_idx].second[sub_idx].returnTypes.push_back(kind_from_str(retid.c_str()));

				if (m_current->kind == TokenKind::COMMA) {
					consume(TokenKind::COMMA);
				}
			}
		}


		// Check each parameter and push a bind opcode with each param
		if (m_env->defs.procedures[proc_idx].second[sub_idx].parameters.size() > 0) {
			push_bytes(ByteCode::BIND_PARAM, m_env->defs.procedures[proc_idx].second[sub_idx].parameters.size());

			for(auto param : m_env->defs.procedures[proc_idx].second[sub_idx].parameters) {
				auto name_it = std::find(m_env->idLiterals.begin(), m_env->idLiterals.end(), param.id);

				if (name_it == m_env->idLiterals.end()) {
					m_env->idLiterals.push_back(param.id);
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
			if (m_env->defs.procedures[proc_idx].second.size() > 1) {
				error("Multiple definitions of main");
			}

			// Check main arguments and return
			if (m_env->defs.procedures[proc_idx].second[0].parameters.size() > 0 ||
				m_env->defs.procedures[proc_idx].second[0].returnTypes.size() > 1 ||
				m_env->defs.procedures[proc_idx].second[0].returnTypes[0] != ValueKind::VOID) {
				error("Main must be defined without arguments and return void");
			}

			push_byte(ByteCode::HALT);
		}
	}

	void Parser::program() {
		while(m_current->kind != TokenKind::ENDOFFILE) {
			m_is_top_level = false;

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
					// TODO: Add top-level code
					// Assume top-level is a "procedure"? Will have to add resolution between frames when using bindings
					// m_is_top_level = true;
					// statement();
					error(Util::string_format(
						"Unknown token found '%s'",
						get_token_name(m_current->kind)
					));
					break;
				}
			}
		}
	}


	Parser::Parser(std::string base) :m_base_dir(base) {
		m_env = std::shared_ptr<Environment>(new Environment());
	}

	std::shared_ptr<Environment> Parser::parse(std::string source, std::vector<std::string> argv) {
		m_lexer = std::make_unique<Lexer>(Lexer(source));
		m_current = m_lexer->get_token();

		size_t hash = Util::hash(source.c_str(), source.size());
		m_file_hashes.insert(hash);

		def_native_procs(m_env);

		// Import argv and bind argc
		m_env->argv = argv;

		program();
		m_is_top_level = false;
		m_completed = true;

		return m_env;
	}
}