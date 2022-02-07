#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include "../process/util.hpp"


namespace Runtime {
	enum class ValueKind {
		VOID, INT, FLOAT, BOOL, STRING, STRUCT
	};

	union ValueData {
		int integer;
		float floating;
		bool boolean;
		char *string;
	};
	
	struct Value {
		ValueKind kind;
		ValueData data;
		bool read_only;
	};

	
	static void write_value(Value& value) {
		switch(value.kind) {
			case ValueKind::VOID: 		std::cout << "void"; break;
			case ValueKind::INT: 		std::cout << value.data.integer; break;
			case ValueKind::FLOAT: 		std::cout << value.data.floating; break;
			case ValueKind::BOOL: 		std::cout << value.data.boolean; break;
			case ValueKind::STRING: 	std::cout << value.data.string; break;
			case ValueKind::STRUCT: 	std::cout << "struct"; break;
		}
	}


	static ValueKind kind_from_str(const char *name) {
		switch(Util::hash(name, std::strlen(name))) {
			case Util::hash("void", 4):			return ValueKind::VOID;
			case Util::hash("int", 3): 			return ValueKind::INT;
			case Util::hash("float", 5): 		return ValueKind::FLOAT;
			case Util::hash("bool", 4): 		return ValueKind::BOOL;
			case Util::hash("string", 6): 		return ValueKind::STRING;
			case Util::hash("struct", 6): 		return ValueKind::STRUCT;
		}
	}

	static const char *kind_as_str(ValueKind kind) {
		switch(kind) {
			case ValueKind::VOID:		return "void";
			case ValueKind::INT:		return "int";
			case ValueKind::FLOAT:		return "float";
			case ValueKind::BOOL:		return "bool";
			case ValueKind::STRING:		return "string";
			case ValueKind::STRUCT:		return "struct";
		}
	}
}