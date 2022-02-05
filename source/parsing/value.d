module parsing.value;

import std.stdio;


enum ValueKind {
	VOID, INT, FLOAT, STRING, BOOl, STRUCT,
}

union ValueData {
	bool bdata;
	int idata;
	float fdata;
	string sdata;
	ValueData[string] structdata;
}

struct Value {
	ValueKind kind;
	ValueData data;
}

ValueKind getFromString(string typeName) {
	final switch(typeName) {
		case "void":	return ValueKind.VOID;
		case "int":		return ValueKind.INT;
		case "float":	return ValueKind.FLOAT;
		case "bool":	return ValueKind.BOOl;
		case "string":	return ValueKind.STRING;
		case "struct":	return ValueKind.STRUCT;
	}
}

void writeValue(Value value) {
	final switch(value.kind) {
		case ValueKind.VOID: 	write("VOID"); break;
		case ValueKind.INT:		write(value.data.idata); break;
		case ValueKind.FLOAT:	write(value.data.fdata); break;
		case ValueKind.STRING: 	writef("'%s'", value.data.sdata); break;
		case ValueKind.BOOl: 	write(value.data.bdata); break;
		case ValueKind.STRUCT: 	break; // FIXME: Print struct fields
	}
}