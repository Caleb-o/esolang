#pragma once
#include <iostream>
#include <fstream>
#include <memory>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <exception>
#include "lexer.hpp"
#include "parser.hpp"
#include "environment.hpp"
#include "../runtime/value.hpp"
#include "../runtime/vm.hpp"


using namespace Runtime;


namespace Process {
	// TypeNames for easier typing
	using Func = std::function<void(VM*)>;
	using FuncPtr = std::shared_ptr<Func>;
	using Native = std::shared_ptr<NativeDef>;

	// --- Native Functions ---
	static void native_error(VM *vm) {
		vm->error(false, vm->pop_stack()->string);
	}

	static void native_assertm(VM *vm) {
		auto message = vm->pop_stack();
		auto condition = vm->pop_stack();

		if (condition->data.boolean) {
			vm->error(false, Util::string_format("Assert: %s", message->string.c_str()));
		}
	}

	static void native_assert(VM *vm) {
		auto condition = vm->pop_stack();

		if (condition->data.boolean) {
			vm->error(false, "Assertion has been triggered");
		}
	}

	static void native_file_exists(VM *vm) {
		auto file_name = vm->pop_stack();
		std::ifstream file(file_name->string.c_str());

		vm->push_stack(create_value(file.good()));
		file.close();
	}

	static void native_read_file(VM *vm) {
		auto file_name = vm->pop_stack();
		std::ifstream file(file_name->string.c_str());

		if (file.good()) {
			vm->push_stack(create_value(Util::read_file(file_name->string.c_str())));
			file.close();
		}
	}

	static void native_eval(VM *vm) {
		auto source = vm->pop_stack();

		try {
			if (source->string.size() > 0) {
				Parser p("INTERNAL");
				std::shared_ptr<Environment> env = p.parse(source->string, {});

				VM eval_vm(env);
				eval_vm.run();
			}
		}  catch (const char *msg) {
			std::cout << msg << "\n";
		} catch (std::string& msg) {
			std::cout << msg << "\n";
		} catch (std::exception& e) {
			std::cout << "Pre-process: " << e.what() << "\n";
		}
	}

	static void native_input(VM *vm) {
		std::cout << vm->pop_stack()->string;
		std::string in;

		std::getline(std::cin, in);
		vm->push_stack(create_value(in));
	}

	static void native_str_len(VM *vm) {
		auto str = vm->pop_stack();
		vm->push_stack(create_value((long long)str->string.size()));
	}

	static void native_str_cmp(VM *vm) {
		auto str_b = vm->pop_stack();
		auto str_a = vm->pop_stack();

		vm->push_stack(create_value(std::strcmp(str_a->string.c_str(), str_b->string.c_str()) == 0));
	}

	static void native_str_index(VM *vm) {
		auto idx = vm->pop_stack();
		auto str = vm->pop_stack();

		vm->push_stack(create_value(std::string(1, str->string[idx->data.integer])));
	}

	static void native_stoi(VM *vm) {
		auto str = vm->pop_stack();
		try {
			vm->push_stack(create_value(std::stoll(str->string)));
		} catch(std::exception&) {
			vm->push_stack(create_value(false));
		}
	}

	static void native_stof(VM *vm) {
		auto str = vm->pop_stack();
		try {
			vm->push_stack(create_value(std::stof(str->string)));
		} catch(std::exception&) {
			vm->push_stack(create_value(false));
		}
	}

	static void native_stob(VM *vm) {
		auto str = vm->pop_stack();
		try {
			vm->push_stack(create_value(str->string == "true" || !(str->string == "false")));
		} catch(std::exception&) {
			vm->push_stack(create_value(false));
		}
	}

	static void native_flip(VM *vm) {
		auto integer = vm->pop_stack()->data.integer;
		vm->push_stack(create_value((long long)integer * -1));
	}

	static void native_flipf(VM *vm) {
		auto floating = vm->pop_stack()->data.floating;
		vm->push_stack(create_value((float)(floating * -1.0)));
	}

	static void native_kind_cmp(VM *vm) {
		auto val_b = vm->pop_stack();
		auto val_a = vm->pop_stack();

		vm->push_stack(create_value(val_a->kind == val_b->kind));
	}

	static void native_peek(VM *vm) {
		auto n = vm->pop_stack()->data.integer;

		// Since we consider 0 the top, we have to minus one
		if (n >= vm->stack_len()) {
			vm->error(false, Util::string_format(
				"Trying to peek %d values down, but it only contains %d",
				n, vm->stack_len()
			));
		}

		vm->push_stack(vm->peek_stack(n));
	}

	static void native_drop_n(VM *vm) {
		auto n = vm->pop_stack()->data.integer;
		size_t i = 0;

		if (n-1 > vm->stack_len()) {
			vm->error(false, Util::string_format(
				"Trying to drop %d values from the stack, but it only contains %d",
				n, vm->stack_len()
			));
		}

		while(i++ < n) {
			vm->pop_stack();
		}
	}

