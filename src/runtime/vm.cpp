#include <iterator>
#include "vm.hpp"
#include "../process/util.hpp"


void VM::add_call_frame(std::string proc_name, size_t ret_idx, size_t stack_start) {
	std::shared_ptr<CallFrame> frame = std::make_shared<CallFrame>();
	frame->proc_id = proc_name;
	frame->return_idx = ret_idx;
	frame->stack_start = m_stack.size() - stack_start;

	m_call_stack.push_back(frame);
	m_top_stack = frame;
}

void VM::kill_frame() {
	m_call_stack.pop_back();
	size_t new_size = m_call_stack.size();
	m_top_stack = (new_size > 0) ? m_call_stack[new_size-1] : nullptr;
}


void VM::unwind_stack() {
	if (m_call_stack.size() > 0) {
		std::cout << "== Call Stack ==\n";
		
		for(int frame_idx = m_call_stack.size()-1; frame_idx >= 0; --frame_idx) {
			std::shared_ptr<CallFrame> frame = m_call_stack[frame_idx];
			std::cout << "depth " << frame_idx << " '" <<  frame->proc_id << "' [";
			// TODO: Print each binding here
			std::cout << "] stack | start: " << frame->stack_start << ":\n";

			if (m_stack.size() <= frame->stack_start) {
				std::cout << "-- Empty --\n";
			} else {
				// Print all stack items
				for(int stack_idx = m_stack.size(); stack_idx > frame->stack_start; --stack_idx) {
					int idx = stack_idx-1;

					std::cout << "[" << idx << "] ";
					write_value(m_stack[idx]);
					std::cout << " : " << kind_as_str(m_stack[idx]->kind);
					std::cout << std::endl;

					m_stack.pop_back();
				}
			}

			kill_frame();
			std::cout << std::endl;
		}
	}
}

void VM::error(bool internal, std::string msg) {
	if (internal) {
		std::cout << "Internal: " << msg << std::endl << std::endl;
	} else {
		std::cout << msg << " at code " << get_bytecode_name(*m_ip) << " at pos " << (m_ip - m_env->code.data()) << std::endl << std::endl;
		unwind_stack();
	}
	throw "Runtime exception occured";
}

void VM::push_stack(std::shared_ptr<Value> value) {
	m_stack.push_back(value);
}

std::shared_ptr<Value> VM::pop_stack() {
	if (m_stack.size() <= 0 || m_stack.size() < m_top_stack->stack_start) {
		error(false, "Trying to pop an empty stack");
	}

	std::shared_ptr<Value> tmp = m_stack[m_stack.size()-1];
	m_stack.pop_back();
	return tmp;
}

std::shared_ptr<Value> VM::peek_stack(size_t idx) {
	if (m_stack.size() <= 0 || m_stack.size() < m_top_stack->stack_start) {
		error(false, "Trying to peek an empty stack");
	}

	return m_stack[m_stack.size() - (idx + 1)];
}


