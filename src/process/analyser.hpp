#pragma once
#include <vector>
#include <map>
#include <string>
#include <memory>
#include "token.hpp"
#include "environment.hpp"
#include "../runtime/value.hpp"


namespace Process {
	enum TypeFlag {
		INT = 0x01,
		FLOAT = 0x02,
		BOOL = 0x04,
		STRING = 0x08,
		CAPTURED_VALUES = 0x16,
	};

	class Analyser {
	private:
		size_t m_capture_count = { 0 };
		std::vector<size_t> m_stack;
		std::vector<TypeFlag> m_type_stack;
		std::map<TypeFlag, int> m_allowed_types;
		std::map<std::string, TypeFlag> m_type_bindings;
		std::map<std::string, std::vector<TypeFlag>> m_proc_rets;
		std::shared_ptr<Token> m_current;

	public:
		void error(std::string);
		void unwind();

		Analyser();


		void branch(); // Used when we enter a new procedure
		void merge(); // Removes the last branch
		size_t stack_size() { return m_type_stack.size() - m_stack.back(); }
		bool is_allowed(TypeFlag, TypeFlag);

		void op(std::shared_ptr<Token>);
		void capture(size_t);
		size_t eval_return(std::shared_ptr<Environment>, size_t, std::vector<Runtime::ValueKind>&);
		size_t capture_size() { return m_capture_count; }
		void add_proc_ret(std::string id, TypeFlag flag) { m_proc_rets[id].push_back(flag); }
		std::vector<TypeFlag>& get_proc_ret(std::string id) { return m_proc_rets[id]; }
		void bind(std::string);
		void bind_param(std::string, TypeFlag);
		void unbind(std::string);
		void load_binding(std::string);

		void push(TypeFlag);
		TypeFlag peek(size_t idx) { return *(m_type_stack.end() - idx - 1); }
		void pop();
	};


	static Runtime::ValueKind flag_to_valuekind(TypeFlag flag) {
		switch (flag) {
			case TypeFlag::INT:					return Runtime::ValueKind::INT;
			case TypeFlag::FLOAT:				return Runtime::ValueKind::FLOAT;
			case TypeFlag::BOOL:				return Runtime::ValueKind::BOOL;
			case TypeFlag::STRING:				return Runtime::ValueKind::STRING;
			case TypeFlag::CAPTURED_VALUES:		return Runtime::ValueKind::CAPTURE;
		}
	}

	static TypeFlag valuekind_to_flag(Runtime::ValueKind kind) {
		switch (kind) {
			case Runtime::ValueKind::INT:	 		return TypeFlag::INT;
			case Runtime::ValueKind::FLOAT:	 		return TypeFlag::FLOAT;
			case Runtime::ValueKind::BOOL:	 		return TypeFlag::BOOL;
			case Runtime::ValueKind::STRING:	 	return TypeFlag::STRING;
			case Runtime::ValueKind::CAPTURE:	 	return TypeFlag::CAPTURED_VALUES;
		}
	}
	
	static const char *get_type_name(TypeFlag flag) {
		switch (flag) {
			case TypeFlag::INT:					return "int";
			case TypeFlag::FLOAT:				return "float";
			case TypeFlag::BOOL:				return "bool";
			case TypeFlag::STRING:				return "string";
			case TypeFlag::CAPTURED_VALUES:		return "capture";
		}
	}
}