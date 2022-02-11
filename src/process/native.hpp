#pragma once
#include <iostream>
#include <memory>
#include <functional>
#include <map>
#include <string>
#include <vector>
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

	static void native_assert(VM *vm) {
		auto message = vm->pop_stack();
		auto condition = vm->pop_stack();

		if (condition->data.boolean) {
			vm->error(false, Util::string_format("Assert: %s", message->string.c_str()));
		}
	}

	static void native_read_file(VM *vm) {
		auto file_name = vm->pop_stack();
		vm->push_stack(create_value(Util::read_file(file_name->string.c_str())));
	}

	static void native_input(VM *vm) {
		std::cout << vm->pop_stack()->string;
		std::string in;

		std::getline(std::cin, in);
		vm->push_stack(create_value(in));
	}

	static void native_str_len(VM *vm) {
		auto str = vm->peek_stack();
		vm->push_stack(create_value((long long)str->string.size()));
	}

	static void native_str_cmp(VM *vm) {
		auto str_b = vm->peek_stack(0);
		auto str_a = vm->peek_stack(1);

		vm->push_stack(create_value(std::strcmp(str_a->string.c_str(), str_b->string.c_str()) == 0));
	}

	static void native_str_index(VM *vm) {
		auto idx = vm->pop_stack();
		auto str = vm->peek_stack();

		vm->push_stack(create_value(std::string(1, str->string[idx->data.integer])));
	}

	static void native_stoi(VM *vm) {
		auto str = vm->peek_stack();
		vm->push_stack(create_value(std::stoll(str->string)));
	}

	static void native_stof(VM *vm) {
		auto str = vm->peek_stack();
		vm->push_stack(create_value(std::stof(str->string)));
	}

	static void native_stob(VM *vm) {
		auto str = vm->peek_stack();
		vm->push_stack(create_value(str->string == "true" || !(str->string == "false")));
	}

	static void native_kind_cmp(VM *vm) {
		auto val_b = vm->peek_stack(0);
		auto val_a = vm->peek_stack(1);

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
	static std::shared_ptr<NativeDef> create_native(Functor f, std::vector<ValueKind> values) {
		return std::make_shared<NativeDef>((NativeDef){ FuncPtr(new Func(f)), values });
	}

	// Define all native procedures
	static void def_native_procs(std::shared_ptr<Environment> env) {
		env->defs.native_procs["error"] 		= create_native(native_error, 			{ ValueKind::STRING });
		env->defs.native_procs["assert"] 		= create_native(native_assert, 			{ ValueKind::STRING, ValueKind::BOOL });
		env->defs.native_procs["read_file"]		= create_native(native_read_file,		{ ValueKind::STRING });
		env->defs.native_procs["input"]			= create_native(native_input,			{ ValueKind::STRING });
		env->defs.native_procs["stoi"]			= create_native(native_stoi,			{ ValueKind::STRING });
		env->defs.native_procs["stof"]			= create_native(native_stof,			{ ValueKind::STRING });
		env->defs.native_procs["stob"]			= create_native(native_stob,			{ ValueKind::STRING });
		env->defs.native_procs["str_len"] 		= create_native(native_str_len, 		{ ValueKind::STRING });
		env->defs.native_procs["str_cmp"] 		= create_native(native_str_cmp, 		{ ValueKind::STRING, ValueKind::STRING });
		env->defs.native_procs["str_split"] 	= create_native(native_str_split, 		{ ValueKind::STRING, ValueKind::STRING });
		env->defs.native_procs["to_bytes"] 		= create_native(native_to_bytes, 		{ ValueKind::STRING });
		env->defs.native_procs["from_bytes"]	= create_native(native_from_bytes, 		{ ValueKind::INT });
		env->defs.native_procs["str_index"] 	= create_native(native_str_index,		{ ValueKind::INT, ValueKind::STRING });
		env->defs.native_procs["kind_cmp"]		= create_native(native_kind_cmp,		{ });
		env->defs.native_procs["peek"] 			= create_native(native_peek, 			{ ValueKind::INT });
		env->defs.native_procs["drop_n"] 		= create_native(native_drop_n, 			{ ValueKind::INT });
		env->defs.native_procs["argv"] 			= create_native(native_argv, 			{ });
		env->defs.native_procs["argc"] 			= create_native(native_argc, 			{ });
		env->defs.native_procs["drop_stack"] 	= create_native(native_drop_stack, 		{ });
		env->defs.native_procs["stack_len"] 	= create_native(native_stack_len, 		{ });
		env->defs.native_procs["global_stack_len"] 	= create_native(native_global_stack_len, 		{ });
	}
}