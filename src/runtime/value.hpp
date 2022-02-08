#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <memory>
#include "../process/util.hpp"


namespace Runtime {
	enum class ValueKind {
		VOID, INT, FLOAT, BOOL, STRING, STRUCT, CAPTURE,
	};

	union ValueData {
		int integer;
		float floating;
		bool boolean;
	};
	
	struct Value {
		ValueKind kind;
		ValueData data;
		bool read_only;

		size_t capture_len;
		std::vector<std::shared_ptr<Value>> capture;
		
		std::string string;
	};

	static std::shared_ptr<Value> create_value(int value, bool read_only = true) {
		std::shared_ptr<Value> val = std::make_shared<Value>();
		*val = (Value){ 
			ValueKind::INT,
			{ .integer=value },
			read_only
		};
		return val;
	}

	static std::shared_ptr<Value> create_value(float value, bool read_only = true) {
		std::shared_ptr<Value> val = std::make_shared<Value>();
		*val = (Value){ 
			ValueKind::FLOAT,
			{ .floating=value },
			read_only
		};
		return val;
	}

	static std::shared_ptr<Value> create_value(bool value, bool read_only = true) {
		std::shared_ptr<Value> val = std::make_shared<Value>();
		*val = (Value){ 
			ValueKind::BOOL,
			{ .boolean=value },
			read_only
		};
		return val;
	}

	static std::shared_ptr<Value> create_value(std::string value, bool read_only = true) {
		std::shared_ptr<Value> val = std::make_shared<Value>();
		*val = (Value){
			ValueKind::STRING,
			{ 0 },
			read_only
		};
		val->string = value;
		return val;
	}

	static std::shared_ptr<Value> create_value(std::vector<std::shared_ptr<Value>> capture, size_t capture_len, bool read_only = true) {
		std::shared_ptr<Value> val = std::make_shared<Value>();
		*val = (Value){
			ValueKind::CAPTURE,
			{ 0 },
			read_only,
			capture_len,
			capture,
		};
		return val;
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

	static void write_value(std::shared_ptr<Value> value) {
		switch(value->kind) {
			case ValueKind::VOID: 		std::cout << "void"; break;
			case ValueKind::INT: 		std::cout << value->data.integer; break;
			case ValueKind::FLOAT: 		std::cout << value->data.floating; break;
			case ValueKind::BOOL: 		std::cout << ((value->data.boolean) ? "true" : "false"); break;
			case ValueKind::STRING: 	std::cout << value->string; break;
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