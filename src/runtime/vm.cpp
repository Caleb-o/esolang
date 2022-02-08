#include "vm.hpp"
#include "../process/util.hpp"


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
				std::cout << " : " << kind_as_str(frame->stack[stack_idx].kind);
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
	}
}

void VM::error(bool internal, std::string msg) {
	std::cout << msg << " at code " << get_bytecode_name(m_env->code[m_ip]) << " at pos " << m_ip << std::endl;
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


void VM::arithmetic_op() {
	auto rhs = pop_stack();
	auto lhs = pop_stack();

	// Type check left and right side kinds
	if (lhs.kind != rhs.kind) {
		error(false, Util::string_format("Trying to operate on different value types. Lhs '%s', Rhs '%s'",
			kind_as_str(lhs.kind), kind_as_str(rhs.kind)
		));
	}

	auto op = m_env->code[m_ip];

	switch(lhs.kind) {
		case ValueKind::INT: {
			switch(op) {
				case ByteCode::ADD:	push_stack(create_value(lhs.data.integer + rhs.data.integer)); break;
				case ByteCode::SUB:	push_stack(create_value(lhs.data.integer - rhs.data.integer)); break;
				case ByteCode::MUL:	push_stack(create_value(rhs.data.integer * lhs.data.integer)); break;
				case ByteCode::DIV:	push_stack(create_value(rhs.data.integer / lhs.data.integer)); break;

				default:	error(false, 
								Util::string_format("Unknown operation '%s'",
								get_bytecode_name(op)
							)); break;
			}
			break;
		}

		case ValueKind::FLOAT: {
			switch(op) {
				case ByteCode::ADD:	push_stack(create_value(lhs.data.floating + rhs.data.floating)); break;
				case ByteCode::SUB:	push_stack(create_value(lhs.data.floating - rhs.data.floating)); break;
				case ByteCode::MUL:	push_stack(create_value(rhs.data.floating * lhs.data.floating)); break;
				case ByteCode::DIV:	push_stack(create_value(rhs.data.floating / lhs.data.floating)); break;

				default:	error(false, 
								Util::string_format("Unknown operation '%s'",
								get_bytecode_name(op)
							)); break;
			}
			break;
		}

		case ValueKind::BOOL:
		case ValueKind::STRING:
		case ValueKind::STRUCT: {
			error(false, 
				Util::string_format("Cannot use arithmetic operations on type '%s'",
				kind_as_str(lhs.kind)
			));
			break;
		}

		default: break;
	}
}

void VM::comparison_op() {
		Value rhs = pop_stack();
		Value lhs = pop_stack();

		// Type check left and right side kinds
		if (lhs.kind != rhs.kind) {
			error(false,
				Util::string_format("Trying to operate on different value types. Lhs '%s', Rhs '%s'",
				kind_as_str(lhs.kind), kind_as_str(rhs.kind)
			));
		}

		auto op = m_env->code[m_ip];

		switch(lhs.kind) {
			case ValueKind::INT: {
				switch(op) {
					case ByteCode::GREATER:		push_stack(create_value(lhs.data.integer > rhs.data.integer)); break;
					case ByteCode::GREATER_EQ:	push_stack(create_value(lhs.data.integer >= rhs.data.integer)); break;
					case ByteCode::LESS:		push_stack(create_value(lhs.data.integer < rhs.data.integer)); break;
					case ByteCode::LESS_EQ:		push_stack(create_value(lhs.data.integer <= rhs.data.integer)); break;
					case ByteCode::EQUAL:		push_stack(create_value(lhs.data.integer == rhs.data.integer)); break;

					default:	error(false,
									Util::string_format("Unknown operation '%s'",
									get_bytecode_name(op)
								)); break;
				}
				break;
			}

			case ValueKind::FLOAT: {
				switch(op) {
					case ByteCode::GREATER:		push_stack(create_value(lhs.data.floating > rhs.data.floating)); break;
					case ByteCode::GREATER_EQ:	push_stack(create_value(lhs.data.floating >- rhs.data.floating)); break;
					case ByteCode::LESS:		push_stack(create_value(lhs.data.floating < rhs.data.floating)); break;
					case ByteCode::LESS_EQ:		push_stack(create_value(lhs.data.floating <= rhs.data.floating)); break;
					case ByteCode::EQUAL:		push_stack(create_value(lhs.data.floating == rhs.data.floating)); break;

					default:	error(false,
									Util::string_format("Unknown operation '%s'",
									get_bytecode_name(op)
								)); break;
				}
				break;
			}

			case ValueKind::BOOL: {
				switch(op) {
					case ByteCode::EQUAL:	push_stack(create_value(lhs.data.boolean == rhs.data.boolean)); break;

					default:	error(false,
									Util::string_format("Unknown operation '%s'",
									get_bytecode_name(op)
								)); break;
				}
				break;
			}

			case ValueKind::STRING: {
				switch(op) {
					case ByteCode::GREATER:		push_stack(create_value(std::strlen(lhs.data.string) > 	std::strlen(rhs.data.string))); break;
					case ByteCode::GREATER_EQ:	push_stack(create_value(std::strlen(lhs.data.string) >= std::strlen(rhs.data.string))); break;
					case ByteCode::LESS:		push_stack(create_value(std::strlen(lhs.data.string) < 	std::strlen(rhs.data.string))); break;
					case ByteCode::LESS_EQ:		push_stack(create_value(std::strlen(lhs.data.string) <= std::strlen(rhs.data.string))); break;
					case ByteCode::EQUAL:		push_stack(create_value(std::strlen(lhs.data.string) == std::strlen(rhs.data.string))); break;

					default:	error(false,
									Util::string_format("Unknown operation '%s'",
									get_bytecode_name(op)
								)); break;
				}
				break;
			}

			case ValueKind::STRUCT: {
				error(false,
					Util::string_format("Cannot use arithmetic operations on type '%s'",
					kind_as_str(lhs.kind)
				));
				break;
			}

			default: break;
		}
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

			case ByteCode::ADD: case ByteCode::SUB:
			case ByteCode::MUL: case ByteCode::DIV: {
				arithmetic_op();
				break;
			}

			case ByteCode::GREATER: case ByteCode::GREATER_EQ:
			case ByteCode::LESS: case ByteCode::LESS_EQ: 
			case ByteCode::EQUAL: {
				comparison_op();
				break;
			}

			case ByteCode::IF: {
				size_t false_idx = m_env->code[++m_ip];

				if (m_top_stack->stack.size() == 0 || peek_stack().kind != ValueKind::BOOL) {
					error(false, "Cannot evaluate an empty stack or non-boolean value");
				}

				Value condition = pop_stack();

				// Jump if false
				if (!condition.data.boolean) {
					m_ip = false_idx;
				}
				break;
			}

			case ByteCode::GOTO: {
				size_t jump_idx = m_env->code[++m_ip];
				m_ip = jump_idx;
				break;
			}

			case ByteCode::DROP: 		pop_stack(); break;
			case ByteCode::DUPLICATE:	push_stack(peek_stack()); break;

			case ByteCode::HALT: {
				if (m_top_stack->stack.size() > 0) {
					error(false, "Program exiting with non-empty stack");
				}

				running = false; 
				break;
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

			default: {
				error(false,
					Util::string_format("Unknown opcode %d at pos %d", m_env->code[m_ip], m_ip)
				);
			}
		}

		m_ip++;
	}
}