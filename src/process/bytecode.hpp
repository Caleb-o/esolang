#pragma once

namespace Process {
	enum ByteCode {
		HALT,					// HALT
		PRINT, PRINTLN,			// PRINT|PRINTLN
		DROP,					// POP
		PUSH,					// PUSH IDX
		BIND, BINDMOVE,			// BIND|BINDMOVE
		CONV,					// CONV TYPEID [, ID] (id index + 1, 0 for none)
		PROCCALL,				// PROCCALL FUNCTION_DEF_ID
		BINDING,				// BINDING BINDING_ID
		GOTO,					// GOTO POS
		IF,						// IF GOTO_FALSE_POS
		RETURN,					// RETURN
		ADD, SUB, MUL, DIV,		// ADD|SUB|MUL|DIV
		GREATER, LESS, EQUAL,	// GREATER|LESS|EQUAL
		GREATER_EQ, LESS_EQ,	// GREATER_EQ|LESS_EQ
		DUPLICATE, SWAP,		// DUPLICATE|SWAP
	};

	static const char *get_bytecode_name(ByteCode byte) {
		switch(byte) {
			case ByteCode::PUSH:		return "PUSH";
			case ByteCode::DROP:		return "DROP";

			case ByteCode::ADD:			return "ADD";
			case ByteCode::SUB:			return "SUB";
			case ByteCode::MUL:			return "MUL";
			case ByteCode::DIV:			return "DIV";
			
			case ByteCode::PROCCALL:	return "PROCCALL";
			case ByteCode::HALT:		return "HALT";
			case ByteCode::GOTO:		return "GOTO";
			case ByteCode::RETURN:		return "RETURN";
			
			case ByteCode::PRINT:		return "PRINT";
			case ByteCode::PRINTLN:		return "PRINTLN";

			default:	return "Unknown";
		}
	}
}