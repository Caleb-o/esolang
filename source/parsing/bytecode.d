module parsing.bytecode;

import std.stdio;
import parsing.environment;


final enum ByteCode : ubyte {
	PRINT, PRINTLN,			// PRINT|PRINTLN
	POP,					// POP
	PUSH,					// PUSH IDX
	STORE,					// STORE IDX
	LOAD, LOAD_LIT,			// LOAD|LOAD_LIT IDX
	ADD, SUB, MUL, DIV,		// ADD|SUB|MUL|DIV
}

void printCode(Environment env) {
	for(size_t i = 0; i < env.code.length; ++i) {
		switch(env.code[i]) {
			case ByteCode.PUSH: {
				immutable int idx = env.code[i+1];
				writefln("%s<%s>", env.code[i], env.literals[idx]);
				i++;
				break;
			}

			case ByteCode.STORE: .. case ByteCode.LOAD_LIT: {
				immutable int idx = env.code[i+1];
				writefln("%s<%s>", env.code[i], idx);
				i++;
				break;
			}

			default: {
				writeln(env.code[i]);
				break;
			}
		}
	}
}