#pragma once
#include <iostream>
#include <map>
#include <vector>
#include "util.hpp"
#include "../runtime/value.hpp"
#include "bytecode.hpp"


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
		std::vector<std::shared_ptr<Value>> literals;
		// IDs - these are used for bindings
		std::vector<std::string> idLiterals;
		// Definitions are compile-time known structs and procedures
		Definitions defs;

		~Environment() {
			literals.clear();
			idLiterals.clear();
		}
	};


	static size_t get_proc_idx(std::shared_ptr<Environment> env, const char *proc_id) {
		return std::distance(env->defs.procedures.begin(), env->defs.procedures.find(proc_id));
	}


	static void print_env(std::shared_ptr<Environment> env) {
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
				case ByteCode::PUSH: {
					int val_idx = (int)env->code[++i];
					std::cout << get_bytecode_name(env->code[i-1]) << "<" << val_idx << ", '";
					write_value(env->literals[val_idx]);
					std::cout << "'>\n";
					break;
				}

				case ByteCode::BIND: {
					int bindings = (int)env->code[++i];
					int count = i + bindings;

					std::cout << get_bytecode_name(env->code[i-1]) << "<" << bindings;

					if (bindings > 0) {
						std::cout << ": ";
					}
					while(i < count) {
						std::cout << env->code[i++];

						if (i < count - 1) {
							std::cout << " ";
						}
					}
					std::cout << ">\n";
					break;
				}

				case ByteCode::RETURN:
				case ByteCode::PROCCALL:
				case ByteCode::CAPTURE:
				case ByteCode::GOTO:
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

		std::cout << std::endl;
	}
}