void VM::arithmetic_op() {
	std::shared_ptr<Value> rhs = pop_stack();
	std::shared_ptr<Value> lhs = pop_stack();

	// Type check left and right side kinds
	if (lhs->kind != rhs->kind) {
		error(false, Util::string_format("Trying to operate on different value types. Lhs '%s', Rhs '%s'",
			kind_as_str(lhs->kind), kind_as_str(rhs->kind)
		));
	}

	auto op = *m_ip;

	switch(lhs->kind) {
		case ValueKind::INT: {
			switch(op) {
				case ByteCode::ADD:	push_stack(create_value((long long)(lhs->data.integer + rhs->data.integer))); break;
				case ByteCode::SUB:	push_stack(create_value((long long)(lhs->data.integer - rhs->data.integer))); break;
				case ByteCode::MUL:	push_stack(create_value((long long)(rhs->data.integer * lhs->data.integer))); break;
				case ByteCode::DIV:	push_stack(create_value((long long)(rhs->data.integer / lhs->data.integer))); break;

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
		std::shared_ptr<Value> rhs = pop_stack();
		std::shared_ptr<Value> lhs = pop_stack();

		// Type check left and right side kinds
		if (lhs->kind != rhs->kind) {
			error(false,
				Util::string_format("Trying to operate on different value types. Lhs '%s', Rhs '%s'",
				kind_as_str(lhs->kind), kind_as_str(rhs->kind)
			));
		}

		auto op = *m_ip;

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
					case ByteCode::GREATER:		push_stack(create_value(lhs->string.size() >  rhs->string.size())); break;
					case ByteCode::GREATER_EQ:	push_stack(create_value(lhs->string.size() >= rhs->string.size())); break;
					case ByteCode::LESS:		push_stack(create_value(lhs->string.size() <  rhs->string.size())); break;
					case ByteCode::LESS_EQ:		push_stack(create_value(lhs->string.size() <= rhs->string.size())); break;
					case ByteCode::EQUAL:		push_stack(create_value(lhs->string.size() == rhs->string.size())); break;

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
	}


void VM::run() {
	// Find the main symbol
	size_t main_idx = get_proc_idx(m_env, "main");
	if (m_env->defs.procedures.find("main") == m_env->defs.procedures.end()) {
		error(true, "Could not find main symbol");
	}

	// Setup a callframe
	add_call_frame("main", -1, 0);
	m_ip = m_env->code.data() + m_env->defs.procedures["main"][0].startIdx;

	const ByteCode *code_len = m_env->code.data() + m_env->code.size();
	bool running = true;

	while(running && m_ip < code_len) {
		switch(*m_ip) {
			case ByteCode::PUSH: {
				push_stack(m_env->literals[*(++m_ip)]);
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
			
			case ByteCode::IF:
			case ByteCode::LOOP: {
				size_t false_idx = *(++m_ip);

				if (m_stack.size() < m_top_stack->stack_start || peek_stack()->kind != ValueKind::BOOL) {
					error(false, "Cannot evaluate an empty stack or non-boolean value");
				}

				std::shared_ptr<Value> condition = pop_stack();

				// Jump if false
				if (!condition->data.boolean) {
					m_ip = m_env->code.data() + false_idx;
				}
				break;
			}

			case ByteCode::GOTO: {
				size_t jump_idx = *(++m_ip);
				m_ip = m_env->code.data() + jump_idx;
				break;
			}

			case ByteCode::BIND: {
				size_t bind_count = *(++m_ip);
				ByteCode *end = m_ip + bind_count;
				size_t bind_idx = 0;

				if (bind_count > m_stack.size() - m_top_stack->stack_start) {
					error(false, Util::string_format(
						"Trying to bind %d value(s), but the stack contains %d value(s)",
						bind_count, m_stack.size() - m_top_stack->stack_start
					));
				}

				while(m_ip < end) {
					bind_idx = *(++m_ip);
					auto binding_it = m_top_stack->bindings.find(m_env->idLiterals[bind_idx]);
					
					// Check if binding exists
					if (binding_it == m_top_stack->bindings.end()) {
						m_top_stack->bindings[m_env->idLiterals[bind_idx]] = std::make_shared<Binding>();
					} else {
						if (m_top_stack->bindings[m_env->idLiterals[bind_idx]]->strict) {
							error(false, Util::string_format(
								"Trying to rebind a parameter (strict binding) '%s'",
								m_env->idLiterals[bind_idx].c_str()
							));
						}
					}

					// Whether we can unbind or not
					m_top_stack->bindings[m_env->idLiterals[bind_idx]]->strict = false;
					m_top_stack->bindings[m_env->idLiterals[bind_idx]]->value = pop_stack();
				}
				break;
			}

			case ByteCode::BIND_MOVE: {
				size_t bind_count = *(++m_ip);
				ByteCode *end = m_ip + bind_count;
				size_t bind_idx = 0;

				while(m_ip < end) {
					bind_idx = *(++m_ip);
					auto binding_it = m_top_stack->bindings.find(m_env->idLiterals[bind_idx]);
					
					// Check if binding exists
					if (binding_it == m_top_stack->bindings.end()) {
						m_top_stack->bindings[m_env->idLiterals[bind_idx]] = std::make_shared<Binding>();
					}

					// Whether we can unbind or not
					m_top_stack->bindings[m_env->idLiterals[bind_idx]]->strict = true;
					m_top_stack->bindings[m_env->idLiterals[bind_idx]]->value = pop_stack();
				}
				break;
			}

			case ByteCode::CAPTURE: {
				size_t capture_count = *(++m_ip);
				push_stack(create_value((int)capture_count));
				break;
			}

			case ByteCode::LOAD_BINDING: {
				size_t binding_idx = *(++m_ip);
				std::string binding = m_env->idLiterals[binding_idx];

				// Does not exist
				if (m_top_stack->bindings.find(binding) == m_top_stack->bindings.end()) {
					error(false,
						Util::string_format("Trying to access unbound binding '%s'",
						binding.c_str()
					));
				}

				push_stack(m_top_stack->bindings[binding]->value);
				break;
			}

			case ByteCode::PROCCALL: {
				auto proc_it = std::next(m_env->defs.procedures.begin(), *(++m_ip));
				int sub_idx = 0;

				// TODO: Make it so void procs can go without a capture list
				// Must be a capture list
				if (m_stack.size() < m_top_stack->stack_start || peek_stack()->kind != ValueKind::CAPTURE) {
					error(false, "Procedure call requires a capture list on the stack top");
				}

				std::shared_ptr<Value> capture_list = pop_stack();

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
					for(int param_idx = 0; param_idx < it->parameters.size(); ++param_idx) {
						// Types don't equal then it's correct
						auto param = std::next(it->parameters.begin(), param_idx);

						if (param->second != peek_stack(param_idx)->kind) {
							found = false;
							continue;
						}
					}

					// Correct type found
					if (found) break;
					sub_idx++;
				}

				// Failed to find a valid procedure that matches stack items
				if (sub_idx >= proc_it->second.size()) {
					error(false, "Could not find a procedure that matches stack values");
				}

				// Setup a callframe
				size_t return_idx = (m_ip - m_env->code.data());
				add_call_frame(proc_it->first, return_idx, capture_list->capture_len);

				m_ip = m_env->code.data() + proc_it->second[sub_idx].startIdx - 1;
				break;
			}

			case ByteCode::RETURN: {
				size_t sub_idx = *(++m_ip);

				ProcedureDef *proc_def = &m_env->defs.procedures[m_top_stack->proc_id][sub_idx];
				size_t last_frame = m_call_stack.size() - 2;
				size_t stack_idx = 0;

				
				// Handle return data
				if (proc_def->returnTypes[0] != ValueKind::VOID) {
					// Too few items on the stack
					if (m_stack.size() < m_top_stack->stack_start + proc_def->returnTypes.size()) {
						error(false, Util::string_format(
							"Trying to return with %d value(s) on the stack, but expected %d",
							m_stack.size() - m_top_stack->stack_start,
							proc_def->returnTypes.size()
						));
					}

					for(int ret_idx = proc_def->returnTypes.size() - 1; ret_idx >= 0; --ret_idx) {
						if (peek_stack(stack_idx)->kind != proc_def->returnTypes[ret_idx]) {
							error(false, Util::string_format(
								"Expected type '%s' but got '%s'",
								kind_as_str(proc_def->returnTypes[ret_idx]),
								kind_as_str(peek_stack(stack_idx)->kind)
							));
						}
						stack_idx++;
					}
				}

				// Too many values on the stack
				if (m_stack.size() - stack_idx > m_top_stack->stack_start) {
					error(false, Util::string_format(
						"Trying to return with %d value(s) on the stack, but expected %d",
						m_stack.size() - m_top_stack->stack_start,
						proc_def->returnTypes.size()
					));
				}

				m_ip = m_env->code.data() + m_top_stack->return_idx;
				kill_frame();
				break;
			}

			case ByteCode::DROP: 		pop_stack(); break;
			case ByteCode::DUPLICATE:	push_stack(peek_stack()); break;
			case ByteCode::ROTATE: {
				auto c = peek_stack(0);
				auto a = peek_stack(2);

				m_stack[m_stack.size()-1] = a;
				m_stack[m_stack.size()-3] = c;
				break;
			}

			case ByteCode::HALT: {
				if (m_stack.size() > 0) {
					error(false, "Program exiting with non-empty stack");
				}

				kill_frame();

				running = false; 
				break;
			}

			case ByteCode::SWAP: {
				std::shared_ptr<Value> rhs = pop_stack();
				std::shared_ptr<Value> lhs = pop_stack();

				push_stack(rhs);
				push_stack(lhs);
				break;
			}

			case ByteCode::PRINT: {
				std::shared_ptr<Value> val = peek_stack();
				write_value(val);
				break;
			}

			case ByteCode::PRINTLN: {
				std::shared_ptr<Value> val = peek_stack();
				write_value(val);
				std::cout << std::endl;
				break;
			}

			default: {
				error(false,
					Util::string_format("Unknown opcode %d", *m_ip)
				);
			}
		}

		m_ip++;
	}
}