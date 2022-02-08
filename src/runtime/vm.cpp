#include <iterator>
#include "vm.hpp"
#include "../process/util.hpp"


void VM::add_call_frame(std::string proc_name, size_t ret_idx) {
	CallFrame *frame = new CallFrame;
	frame->proc_id = proc_name;
	frame->return_idx = ret_idx;

	m_call_stack.push_back(frame);
	m_top_stack = frame;
}

void VM::kill_frame() {
	if (m_top_stack) {
		for(int i = m_top_stack->stack.size() - 1; i >= 0; --i) {
			if (m_top_stack->stack[i]) delete m_top_stack->stack[i];
		}

		delete m_top_stack;

		size_t new_size = m_call_stack.size();
		m_top_stack = (new_size > 0) ? m_call_stack[new_size-1] : nullptr;
		std::cout << "Killed stack\n";
	}
}


void VM::unwind_stack() {
	if (m_call_stack.size() > 0) {
		std::cout << "== Call Stack ==\n";
		
		for(int frame_idx = m_call_stack.size()-1; frame_idx >= 0; --frame_idx) {
			CallFrame *frame = m_call_stack[frame_idx];
			std::cout << "depth " << frame_idx << " '" <<  frame->proc_id << "' [";
			// TODO: Print each binding here
			std::cout << "] stack\n";

			if (frame->stack.size() == 0) {
				std::cout << "-- Empty --\n";
				continue;
			}

			for(int stack_idx = frame->stack.size()-1; stack_idx >= 0; --stack_idx) {
				std::cout << "[" << stack_idx << "] ";
				write_value(frame->stack[stack_idx]);
				std::cout << " : " << kind_as_str(frame->stack[stack_idx]->kind);
				std::cout << std::endl;

				delete frame->stack[stack_idx];
			}

			kill_frame();
			std::cout << std::endl;
		}
	}
}

void VM::error(bool internal, std::string msg) {
	std::cout << msg << " at code " << get_bytecode_name(m_env->code[m_ip]) << " at pos " << m_ip << std::endl;
	if (!internal) unwind_stack();
	throw "Runtime exception occured";
}

void VM::push_stack(Value *value) {
	m_top_stack->stack.push_back(value);
}

Value *VM::pop_stack() {
	if (m_top_stack->stack.size() == 0) {
		error(false, "Trying to pop an empty stack");
	}

	Value *tmp = m_top_stack->stack.back();
	m_top_stack->stack.pop_back();
	return tmp;
}

Value *VM::peek_stack(size_t idx) {
	return m_top_stack->stack[m_top_stack->stack.size() - (idx + 1)];
}


void VM::arithmetic_op() {
	Value *rhs = pop_stack();
	Value *lhs = pop_stack();

	// Type check left and right side kinds
	if (lhs->kind != rhs->kind) {
		error(false, Util::string_format("Trying to operate on different value types. Lhs '%s', Rhs '%s'",
			kind_as_str(lhs->kind), kind_as_str(rhs->kind)
		));
	}

	auto op = m_env->code[m_ip];

	switch(lhs->kind) {
		case ValueKind::INT: {
			switch(op) {
				case ByteCode::ADD:	push_stack(create_value(lhs->data.integer + rhs->data.integer)); break;
				case ByteCode::SUB:	push_stack(create_value(lhs->data.integer - rhs->data.integer)); break;
				case ByteCode::MUL:	push_stack(create_value(rhs->data.integer * lhs->data.integer)); break;
				case ByteCode::DIV:	push_stack(create_value(rhs->data.integer / lhs->data.integer)); break;

				default:	error(false, 
								Util::string_format("Unknown operation '%s'",
								get_bytecode_name(op)
							)); break;
			}
			break;
		}

		case ValueKind::FLOAT: {
			switch(op) {
				case ByteCode::ADD:	push_stack(create_value(lhs->data.floating + rhs->data.floating)); break;
				case ByteCode::SUB:	push_stack(create_value(lhs->data.floating - rhs->data.floating)); break;
				case ByteCode::MUL:	push_stack(create_value(rhs->data.floating * lhs->data.floating)); break;
				case ByteCode::DIV:	push_stack(create_value(rhs->data.floating / lhs->data.floating)); break;

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
				kind_as_str(lhs->kind)
			));
			break;
		}

		default: break;
	}
}

