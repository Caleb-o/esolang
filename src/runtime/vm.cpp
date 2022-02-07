#include "vm.hpp"


static void add_call_frame(std::vector<CallFrame>& stack, std::string proc_name, size_t ret_idx) {
	CallFrame frame;
	frame.proc_id = proc_name;
	frame.return_idx = ret_idx;

	stack.push_back(frame);
}


void VM::unwind_stack() {
	if (m_call_stack.size() > 0) {
		std::cout << "== Call Stack ==\n";
		
		for(int frame_idx = m_call_stack.size()-1; frame_idx >= 0; frame_idx--) {
			CallFrame *frame = &m_call_stack[frame_idx];
			std::cout << "depth " << frame_idx << " '" <<  frame->proc_id << "' [";
			// TODO: Print each binding here
			std::cout << "] stack\n";

			if (frame->stack.size() == 0) {
				std::cout << "-- Empty --\n";
				continue;
			}

			for(int stack_idx = frame->stack.size()-1; stack_idx >= 0; stack_idx--) {
				std::cout << "[" << stack_idx << "] ";
				write_value(frame->stack[stack_idx]);
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
	}
}

void VM::error(bool internal, std::string msg) {
	std::cout << msg << std::endl;
	if (!internal) unwind_stack();
	throw "Runtime exception occured";
}

void VM::push_stack(Value value) {
	m_top_stack->stack.push_back(value);
}

Value VM::pop_stack() {
	if (m_top_stack->stack.size() == 0) {
		error(false, "Trying to pop an empty stack");
	}

	Value tmp = m_top_stack->stack.back();
	m_top_stack->stack.pop_back();
	return tmp;
}

Value VM::peek_stack() {
	return m_top_stack->stack.back();
}

void VM::run() {
	// Find the main symbol
	size_t main_idx = get_proc_idx(m_env, "main");
	if (main_idx < 0) {
		error(true, "Could not find main symbol");
	}

	// Setup a callframe
	add_call_frame(m_call_stack, "main", -1);
	m_ip = m_env->defs.procedures["main"][0].startIdx;

	const size_t code_len = m_env->code.size();
	m_top_stack = &m_call_stack[m_call_stack.size()-1];
	bool running = true;

	while(running && m_ip < code_len) {
		switch(m_env->code[m_ip]) {
			case ByteCode::PUSH: {
				push_stack(m_env->literals[m_env->code[++m_ip]]);
				break;
			}

			case ByteCode::DROP: 		pop_stack(); break;
			case ByteCode::DUPLICATE:	push_stack(peek_stack()); break;
			case ByteCode::HALT: {
				if (m_top_stack->stack.size() > 0) {
					error(false, "Program exiting with non-empty stack");
				}

				running = false; break;
			}

			case ByteCode::SWAP: {
				Value rhs = pop_stack();
				Value lhs = pop_stack();

				push_stack(rhs);
				push_stack(lhs);
				break;
			}

			case ByteCode::PRINT: {
				Value val = peek_stack();
				write_value(val);
				break;
			}

			case ByteCode::PRINTLN: {
				Value val = peek_stack();
				write_value(val);
				std::cout << std::endl;
				break;
			}
		}

		m_ip++;
	}
}