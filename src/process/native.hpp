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
	static void native_str_len(VM *vm) {
		auto str = vm->peek_stack();
		vm->push_stack(create_value((long long)str->string.size()));
	}

	static void native_str_cmp(VM *vm) {
		auto str_b = vm->peek_stack(0);
		auto str_a = vm->peek_stack(1);

		vm->push_stack(create_value(std::strcmp(str_a->string.c_str(), str_b->string.c_str()) == 0));
	}

	static void native_drop_stack(VM *vm) {
		while(vm->stack_len() > 0) {
			vm->pop_stack();
		}
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
		env->defs.native_procs["str_len"] 		= create_native(native_str_len, 		{ ValueKind::STRING });
		env->defs.native_procs["str_cmp"] 		= create_native(native_str_cmp, 		{ ValueKind::STRING, ValueKind::STRING });
		env->defs.native_procs["drop_stack"] 	= create_native(native_drop_stack, 		{ });
		env->defs.native_procs["stack_len"] 	= create_native(native_stack_len, 		{ });
		env->defs.native_procs["global_stack_len"] 	= create_native(native_global_stack_len, 		{ });
	}
}