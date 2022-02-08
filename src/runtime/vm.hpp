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
		std::map<std::string, Value> bindings;
		std::vector<Value> stack;
	};

	class VM {
		Environment *m_env;
		size_t m_ip = { 0 };
		CallFrame *m_top_stack = { 0 };
		std::vector<CallFrame> m_call_stack;

	private:
		void unwind_stack();
		void error(bool, std::string);
		void push_stack(Value);
		void arithmetic_op();
		void comparison_op();
		Value pop_stack();
		Value peek_stack(size_t idx = 0);

	public:
		VM(Environment *env) { m_env = env; }
		~VM() {
			if (m_env) {
				for(Value& val : m_env->literals) {
					if (val.kind == ValueKind::STRING)
						if (val.data.string)
							delete[] val.data.string;
					
					if (val.kind == ValueKind::CAPTURE)
						if (val.capture)
							delete[] val.capture;
				}

				delete m_env;
			}
		}

		void run();
	};
}