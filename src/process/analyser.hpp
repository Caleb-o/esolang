#pragma once
#include <vector>
#include <map>
#include <string>
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
		std::vector<std::vector<TypeFlag>> m_type_stack;
		std::map<TypeFlag, int> m_allowed_types;

	private:
		void error(std::string);
	
	public:
		Analyser();

		void branch(); // Used when we enter a new procedure
		void merge(); // Removes the last branch
		bool is_allowed(TypeFlag, TypeFlag);

		void op(TokenKind);
		void push(TypeFlag);
	};
}