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
		std::cout << "=== Procedures ===\n";
		size_t idx = 0;
		for(auto& proc : env->defs.procedures) {
			std::cout << "Proc '" << proc.first << "' #" << idx << "\n";

			size_t overload = 0;
			for(auto& procDef : proc.second) {
				std::cout << "overload #" << overload << ", starts at pos: " << procDef.startIdx << std::endl;
				std::cout << "== Params == " << std::endl;

				if (procDef.parameters.size() == 0) {
					std::cout << "-- Empty --\n";
				}
				
				for(auto& param : procDef.parameters) {
					std::cout << param.first << " : ";
					std::cout << ((param.second.isMoved) ? "move" : "copy") << " ";
					std::cout << kind_as_str(param.second.kind) << std::endl;
				}

				std::cout << "== Return == " << std::endl;
				
				for(auto& ret : procDef.returnTypes) {
					std::cout << kind_as_str(ret) << std::endl;
				}

				std::cout << std::endl;
				overload++;
			}
			idx++;
		}

		std::cout << "=== ByteCode ===\n";
		size_t i = 0;

		while(i < env->code.size()) {
			std::cout << Util::string_format("%04d ", i);

			switch(env->code[i]) {
				case ByteCode::RETURN: {
					int proc_idx = (int)env->code[++i];
					int sub_idx = (int)env->code[++i];
					std::cout << get_bytecode_name(env->code[i-2]) << "<" << proc_idx << ", " << sub_idx << ">\n";
					break;
				}

				case ByteCode::PROCCALL:
				case ByteCode::GOTO:
				case ByteCode::PUSH:
				case ByteCode::IF: {
					int val_idx = (int)env->code[++i];
					std::cout << get_bytecode_name(env->code[i-1]) << "<" << val_idx << ">\n";
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