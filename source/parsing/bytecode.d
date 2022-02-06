module parsing.bytecode;

import std.stdio;
import parsing.environment;
import parsing.value;


final enum ByteCode : ubyte {
	PRINT, PRINTLN,			// PRINT|PRINTLN
	POP,					// POP
	PUSH,					// PUSH IDX
	BIND, BINDMOVE,			// BIND|BINDMOVE
	CONV,					// CONV TYPEID [, ID] (id index + 1, 0 for none)
	PROCCALL,				// PROCCALL FUNCTION_DEF_ID
	BINDING,				// BINDING BINDING_ID
	RETURN,					// RETURN
	ADD, SUB, MUL, DIV,		// ADD|SUB|MUL|DIV
}

void printCode(Environment env) {
	for(size_t i = 0; i < env.code.length; ++i) {
		writef("%04d ", i);

		switch(env.code[i]) {
			case ByteCode.PUSH: {
				immutable int idx = env.code[i+1];
				
				writef("%s<%d, ", env.code[i], idx);
				writeValue(env.literals[idx]);
				writeln(">");

				i++;
				break;
			}

			case ByteCode.PROCCALL: .. case ByteCode.RETURN: {
				immutable int idx = env.code[i+1];
				writefln("%s<%s>", env.code[i], idx);
				i++;
				break;
			}

			case ByteCode.CONV: {
				immutable int idxa = env.code[i+1];
				immutable int idxb = env.code[i+2];
				writefln("%s<%s, %s>", env.code[i], idxa, idxb);
				i+=2;
				break;
			}

			default: {
				writeln(env.code[i]);
				break;
			}
		}
	}
}