	static void native_str_split(VM *vm) {
		auto delim = vm->pop_stack();
		auto buffer = vm->pop_stack();

		size_t pos = 0;
		std::vector<std::string> tokens;

		// Split the string by delimiters
		while ((pos = buffer->string.find(delim->string)) != std::string::npos) {
			tokens.push_back(buffer->string.substr(0, pos));
			buffer->string.erase(0, pos + delim->string.size());
		}
		tokens.push_back(buffer->string);

		// Add each string in reverse order
		auto current = tokens.end();
		while(current-- != tokens.begin()) {
			vm->push_stack(create_value(*current));
		}
	
		// Push the count of strings
		vm->push_stack(create_value((long long)tokens.size()));
	}

	static void native_to_bytes(VM *vm) {
		auto str = vm->pop_stack();

		for(auto ch : str->string) {
			vm->push_stack(create_value((long long)ch));
		}
	}

	static void native_from_bytes(VM *vm) {
		auto n = vm->pop_stack()->data.integer + 1;
		char *bytes = new char[n];
		bytes[n-1] = '\0';

		for(int i = n-2; i >= 0; --i) {
			bytes[i] = vm->pop_stack()->data.integer;
		}

		vm->push_stack(create_value(std::string(reinterpret_cast<char *>(bytes), n-1)));
		delete[] bytes;
	}

	static void native_drop_stack(VM *vm) {
		while(vm->stack_len() > 0) {
			vm->pop_stack();
		}
	}

	static void native_argv(VM *vm) {
		for (auto str : vm->argv()) {
			vm->push_stack(create_value(str));
		}
	}

	static void native_argc(VM *vm) {
		vm->push_stack(create_value((long long)vm->argc()));
	}

	static void native_stack_len(VM *vm) {
		vm->push_stack(create_value((long long)vm->stack_len()));
	}

	static void native_global_stack_len(VM *vm) {
		vm->push_stack(create_value((long long)vm->global_stack_len()));
	}

	// Helper function to create a def
	template <class Functor>
	static std::pair<std::string, std::shared_ptr<NativeDef>> create_native(std::string id, Functor f, std::vector<ValueKind> params, std::vector<ValueKind> returns) {
		return std::make_pair(id, std::make_shared<NativeDef>((NativeDef){ FuncPtr(new Func(f)), params, returns }));
	}

	// Define all native procedures
	static void def_native_procs(std::shared_ptr<Environment> env) {
		auto np = &env->defs.native_procs;

		np->push_back(create_native("error", native_error, 									{ ValueKind::STRING }, {}));
		np->push_back(create_native("native_assertm", native_assertm, 						{ ValueKind::STRING, ValueKind::BOOL }, {}));
		np->push_back(create_native("native_assert", native_assert,							{ ValueKind::BOOL }, {}));
		np->push_back(create_native("file_exists", native_file_exists,						{ ValueKind::STRING }, { ValueKind::BOOL }));
		np->push_back(create_native("read_file", native_read_file,							{ ValueKind::STRING }, { ValueKind::STRING }));
		np->push_back(create_native("eval", native_eval,									{ ValueKind::STRING }, {}));
		np->push_back(create_native("input", native_input,									{ ValueKind::STRING }, { ValueKind::STRING }));
		np->push_back(create_native("stoi", native_stoi,									{ ValueKind::STRING }, { ValueKind::INT }));
		np->push_back(create_native("stof", native_stof, 									{ ValueKind::STRING }, { ValueKind::FLOAT }));
		np->push_back(create_native("stob", native_stob,									{ ValueKind::STRING }, { ValueKind::BOOL }));
		np->push_back(create_native("flip", native_flip,									{ ValueKind::INT }, {}));
		np->push_back(create_native("flipf", native_flipf,									{ ValueKind::FLOAT }, {}));
		np->push_back(create_native("str_len", native_str_len,								{ ValueKind::STRING }, { ValueKind::INT }));
		np->push_back(create_native("str_cmp", native_str_cmp,								{ ValueKind::STRING, ValueKind::STRING }, { ValueKind::BOOL }));
		np->push_back(create_native("split", native_str_split,								{ ValueKind::STRING, ValueKind::STRING }, { ValueKind::STRING }));
		np->push_back(create_native("to_bytes", native_to_bytes,							{ ValueKind::STRING }, { ValueKind::INT }));
		np->push_back(create_native("from_bytes", native_from_bytes,						{ ValueKind::INT }, { ValueKind::STRING }));
		np->push_back(create_native("str_index", native_str_index,							{ ValueKind::INT, ValueKind::STRING }, { ValueKind::STRING }));
		np->push_back(create_native("kind_cmp", native_kind_cmp,							{ }, { ValueKind::BOOL }));
		np->push_back(create_native("peek", native_peek,									{ ValueKind::INT }, { }));
		np->push_back(create_native("drop_n", native_drop_n,								{ ValueKind::INT }, { }));
		np->push_back(create_native("argv", native_argv,									{ }, { ValueKind::STRING }));
		np->push_back(create_native("argc", native_argc,									{ }, { ValueKind::INT }));
		np->push_back(create_native("drop_stack", native_drop_stack,						{ }, {}));
		np->push_back(create_native("stack_len", native_stack_len,							{ }, { ValueKind::INT }));
		np->push_back(create_native("global_stack_len", native_global_stack_len, 			{ }, { ValueKind::INT }));
	}
}