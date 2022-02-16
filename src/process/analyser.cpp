#include "analyser.hpp"

namespace Process {
	void Analyser::error(std::string err) {
		throw err;
	}
	
	Analyser::Analyser() {
		m_allowed_types[TypeFlag::INT] = TypeFlag::INT | TypeFlag::FLOAT;
		m_allowed_types[TypeFlag::FLOAT] = TypeFlag::INT | TypeFlag::FLOAT;
		m_allowed_types[TypeFlag::BOOL] = TypeFlag::BOOL;
		m_allowed_types[TypeFlag::STRING] = TypeFlag::STRING;
	}
	
	void Analyser::branch() {
		m_type_stack.push_back({});
	}

	void Analyser::merge() {
		m_type_stack.pop_back();
	}

	bool Analyser::is_allowed(TypeFlag flag_a, TypeFlag flag_b) {
		return m_allowed_types[flag_a] & flag_b != 0;
	}

	void Analyser::op(TokenKind kind) {
		
	}

	void Analyser::push(TypeFlag flag) {
		m_type_stack.back().push_back(flag);
	}
}