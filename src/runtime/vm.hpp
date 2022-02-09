#pragma once
#include <vector>
#include <map>
#include <string>
#include "../process/environment.hpp"
#include "value.hpp"


namespace Runtime {
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

	class VM {
		std::shared_ptr<Environment> m_env;
		ByteCode *m_ip = { 0 };
		std::vector<std::shared_ptr<Value>> m_stack;
		std::shared_ptr<CallFrame> m_top_stack = { 0 };
		std::vector<std::shared_ptr<CallFrame>> m_call_stack;

	private:
		void add_call_frame(std::string, size_t, size_t);
		void kill_frame();
		void unwind_stack();
		void error(bool, std::string);
		void push_stack(std::shared_ptr<Value>);
		void arithmetic_op();
		void comparison_op();
		void bind(bool);
		std::shared_ptr<Value> pop_stack();
		std::shared_ptr<Value> peek_stack(size_t idx = 0);

	public:
		VM(std::shared_ptr<Environment> env) { m_env = env; }

		void run();
	};
}