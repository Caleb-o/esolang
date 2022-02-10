#pragma once
#include <iostream>
#include <memory>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include "environment.hpp"
#include "../runtime/value.hpp"

using namespace Runtime;

namespace Runtime {
	class VM;
}

namespace Process {
	using Func = std::function<void(VM*)>;
	using FuncPtr = std::shared_ptr<Func>;
	using Native = std::shared_ptr<NativeDef>;

	static void native_str_len(VM *vm) {
		std::cout << "STRLEN" << std::endl;
	}

	template <class Functor>
	static std::shared_ptr<NativeDef> create_native(Functor f, std::vector<ValueKind> values) {
		return std::make_shared<NativeDef>((NativeDef){ FuncPtr(new Func(f)), values });
	}

	// TODO: Since the bound functions are static we can move them to their own header file
	static void def_native_procs(std::shared_ptr<Environment> env) {
		env->defs.native_procs["str_len"] = create_native(native_str_len, { ValueKind::STRING });
	}
}