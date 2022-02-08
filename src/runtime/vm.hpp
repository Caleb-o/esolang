#pragma once
#include <vector>
#include <map>
#include <string>
#include "../process/environment.hpp"
#include "value.hpp"


namespace Runtime {
	struct CallFrame {
		std::string proc_id;
		size_t return_idx;
		std::map<std::string, Value *> bindings;
		std::vector<Value *> stack;

		~CallFrame() {
			bindings.clear();
			stack.clear();
		}
	};

	class VM {
		Environment *m_env;
		size_t m_ip = { 0 };
		CallFrame *m_top_stack = { 0 };
		std::vector<CallFrame *> m_call_stack;

	private:
		void add_call_frame(std::string, size_t);
		void kill_frame();
		void unwind_stack();
		void error(bool, std::string);
		void push_stack(Value *);
		void arithmetic_op();
		void comparison_op();
		Value *pop_stack();
		Value *peek_stack(size_t idx = 0);

	public:
		VM(Environment *env) { m_env = env; }
		~VM() {
			if (m_env) delete m_env;
		}

		void run();
	};
}