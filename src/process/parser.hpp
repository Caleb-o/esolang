#pragma once
#include <string>
#include <map>
#include <unordered_set>
#include <memory>
#include "bytecode.hpp"
#include "environment.hpp"
#include "lexer.hpp"


namespace Process {
	class Parser {
		// Hash each file we include, so we can check it when importing, mitigates including the same file twice
		std::unordered_set<size_t> m_file_hashes;
		// This is a pointer, so we can later swap it out when importing another module
		std::vector<ByteCode> m_top_level;
		bool m_is_top_level = { false };
		std::shared_ptr<Lexer> m_lexer;
		std::vector<std::shared_ptr<Lexer>> m_lexers;
		std::shared_ptr<Token> m_current;
		std::shared_ptr<Environment> m_env;
		std::string m_base_dir;
		bool m_completed = { false };
		bool m_ignore_main = { false };

	private:
		void error(std::string);
		void warning(std::string);
		
		void push_byte(ByteCode);
		void push_byte(size_t);
		void push_bytes(ByteCode, ByteCode);
		void push_bytes(ByteCode, size_t);

		void consume(TokenKind);

		size_t add_literal_to_env(std::shared_ptr<Value> value);
		size_t add_literal();
		void capture_list();

		void expr();
		void arithmetic_statement();
		void comparison_statement();
		void if_statement();
		void loop_statement();
		void proc_call_statement();
		void native_call_statement();
		void binding_access_statement();
		void bind_statement(BindFlag, bool);
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
		Parser(std::string);

		std::shared_ptr<Environment> parse(std::string, std::vector<std::string>);
	};
}