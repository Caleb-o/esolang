#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include "../process/util.hpp"


namespace Runtime {
	enum class ValueKind {
		VOID, INT, FLOAT, BOOL, STRING, STRUCT, CAPTURE,
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

		size_t capture_len;
		Value **capture;
		

		~Value() {
			if (kind == ValueKind::STRING) delete[] data.string;
			if (kind == ValueKind::CAPTURE) delete[] capture;
		}
	};

	static Value *create_value(int value, bool read_only = true) {
		return new (Value){ 
			ValueKind::INT,
			{ .integer=value },
			read_only
		};
	}

	static Value *create_value(float value, bool read_only = true) {
		return new (Value){ 
			ValueKind::FLOAT,
			{ .floating=value },
			read_only
		};
	}

	static Value *create_value(bool value, bool read_only = true) {
		return new (Value){ 
			ValueKind::BOOL,
			{ .boolean=value },
			read_only
		};
	}

	static Value *create_value(char *value, bool read_only = true) {
		return new (Value){ 
			ValueKind::STRING,
			{ .string=value },
			read_only
		};
	}

	static Value *create_value(Value **capture, size_t capture_len, bool read_only = true) {
		return new (Value){
			ValueKind::CAPTURE,
			{ 0 },
			read_only,
			capture_len,
			capture,
		};
	}


	static ValueKind kind_from_str(const char *name) {
		switch(Util::hash(name, std::strlen(name))) {
			case Util::hash("void", 4):			return ValueKind::VOID;
			case Util::hash("int", 3): 			return ValueKind::INT;
			case Util::hash("float", 5): 		return ValueKind::FLOAT;
			case Util::hash("bool", 4): 		return ValueKind::BOOL;
			case Util::hash("string", 6): 		return ValueKind::STRING;
			case Util::hash("struct", 6): 		return ValueKind::STRUCT;
			case Util::hash("capture", 8): 		return ValueKind::CAPTURE;
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
			case ValueKind::CAPTURE:	return "capture";
		}
	}

	static void write_value(Value *value) {
		switch(value->kind) {
			case ValueKind::VOID: 		std::cout << "void"; break;
			case ValueKind::INT: 		std::cout << value->data.integer; break;
			case ValueKind::FLOAT: 		std::cout << value->data.floating; break;
			case ValueKind::BOOL: 		std::cout << ((value->data.boolean) ? "true" : "false"); break;
			case ValueKind::STRING: 	std::cout << value->data.string; break;
			case ValueKind::STRUCT: 	std::cout << "struct"; break;
			case ValueKind::CAPTURE: {
				std::cout << "capture: ";
				for(size_t i = 0; i < value->capture_len; ++i) {
					write_value(value->capture[i]);
					std::cout << " ";
				}
				break;
			}
		}
	}
}