void VM::comparison_op() {
		Value *rhs = pop_stack();
		Value *lhs = pop_stack();

		// Type check left and right side kinds
		if (lhs->kind != rhs->kind) {
			error(false,
				Util::string_format("Trying to operate on different value types. Lhs '%s', Rhs '%s'",
				kind_as_str(lhs->kind), kind_as_str(rhs->kind)
			));
		}

		auto op = m_env->code[m_ip];

		switch(lhs->kind) {
			case ValueKind::INT: {
				switch(op) {
					case ByteCode::GREATER:		push_stack(create_value(lhs->data.integer > rhs->data.integer)); break;
					case ByteCode::GREATER_EQ:	push_stack(create_value(lhs->data.integer >= rhs->data.integer)); break;
					case ByteCode::LESS:		push_stack(create_value(lhs->data.integer < rhs->data.integer)); break;
					case ByteCode::LESS_EQ:		push_stack(create_value(lhs->data.integer <= rhs->data.integer)); break;
					case ByteCode::EQUAL:		push_stack(create_value(lhs->data.integer == rhs->data.integer)); break;

					default:	error(false,
									Util::string_format("Unknown operation '%s'",
									get_bytecode_name(op)
								)); break;
				}
				break;
			}

			case ValueKind::FLOAT: {
				switch(op) {
					case ByteCode::GREATER:		push_stack(create_value(lhs->data.floating > rhs->data.floating)); break;
					case ByteCode::GREATER_EQ:	push_stack(create_value(lhs->data.floating >- rhs->data.floating)); break;
					case ByteCode::LESS:		push_stack(create_value(lhs->data.floating < rhs->data.floating)); break;
					case ByteCode::LESS_EQ:		push_stack(create_value(lhs->data.floating <= rhs->data.floating)); break;
					case ByteCode::EQUAL:		push_stack(create_value(lhs->data.floating == rhs->data.floating)); break;

					default:	error(false,
									Util::string_format("Unknown operation '%s'",
									get_bytecode_name(op)
								)); break;
				}
				break;
			}

			case ValueKind::CAPTURE: {
				switch(op) {
					case ByteCode::GREATER:		push_stack(create_value(lhs->capture_len >  rhs->capture_len)); break;
					case ByteCode::GREATER_EQ:	push_stack(create_value(lhs->capture_len >- rhs->capture_len)); break;
					case ByteCode::LESS:		push_stack(create_value(lhs->capture_len <  rhs->capture_len)); break;
					case ByteCode::LESS_EQ:		push_stack(create_value(lhs->capture_len <= rhs->capture_len)); break;
					
					// TODO: Check for equality in length and types
					// case ByteCode::EQUAL:	push_stack(create_value(lhs->capture_len == rhs->capture_len)); break;

					default:	error(false,
									Util::string_format("Unknown operation '%s'",
									get_bytecode_name(op)
								)); break;
				}
				break;
			}

			case ValueKind::BOOL: {
				switch(op) {
					case ByteCode::EQUAL:	push_stack(create_value(lhs->data.boolean == rhs->data.boolean)); break;

					default:	error(false,
									Util::string_format("Unknown operation '%s'",
									get_bytecode_name(op)
								)); break;
				}
				break;
			}

			case ValueKind::STRING: {
				switch(op) {
					case ByteCode::GREATER:		push_stack(create_value(std::strlen(lhs->data.string) >  std::strlen(rhs->data.string))); break;
					case ByteCode::GREATER_EQ:	push_stack(create_value(std::strlen(lhs->data.string) >= std::strlen(rhs->data.string))); break;
					case ByteCode::LESS:		push_stack(create_value(std::strlen(lhs->data.string) <  std::strlen(rhs->data.string))); break;
					case ByteCode::LESS_EQ:		push_stack(create_value(std::strlen(lhs->data.string) <= std::strlen(rhs->data.string))); break;
					case ByteCode::EQUAL:		push_stack(create_value(std::strlen(lhs->data.string) == std::strlen(rhs->data.string))); break;

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
					kind_as_str(lhs->kind)
				));
				break;
			}

			default: break;
		}

		delete rhs;
		delete lhs;
	}


