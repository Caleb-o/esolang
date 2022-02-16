#pragma once
#include <vector>
#include <map>
#include <string>
#include <memory>
#include "token.hpp"

namespace Process {
	enum TypeFlag {
		INT = 0x01,
		FLOAT = 0x02,
		BOOL = 0x04,
		STRING = 0x08,
	};

	class Analyser {
	private:
		size_t m_stack_start = { 0 };
		std::vector<TypeFlag> m_type_stack;
		std::map<TypeFlag, int> m_allowed_types;
		std::map<std::string, TypeFlag> m_type_bindings;
		std::shared_ptr<Token> m_current;

	private:
		void error(std::string);
	
	public:
		Analyser();

		void branch(); // Used when we enter a new procedure
		void merge(); // Removes the last branch
		size_t stack_size() { return m_type_stack.size() - m_stack_start; }
		bool is_allowed(TypeFlag, TypeFlag);

		void op(std::shared_ptr<Token>);
		void bind(std::string);
		void unbind(std::string);

		void push(TypeFlag);
		void pop();
	};

	static const char *get_type_name(TypeFlag flag) {
		switch (flag) {
			case TypeFlag::INT:			return "int";
			case TypeFlag::FLOAT:		return "float";
			case TypeFlag::BOOL:		return "bool";
			case TypeFlag::STRING:		return "string";
		}
	}
}