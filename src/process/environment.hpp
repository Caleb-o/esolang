#pragma once
#include <map>
#include <vector>
#include "../runtime/value.hpp"

using namespace Runtime;

namespace Process {
	struct StructDef {
		std::map<std::string, ValueKind> fields;
	};

	struct Parameter {
		ValueKind kind;
		bool isMoved;
	};

	struct ProcedureDef {
		size_t startIdx;
		std::map<std::string, Parameter> parameters;
		std::vector<ValueKind> returnTypes;
	};

	struct Definitions {
		std::map<std::string, std::vector<ProcedureDef>> procedures;
		// std::map<std::string, StructDef> structs;
	};

	struct Environment {
		// TODO: Maybe consider an array here
		std::vector<ByteCode> code;
		// Should be unique, if a value already exists, get its index
		std::vector<Value> literals;
		// IDs
		std::vector<std::string> idLiterals;
		// Definitions are compile-time known structs and procedures
		Definitions defs;
	};
}