void VM::run() {
	// Find the main symbol
	size_t main_idx = get_proc_idx(m_env, "main");
	if (main_idx < 0) {
		error(true, "Could not find main symbol");
	}

	// Setup a callframe
	add_call_frame("main", -1);
	m_ip = m_env->defs.procedures["main"][0].startIdx;

	const size_t code_len = m_env->code.size();
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

				if (m_top_stack->stack.size() == 0 || peek_stack()->kind != ValueKind::BOOL) {
					error(false, "Cannot evaluate an empty stack or non-boolean value");
				}

				Value *condition = pop_stack();

				// Jump if false
				if (!condition->data.boolean) {
					m_ip = false_idx;
				}
				delete condition;
				break;
			}

			case ByteCode::GOTO: {
				size_t jump_idx = m_env->code[++m_ip];
				m_ip = jump_idx;
				break;
			}

			case ByteCode::BIND: {
				size_t bind_count = m_env->code[++m_ip];
				size_t i = 0, bind_idx = 0;

				while(i++ < bind_count) {
					bind_idx = m_env->code[++m_ip];
					m_top_stack->bindings[m_env->idLiterals[bind_idx]] = pop_stack();
				}
				break;
			}

			case ByteCode::CAPTURE: {
				size_t capture_count = m_env->code[++m_ip];
				Value ** values = new Value *[capture_count];

				for(int i = capture_count - 1; i >= 0; --i) {
					values[i] = pop_stack();
				}

				push_stack(create_value(values, capture_count));
				break;
			}

			case ByteCode::PROCCALL: {
				auto proc_it = std::next(m_env->defs.procedures.begin(), m_env->code[++m_ip]);
				int sub_idx = 0;

				// TODO: Make it so void procs can go without a capture list
				// Must be a capture list
				if (m_top_stack->stack.size() == 0 || peek_stack()->kind != ValueKind::CAPTURE) {
					error(false, "Procedure call requires a capture list on the stack top");
				}

				Value *capture_list = pop_stack();

				if (proc_it->second.size() > 1) {
					// We must linearly check each overload + each parameter
					// It is possible to immediately skip
					for(auto it = proc_it->second.begin(); it != proc_it->second.end(); ++it) {
						size_t param_idx = 0;
						bool found = true;

						// Skip if arguments aren't correct count
						if (capture_list->capture_len != it->parameters.size()) {
							sub_idx++;
							continue;
						}

						// Check all parameters
						for(int param_idx = it->parameters.size() - 1; param_idx >= 0; --param_idx) {
							// Types don't equal then it's correct
							auto param = std::next(it->parameters.begin(), param_idx);

							if (param->second.kind != capture_list->capture[param_idx]->kind) {
								found = false;
								continue;
							}
						}

						// Correct type found
						if (found) break;
						sub_idx++;
					}
				}

				// Failed to find a valid procedure that matches stack items
				if (sub_idx >= proc_it->second.size()) {
					delete capture_list;
					error(false, "Could not find a procedure that matches stack values");
				}

				// Setup a callframe
				size_t return_idx = m_ip;
				add_call_frame(proc_it->first, return_idx);

				// TODO pass into callee stack (unpack, since we bind)
				for(size_t i = 0; i < capture_list->capture_len; ++i) {
					m_top_stack->stack.push_back(capture_list->capture[i]);
				}

				delete capture_list;
				m_ip = proc_it->second[sub_idx].startIdx - 1;
				break;
			}

			case ByteCode::RETURN: {
				size_t sub_idx = m_env->code[++m_ip];
				ProcedureDef *proc_def = &m_env->defs.procedures[m_top_stack->proc_id][sub_idx];
				size_t last_frame = m_call_stack.size() - 2;

				// Handle return data
				if (proc_def->returnTypes[0] != ValueKind::VOID) {
					for(int ret_idx = proc_def->returnTypes.size(); ret_idx >= 0; --ret_idx) {
						if (peek_stack()->kind != proc_def->returnTypes[ret_idx]) {
							error(false, Util::string_format(
								"Expected type '%s' but got '%s'",
								kind_as_str(proc_def->returnTypes[ret_idx]),
								kind_as_str(peek_stack()->kind)
							));
						}

						// Add to previous call-frame
						m_call_stack[last_frame]->stack.push_back(pop_stack());
					}
				}

				// Too many values on the stack
				if (m_top_stack->stack.size() > 0) {
					error(false, Util::string_format(
						"Trying to return with more than %d items on the stack",
						m_top_stack->stack.size()
					));
				}

				m_ip = m_top_stack->return_idx;
				kill_frame();
				break;
			}

			case ByteCode::DROP: 		delete pop_stack(); break;
			case ByteCode::DUPLICATE:	push_stack(peek_stack()); break;

			case ByteCode::HALT: {
				if (m_call_stack[0]->stack.size() > 0) {
					error(false, "Program exiting with non-empty stack");
				}

				kill_frame();

				running = false; 
				break;
			}

			case ByteCode::SWAP: {
				Value *rhs = pop_stack();
				Value *lhs = pop_stack();

				push_stack(rhs);
				push_stack(lhs);
				break;
			}

			case ByteCode::PRINT: {
				Value *val = peek_stack();
				write_value(val);
				break;
			}

			case ByteCode::PRINTLN: {
				Value *val = peek_stack();
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