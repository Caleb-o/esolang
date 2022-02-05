module parsing.environment;
import parsing.bytecode : ByteCode;
import parsing.value;


struct StructDef {
	Value[string] fields;
}


struct Parameter {
	ValueKind kind;
	bool isMoved;
}

struct ProcedureDef {
	size_t startIdx;
	Parameter[string] parameters;
	ValueKind[] returnTypes;
}


final class Definitions {
	public:

	ProcedureDef[string] procedures;
	StructDef[string] structs;
}


final class Environment {
	public:
	
	ByteCode[] code;
	// Should be unique, if a value already exists, get its index
	Value[] literals;
	// Definitions are compile-time known structs and procedures
	Definitions defs;
	// TODO: Callstack
}