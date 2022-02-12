#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <functional>
#include "util.hpp"
#include "bytecode.hpp"
#include "../runtime/value.hpp"


using namespace Runtime;

namespace Runtime {
	class VM;
}

namespace Process {
	struct Binding {
		bool strict; // Whether we can unbind or not
		std::shared_ptr<Value> value;	
	};

	struct CallFrame {
		std::string proc_id;
		size_t return_idx;
		std::map<std::string, std::shared_ptr<Binding>> bindings;
		size_t stack_start;

		~CallFrame() {
			bindings.clear();
		}
	};

	struct StructDef {
		std::map<std::string, ValueKind> fields;
	};

	struct ProcedureDef {
		size_t startIdx;
		std::map<std::string, ValueKind> parameters;
		std::vector<ValueKind> returnTypes;
	};

	struct NativeDef {
		std::shared_ptr<std::function<void(VM *)>> func;
		std::vector<ValueKind> parameters;
	};

	struct Definitions {
		std::map<std::string, std::vector<ProcedureDef>> procedures;
		std::map<std::string, std::shared_ptr<NativeDef>> native_procs;
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
		std::vector<std::string> argv;
		size_t top_level_len = { 0 };

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
				std::cout << "variation #" << overload << ", starts at opcode pos: " << procDef.startIdx << std::endl;
				std::cout << "== Params == " << std::endl;

				if (procDef.parameters.size() == 0) {
					std::cout << "-- Empty --\n";
				}
				
				for(auto& param : procDef.parameters) {
					std::cout << param.first << " : move ";
					std::cout << kind_as_str(param.second) << std::endl;
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

		std::cout << "=== ID Literals ===\n";
		idx = 0;

		if (env->idLiterals.size() == 0) {
			std::cout << "-- Empty --\n";
		}
		for(auto& id : env->idLiterals) {
			std::cout << "ID '" << id << "' #" << idx++ << std::endl;
		}
		std::cout << std::endl;

		std::cout << "=== Literals ===\n";
		idx = 0;

		if (env->literals.size() == 0) {
			std::cout << "-- Empty --\n";
		}
		for(auto& literal : env->literals) {
			std::cout << "value '"; write_value(literal); std::cout << "' :: '" << kind_as_str(literal->kind) << "' #" << idx++ << std::endl;
		}
		std::cout << std::endl;

		std::cout << "=== ByteCode ===\n";
		size_t i = 0, ops = 0;

		while(i < env->code.size()) {
			std::cout << Util::string_format("%04d  %3d :: ", i, env->code[i]);

			switch(env->code[i]) {
				case ByteCode::PUSH: {
					int val_idx = (int)env->code[++i];
					std::cout << get_bytecode_name(env->code[i-1]) << "<" << val_idx << ", '";
					write_value(env->literals[val_idx]);
					std::cout << "'>\n";
					break;
				}

				case ByteCode::BIND:
				case ByteCode::BIND_STRICT: {
					int bindings = (int)env->code[++i];
					int count = i + bindings;

					std::cout << get_bytecode_name(env->code[i-1]) << "<" << bindings;

					if (bindings > 0) {
						std::cout << ": ";
					}
					while(i < count) {
						std::cout << "'" << env->idLiterals[env->code[++i]] << "'";

						if ((i-1) < count - 1) {
							std::cout << " ";
						}
					}
					std::cout << ">\n";
					break;
				}

				case ByteCode::NATIVECALL: {
					int native_idx = (int)env->code[++i];
					auto native_it = std::next(env->defs.native_procs.begin(), native_idx);
					std::cout << get_bytecode_name(env->code[i-1]) << "<" << native_idx << ", '" << native_it->first << "'>\n";
					break;
				}

				case ByteCode::PROCCALL: {
					int proc_idx = (int)env->code[++i];
					auto proc_it = std::next(env->defs.procedures.begin(), proc_idx);
					std::cout << get_bytecode_name(env->code[i-1]) << "<" << proc_idx << ", '" << proc_it->first << "'>\n";
					break;
				}

				case ByteCode::LOAD_BINDING: {
					int bind_idx = (int)env->code[++i];
					std::cout << get_bytecode_name(env->code[i-1]) << "<" << bind_idx << ", '" << env->idLiterals[bind_idx] << "'>\n";
					break;
				}

				case ByteCode::RETURN:
				case ByteCode::CAPTURE:
				case ByteCode::GOTO:
				case ByteCode::IF:
				case ByteCode::LOOP: {
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
			ops++;
		}

		std::cout << "-- " << ops << " ops --\n";
		std::cout << std::endl;
	}
}