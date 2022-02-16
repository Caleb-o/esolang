#include <iostream>
#include "analyser.hpp"
#include "util.hpp"

namespace Process {
	void Analyser::error(std::string err) {
		throw Util::string_format(
			"Analysis: %s with '%s' on line %d at pos %d",
			err.c_str(),
			get_token_name(m_current->kind),
			m_current->line,
			m_current->col
		);
	}
	
	Analyser::Analyser() {
		m_allowed_types[TypeFlag::INT] = TypeFlag::INT | TypeFlag::FLOAT;
		m_allowed_types[TypeFlag::FLOAT] = TypeFlag::INT | TypeFlag::FLOAT;
		m_allowed_types[TypeFlag::BOOL] = TypeFlag::BOOL;
		m_allowed_types[TypeFlag::STRING] = TypeFlag::STRING;
	}
	
	void Analyser::branch() {
		m_stack_start = m_type_stack.size() - 1;
	}

	void Analyser::merge() {
		if (m_type_stack.size() > m_stack_start) {
			error(Util::string_format(
				"%d unhandled item(s) on the stack",
				m_type_stack.size() - m_stack_start
			));
		}

		if (m_stack_start == 0) {
			std::cout << "Finished type checking\n";
		}

		m_stack_start = 0;
	}

	bool Analyser::is_allowed(TypeFlag flag_a, TypeFlag flag_b) {
		return m_allowed_types[flag_a] & flag_b != 0;
	}

	void Analyser::op(std::shared_ptr<Token> current) {
		m_current = current;
		TokenKind kind = current->kind;

		switch(kind) {
			case TokenKind::INT_LIT: 		m_type_stack.push_back(TypeFlag::INT); break;
			case TokenKind::FLOAT_LIT:		m_type_stack.push_back(TypeFlag::FLOAT); break;
			case TokenKind::BOOL_LIT:		m_type_stack.push_back(TypeFlag::BOOL); break;
			case TokenKind::STRING_LIT:		m_type_stack.push_back(TypeFlag::STRING); break;


			case TokenKind::PLUS: case TokenKind::MINUS:
			case TokenKind::STAR: case TokenKind::SLASH:
			case TokenKind::MOD: {
				if (m_type_stack.size() < m_stack_start + 2) {
					error("Not enough items on the stack");
				}
				
				TypeFlag flag_rhs = m_type_stack.back(); m_type_stack.pop_back();
				// We use the left-hand side type to tell if they're compatible,
				// we also keep it on the stack because the left type should be the result
				// type
				TypeFlag flag_lhs = m_type_stack.back();
				
				if (!is_allowed(flag_lhs, flag_rhs)) {
					error(Util::string_format(
						"Cannot operate on type '%s' with type '%s'",
						get_type_name(flag_lhs),
						get_type_name(flag_rhs)
					));
				}
				break;
			}

			case TokenKind::GREATER: case TokenKind::GREATER_EQ: 
			case TokenKind::LESS: case TokenKind::LESS_EQ: 
			case TokenKind::EQUAL: {
				if (m_type_stack.size() < m_stack_start + 2) {
					error("Not enough items on the stack");
				}

				TypeFlag flag_rhs = m_type_stack.back();
				TypeFlag flag_lhs = *(m_type_stack.end() - 2);
				
				if (!is_allowed(flag_lhs, flag_rhs)) {
					error(Util::string_format(
						"Cannot operate on type '%s' with type '%s'",
						get_type_name(flag_lhs),
						get_type_name(flag_rhs)
					));
				}
				break;
			}

			case TokenKind::SWAP: {
				if (m_type_stack.size() < m_stack_start + 1) {
					error("Not enough items on the stack");
				}
				TypeFlag flag_rhs = m_type_stack.back(); m_type_stack.pop_back();
				TypeFlag flag_lhs = m_type_stack.back(); m_type_stack.pop_back();

				m_type_stack.push_back(flag_rhs); m_type_stack.push_back(flag_lhs);
				break;
			}

			case TokenKind::DUP: {
				if (m_type_stack.size() < m_stack_start + 1) {
					error("Not enough items on the stack");
				}

				m_type_stack.push_back(m_type_stack.back());
				break;
			}

			case TokenKind::POP: {
				if (m_type_stack.size() < m_stack_start + 1) {
					error("Not enough items on the stack");
				}

				m_type_stack.pop_back();
				break;
			}
		}
	}

	void Analyser::push(TypeFlag flag) {
		m_type_stack.push_back(flag);
	}
}