#pragma once

namespace Process {
	enum ByteCode {
		HALT,					// HALT
		PRINT, PRINTLN,			// PRINT|PRINTLN
		DROP,					// POP
		PUSH,					// PUSH IDX
		CAPTURE,				// CAPTURE COUNT
		BIND,					// BIND COUNT, [ID INDEX, ...]
		BIND_STRICT,			// BIND_STRICT, [ID INDEX, ...]
		LOAD_BINDING,			// LOAD_BINDING IDX
		CONV,					// CONV TYPEID [, ID] (id index + 1, 0 for none)
		PROCCALL,				// PROCCALL FUNCTION_DEF_IDX
		NATIVECALL,				// NATIVECALL FUNCTION_DEF_IDX
		GOTO,					// GOTO POS
		IF,						// IF GOTO_FALSE_POS
		LOOP,					// LOOP GOTO_FALSE_POS
		RETURN,					// RETURN SUB_IDX
		ADD, SUB, MUL, DIV,		// ADD|SUB|MUL|DIV
		GREATER, LESS, EQUAL,	// GREATER|LESS|EQUAL
		GREATER_EQ, LESS_EQ,	// GREATER_EQ|LESS_EQ
		DUPLICATE, SWAP,		// DUPLICATE|SWAP
		ROTATE,					// ROTATE
	};

	static const char *get_bytecode_name(ByteCode byte) {
		switch(byte) {
			case ByteCode::PUSH:			return "PUSH";
			case ByteCode::DROP:			return "DROP";
			case ByteCode::DUPLICATE:		return "DUPLICATE";
			case ByteCode::SWAP:			return "SWAP";
			case ByteCode::ROTATE:			return "ROTATE";

			case ByteCode::BIND:			return "BIND";
			case ByteCode::BIND_STRICT:		return "BIND_STRICT";
			case ByteCode::LOAD_BINDING:	return "LOAD_BINDING";


			case ByteCode::ADD:				return "ADD";
			case ByteCode::SUB:				return "SUB";
			case ByteCode::MUL:				return "MUL";
			case ByteCode::DIV:				return "DIV";

			case ByteCode::GREATER:			return "GREATER";
			case ByteCode::GREATER_EQ:		return "GREATER_EQ";
			case ByteCode::LESS:			return "LESS";
			case ByteCode::LESS_EQ:			return "LESS_EQ";
			case ByteCode::EQUAL:			return "EQUAL";

			case ByteCode::IF:				return "IF";
			case ByteCode::LOOP:			return "LOOP";
			
			case ByteCode::PROCCALL:		return "PROCCALL";
			case ByteCode::NATIVECALL:		return "NATIVECALL";
			case ByteCode::CAPTURE:			return "CAPTURE";
			case ByteCode::HALT:			return "HALT";
			case ByteCode::GOTO:			return "GOTO";
			case ByteCode::RETURN:			return "RETURN";

			case ByteCode::PRINT:			return "PRINT";
			case ByteCode::PRINTLN:			return "PRINTLN";

			default:	return "Unknown";
		}
	}
}