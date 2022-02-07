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

	private:
		void push_byte(ByteCode);
		void push_bytes(ByteCode, ByteCode);

		void consume(TokenKind);

		size_t add_literal_to_env(Value value);
		ByteCode add_literal();

		void expr();
		void arithmetic_statement();
		void statement();
		void statement_list();
		void code_block();
		void using_statement();
		void type_list();
		void parameter_list();
		void struct_def();
		void procedure_def();
		void program();

	public:
		Parser();
		// We will need to delete the last token
		~Parser() { delete m_current; }

		Environment *parse(char *);
	};
}