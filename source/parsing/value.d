module parsing.value;

import std.stdio;


enum ValueKind {
	VOID, INT, FLOAT, STRING, BOOL, STRUCT,
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
		case "bool":	return ValueKind.BOOL;
		case "string":	return ValueKind.STRING;
		case "struct":	return ValueKind.STRUCT;
	}
}

Value createValue(int value) {
	ValueData data = { idata:value };
	return Value(ValueKind.INT, data);
}

Value createValue(float value) {
	ValueData data = { fdata:value };
	return Value(ValueKind.FLOAT, data);
}

Value createValue(bool value) {
	ValueData data = { bdata:value };
	return Value(ValueKind.BOOL, data);
}

Value createValue(string value) {
	ValueData data = { sdata:value };
	return Value(ValueKind.STRING, data);
}

void writeValue(Value value) {
	final switch(value.kind) {
		case ValueKind.VOID: 	write("VOID"); break;
		case ValueKind.INT:		write(value.data.idata); break;
		case ValueKind.FLOAT:	write(value.data.fdata); break;
		case ValueKind.STRING: 	writef(value.data.sdata); break;
		case ValueKind.BOOL: 	write(value.data.bdata); break;
		case ValueKind.STRUCT: 	break; // FIXME: Print struct fields
	}
}