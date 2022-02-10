#pragma once
#include <vector>
#include <map>
#include <string>
#include "value.hpp"
#include "../process/environment.hpp"


namespace Runtime {
	class VM {
		std::shared_ptr<Environment> m_env;
		ByteCode *m_ip = { 0 };
		std::vector<std::shared_ptr<Value>> m_stack;
		std::shared_ptr<CallFrame> m_top_stack = { 0 };
		std::vector<std::shared_ptr<CallFrame>> m_call_stack;

	private:
		void def_native_procs();
		
		void add_call_frame(std::string, size_t, size_t);
		void kill_frame();
		void unwind_stack();

		void arithmetic_op();
		void comparison_op();
		void bind(bool);
	
	public:
		void error(bool, std::string);
		void push_stack(std::shared_ptr<Value>);
		std::shared_ptr<Value> pop_stack();
		std::shared_ptr<Value> peek_stack(size_t idx = 0);

		std::vector<std::string>& argv() { return m_env->argv; }
		size_t argc() { return m_env->argv.size(); }
		size_t stack_len() { return m_stack.size() - m_top_stack->stack_start; }
		size_t global_stack_len() { return m_stack.size(); }


	public:
		VM(std::shared_ptr<Environment> env) { m_env = env; }

		void run();
	};
}