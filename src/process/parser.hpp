#pragma once
#include <string>
#include <map>
#include <unordered_set>
#include "bytecode.hpp"
#include "environment.hpp"
#include "lexer.hpp"


namespace Process {
	class Parser {
		// Hash each file we include, so we can check it when importing, mitigates including the same file twice
		std::unordered_set<size_t> m_file_hashes;
		// This is a pointer, so we can later swap it out when importing another module
		Lexer *m_lexer = { 0 };
		Token *m_current = { 0 };
		Environment *m_env;
		bool m_completed = { false };

	private:
		void error(std::string msg);
		
		void push_byte(ByteCode);
		void push_byte(size_t);
		void push_bytes(ByteCode, ByteCode);
		void push_bytes(ByteCode, size_t);

		void consume(TokenKind);

		size_t add_literal_to_env(Value *value);
		size_t add_literal();
		void capture_list();

		void expr();
		void arithmetic_statement();
		void comparison_statement();
		void if_statement();
		void proc_call_statement();
		void statement();
		void statement_list();
		void code_block();
		void using_statement();
		void type_list(const char *, bool);
		void parameter_list(const char *);
		void struct_def();
		void procedure_def();
		void program();

	public:
		Parser();
		// We will need to delete the last token
		~Parser() { 
			if(m_current) delete m_current;
			if (m_lexer) delete m_lexer;

			if (!m_completed) {
				if (m_env) {
					for(size_t i = 0; i < m_env->literals.size(); ++i) {
						delete m_env->literals[i];
					}

					delete m_env;
				}
			}
		}

		Environment *parse(std::string);
	};
}