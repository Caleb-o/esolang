#pragma once
#include <iostream>
#include <map>
#include <vector>
#include "../runtime/value.hpp"
#include "bytecode.hpp"
#include "util.hpp"


using namespace Runtime;

namespace Process {
	struct StructDef {
		std::map<std::string, ValueKind> fields;
	};

	struct Parameter {
		ValueKind kind;
		bool isMoved;
	};

	struct ProcedureDef {
		size_t startIdx;
		std::map<std::string, Parameter> parameters;
		std::vector<ValueKind> returnTypes;
	};

	struct Definitions {
		std::map<std::string, std::vector<ProcedureDef>> procedures;
		// std::map<std::string, StructDef> structs;
	};

	struct Environment {
		// TODO: Maybe consider an array here
		std::vector<ByteCode> code;
		// Should be unique, if a value already exists, get its index
		std::vector<Value> literals;
		// IDs
		std::vector<std::string> idLiterals;
		// Definitions are compile-time known structs and procedures
		Definitions defs;
	};


	static void print_code(Environment *env) {
		std::cout << "=== ByteCode ===\n";

		if (env == nullptr) {
			std::cout << "KEK\n";
			return;
		}

		size_t i = 0;
		while(i < env->code.size()) {
			std::cout << Util::string_format("%04d ", i);

			switch(env->code[i]) {
				case ByteCode::PROCCALL:
				case ByteCode::GOTO:
				case ByteCode::RETURN:
				case ByteCode::PUSH:
				case ByteCode::IF: {
					int val_idx = (int)env->code[i+1];
					std::cout << get_bytecode_name(env->code[i]) << "<" << val_idx << ">\n";

					i++;
					break;
				}

				default: {
					std::cout << get_bytecode_name(env->code[i]) << std::endl;
					break;
				}
			}

			i++;
		}
	}
}