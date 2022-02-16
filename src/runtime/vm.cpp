#include <iterator>
#include "vm.hpp"
#include "../process/util.hpp"
#include "../process/native.hpp"


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
			std::cout << "depth " << frame_idx << " '" <<  frame->proc_id << "' bindings [";

			size_t binding_idx = 0;
			for(auto& binding : frame->bindings) {
				std::cout << "'" << binding.first << "':";

				switch(binding.second->flag) {
					case BindFlag::PLAIN: std::cout << "'plain'"; break;
					case BindFlag::STRICT: std::cout << "'strict'"; break;
					case BindFlag::PARAM: std::cout << "'param'"; break;
				}

				if (binding_idx++ < frame->bindings.size() - 1) {
					std::cout << " ";
				}
			}
			std::cout << "] stack\n";

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
	size_t pos = (m_ip - m_env->code.data()-1 >= 0) ? m_ip - m_env->code.data()-1 : 0;

	if (internal) {
		std::cout << "Internal: ";
	} else {
		std::cout << "Runtime: ";
	}
	
	std::cout << msg << " at opcode pos " << pos << std::endl << std::endl;
	unwind_stack();

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

	auto op = *m_ip;

	switch(lhs->kind) {
		case ValueKind::INT: {
			switch(rhs->kind) {
				case ValueKind::INT: {
					switch(op) {
						case ByteCode::ADD:	push_stack(create_value((long long)(lhs->data.integer + rhs->data.integer))); break;
						case ByteCode::SUB:	push_stack(create_value((long long)(lhs->data.integer - rhs->data.integer))); break;
						case ByteCode::MUL:	push_stack(create_value((long long)(lhs->data.integer * rhs->data.integer))); break;
						case ByteCode::DIV:	push_stack(create_value((long long)(lhs->data.integer / rhs->data.integer))); break;
						case ByteCode::MOD:	push_stack(create_value((long long)(lhs->data.integer % rhs->data.integer))); break;

						default:	error(false, 
										Util::string_format("Unknown operation '%s'",
										get_bytecode_name(op)
									)); break;
					}
					break;
				}

				case ValueKind::FLOAT: {
					switch(op) {
						case ByteCode::ADD:	push_stack(create_value((float)(lhs->data.integer + rhs->data.floating))); break;
						case ByteCode::SUB:	push_stack(create_value((float)(lhs->data.integer - rhs->data.floating))); break;
						case ByteCode::MUL:	push_stack(create_value((float)(lhs->data.integer * rhs->data.floating))); break;
						case ByteCode::DIV:	push_stack(create_value((float)(lhs->data.integer / rhs->data.floating))); break;

						default:	error(false, 
										Util::string_format("Unknown operation '%s'",
										get_bytecode_name(op)
									)); break;
					}
					break;
				}

				default: {
					error(false, Util::string_format("Trying to operate on different value types. Lhs '%s', Rhs '%s'",
						kind_as_str(lhs->kind),
						kind_as_str(rhs->kind)
					));
					break;
				}
			}
			break;
		}

		case ValueKind::FLOAT: {
			switch(rhs->kind) {
				case ValueKind::FLOAT: {
					switch(op) {
						case ByteCode::ADD:	push_stack(create_value(lhs->data.floating + rhs->data.floating)); break;
						case ByteCode::SUB:	push_stack(create_value(lhs->data.floating - rhs->data.floating)); break;
						case ByteCode::MUL:	push_stack(create_value(lhs->data.floating * rhs->data.floating)); break;
						case ByteCode::DIV:	push_stack(create_value(lhs->data.floating / rhs->data.floating)); break;

						default:	error(false, 
										Util::string_format("Unknown operation '%s'",
										get_bytecode_name(op)
									)); break;
					}
					break;
				}

				case ValueKind::INT: {
					switch(op) {
						case ByteCode::ADD:	push_stack(create_value((float)(lhs->data.floating + rhs->data.integer))); break;
						case ByteCode::SUB:	push_stack(create_value((float)(lhs->data.floating - rhs->data.integer))); break;
						case ByteCode::MUL:	push_stack(create_value((float)(lhs->data.floating * rhs->data.integer))); break;
						case ByteCode::DIV:	push_stack(create_value((float)(lhs->data.floating / rhs->data.integer))); break;

						default:	error(false,
										Util::string_format("Unknown operation '%s'",
										get_bytecode_name(op)
									)); break;
					}
					break;
				}

				default: {
					error(false, Util::string_format("Trying to operate on different value types. Lhs '%s', Rhs '%s'",
						kind_as_str(lhs->kind),
						kind_as_str(rhs->kind)
					));
					break;
				}
			}
			break;
		}

		case ValueKind::STRING: {
			switch(op) {
				case ByteCode::ADD: {
					std::string str(lhs->string);
					str.append(rhs->string);

					push_stack(create_value(str));
					break;
				}

				default:	error(false, 
								Util::string_format("Unknown operation '%s'",
								get_bytecode_name(op)
							)); break;
			}
			break;
		}

		case ValueKind::BOOL:
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
				case ByteCode::OR:		push_stack(create_value(lhs->data.boolean || rhs->data.boolean)); break;
				case ByteCode::AND:		push_stack(create_value(lhs->data.boolean && rhs->data.boolean)); break;

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

void VM::bind(BindFlag bindType, bool unbind) {
	size_t bind_count = *(++m_ip);
	ByteCode *end = m_ip + bind_count;
	size_t bind_idx = 0;

	if (!unbind && bind_count > stack_len()) {
		error(false, Util::string_format(
			"Trying to bind %d value(s), but the stack contains %d value(s)",
			bind_count, stack_len()
		));
	}

	while(m_ip < end) {
		bind_idx = *(++m_ip);
		auto binding_it = m_top_stack->bindings.find(m_env->idLiterals[bind_idx]);
		bool at_end = binding_it == m_top_stack->bindings.end();
		
		if (unbind) {
			// Binding does not exist
			if (at_end) {
				error(false, Util::string_format(
					"Trying to unbind an unbound value '%s'",
					m_env->idLiterals[bind_idx].c_str()
				));
			}

			if (m_top_stack->bindings[m_env->idLiterals[bind_idx]]->flag == BindFlag::PARAM) {
				error(false, Util::string_format(
					"Trying to unbind a parameter '%s'",
					m_env->idLiterals[bind_idx].c_str()
				));
			}

			// Unbind the value
			m_top_stack->bindings.erase(m_env->idLiterals[bind_idx]);
		} else {
			// Check if binding exists
			if (at_end) {
				m_top_stack->bindings[m_env->idLiterals[bind_idx]] = std::make_shared<Binding>();
			} else {
				BindFlag flag = m_top_stack->bindings[m_env->idLiterals[bind_idx]]->flag;

				// Cannot override a parameter or strict
				if (flag == BindFlag::STRICT || flag == BindFlag::PARAM) {
					error(false, Util::string_format(
						"Trying to rebind a parameter/strict binding '%s'",
						m_env->idLiterals[bind_idx].c_str()
					));
				}
			}

			m_top_stack->bindings[m_env->idLiterals[bind_idx]]->flag = bindType;
			m_top_stack->bindings[m_env->idLiterals[bind_idx]]->value = pop_stack();
		}
	}
}

void VM::run() {
	// Find the main symbol
	size_t main_idx = get_proc_idx(m_env, "main");
	if (main_idx >= m_env->defs.procedures.size()) {
		error(true, "Could not find main symbol");
	}

	// Setup a callframe
	add_call_frame("main", -1, 0);
	m_ip = m_env->code.data() + m_env->defs.procedures[main_idx].second[0].startIdx;

	const ByteCode *code_len = m_env->code.data() + m_env->code.size();
	bool running = true;

	while(running && m_ip < code_len) {
		switch(*m_ip) {
			case ByteCode::PUSH: {
				push_stack(m_env->literals[*(++m_ip)]);
				break;
			}

			case ByteCode::ADD: case ByteCode::SUB:
			case ByteCode::MUL: case ByteCode::DIV:
			case ByteCode::MOD: {
				arithmetic_op();
				break;
			}

			case ByteCode::GREATER: case ByteCode::GREATER_EQ:
			case ByteCode::LESS: case ByteCode::LESS_EQ: 
			case ByteCode::EQUAL: case ByteCode::OR:
			case ByteCode::AND: {
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
				bind(BindFlag::PLAIN, false);
				break;
			}

			case ByteCode::BIND_STRICT: {
				bind(BindFlag::STRICT, false);
				break;
			}

			case ByteCode::BIND_PARAM: {
				bind(BindFlag::PARAM, false);
				break;
			}

			case ByteCode::UNBIND: {
				bind(BindFlag::PLAIN, true);
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

			case ByteCode::NOT: {
				auto value = pop_stack();

				if (value->kind != ValueKind::BOOL) {
					error(false, Util::string_format(
						"Cannot use a not on value kind '%s'",
						kind_as_str(value->kind)
					));
				}

				push_stack(create_value(!value->data.boolean));
				break;
			}

			case ByteCode::NATIVECALL: {
				auto native_it = &m_env->defs.native_procs[*(++m_ip)];
				// Call the native function
				(*native_it->second->func)(this);
				break;
			}

			case ByteCode::PROCCALL: {
				auto proc_it = std::next(m_env->defs.procedures.begin(), *(++m_ip));
				int sub_idx = *(++m_ip);

				std::shared_ptr<Value> capture_list = pop_stack();

				// Setup a callframe
				size_t return_idx = (m_ip - m_env->code.data());
				add_call_frame(proc_it->first, return_idx, capture_list->capture_len);

				m_ip = m_env->code.data() + proc_it->second[sub_idx].startIdx - 1;
				break;
			}

			case ByteCode::RETURN: {
				size_t sub_idx = *(++m_ip);
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