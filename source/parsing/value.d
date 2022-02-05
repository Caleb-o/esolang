module parsing.value;

import std.stdio;


enum ValueKind {
	VOID, INT, FLOAT, STRING, BOOl,
}

union ValueData {
	bool bdata;
	int idata;
	float fdata;
	string sdata;
}

struct Value {
	ValueKind kind;
	ValueData data;
}

void writeValue(Value value) {
	final switch(value.kind) {
		case ValueKind.VOID: 	write("VOID"); break;
		case ValueKind.INT:		write(value.data.idata); break;
		case ValueKind.FLOAT:	write(value.data.fdata); break;
		case ValueKind.STRING: 	write(value.data.sdata); break;
		case ValueKind.BOOl: 	write(value.data.bdata); break;
	}
}