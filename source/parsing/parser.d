module parsing.parser;

import std.stdio;
import std.format;
import std.conv;
import core.stdc.stdlib : abort;
import std.algorithm : countUntil;

import lexing.token;
import lexing.lexer;
import parsing.environment;
import parsing.bytecode;
import parsing.value;


final class Parser {
	private {
		Environment env;
		Lexer lexer;
		Token current;
	}

	@disable this();
	this(string source) {
		this.env = new Environment();
		this.lexer = new Lexer(source);
		this.current = lexer.getNext();
	}

	private:

	void pushByte(ByteCode b) {
		env.code[env.code.length++] = b;
	}

	void pushBytes(ByteCode a, ulong b) {
		env.code[env.code.length++] = a;
		env.code[env.code.length++] = cast(ByteCode)(cast(ubyte)b);
	}

	void pushBytes(ByteCode a, ByteCode b) {
		env.code[env.code.length++] = a;
		env.code[env.code.length++] = b;
	}

	void consume(Kind kind) {
		if (current.kind == kind) {
			current = lexer.getNext();
		} else {
			writefln("Expected type '%s' ('%s') but got '%s' on line %d at pos %d", kind, current.lexeme, current.kind, current.line, current.col);
			abort();
		}
	}

	size_t addProcDef(string name) {
		size_t idx = env.defs.procedures.length++;

		env.defs.procedures[idx] = ProcedureDef(name);
		env.defs.procedures[idx].startIdx = env.code.length;

		return idx;
	}

	int indexOfProc(string name) {
		foreach(idx, proc; env.defs.procedures) {
			if (proc.name == name) return cast(int)idx;
		}

		return -1;
	}


	void error(string message) {
		writefln("%s on line %d at pos %d", message, current.line, current.col);
		// FIXME: Use exceptions instead, since this will exit the program
		abort();
	}

	size_t addToList(Value value) {
		size_t idx;
		if ((idx = countUntil(env.literals, value)) > -1) {
			return idx;
		}

		idx = env.literals.length;
		env.literals[env.literals.length++] = value;
		return idx;
	}

	size_t addLiteral() {
		switch(current.kind) {
			case Kind.INT: {
				ValueData data = { idata:to!int(current.lexeme) };
				Value value = Value(ValueKind.INT, data);
				return addToList(value);
			}

			case Kind.FLOAT: {
				ValueData data = { fdata:to!float(current.lexeme) };
				Value value = Value(ValueKind.FLOAT, data);
				return addToList(value);
			}

			case Kind.BOOL: {
				ValueData data = { bdata:to!bool(current.lexeme) };
				Value value = Value(ValueKind.BOOL, data);
				return addToList(value);
			}

			case Kind.STRING: {
				ValueData data = { sdata:current.lexeme };
				Value value = Value(ValueKind.STRING, data);
				return addToList(value);
			}

			default: {
				assert(0, format("Unreachable literal type: '%s'", current.kind));
			}
		}
	}

	size_t addIDLiteral() {
		env.idLiterals[env.idLiterals.length++] = current.lexeme;
		return env.idLiterals.length-1u;
	}
	
	void expr() {
		switch(current.kind) {
			case Kind.BOOL: .. case Kind.STRING: {
				pushBytes(ByteCode.PUSH, addLiteral());
				consume(current.kind);
				break;
			}
			
			case Kind.ID: {
				pushBytes(ByteCode.BINDING, addIDLiteral());
				consume(current.kind);
				break;
			}

			default: {
				error(format("Expected a value, but got '%s'", current.lexeme));
			}
		}
	}

	void procedureCall() {
		consume(current.kind);

		immutable string procName = current.lexeme;
		consume(Kind.ID);

		int procIdx = indexOfProc(procName);
		if (procIdx == -1) {
			error(format("Trying to call procedure that has not been defined yet '%s'", procName));
		}

		// Push proc call + idx of procedure
		pushBytes(ByteCode.PROCCALL, procIdx);
	}

	void arithmeticStatement() {
		auto op = current.kind;
		consume(op);

		switch(op) {
			case Kind.PLUS:			pushByte(ByteCode.ADD); break;
			case Kind.MINUS:		pushByte(ByteCode.SUB); break;
			case Kind.STAR:			pushByte(ByteCode.MUL); break;
			case Kind.SLASH:		pushByte(ByteCode.DIV); break;

			default: break;
		}
	}

	void statement() {
		switch(current.kind) {
			case Kind.PLUS: .. case Kind.STAR: arithmeticStatement(); break;

			case Kind.BANG:		procedureCall(); break;
			case Kind.POP:		consume(current.kind); pushByte(ByteCode.POP); break;
			case Kind.PRINT: 	consume(current.kind); pushByte(ByteCode.PRINT); break;
			case Kind.PRINTLN: 	consume(current.kind); pushByte(ByteCode.PRINTLN); break;
			default: 			expr(); break;
		}
	}

	void typeList(string id, bool isProc) {
		auto endType = (isProc) ? Kind.RBRACKET : Kind.RCURLY;
		bool hasMove = false;
		
		while(current.kind != endType) {
			string[] ids;

			// Parse multiple names for a single type
			while (current.kind != Kind.COLON) {
				ids[ids.length++] = current.lexeme;
				consume(Kind.ID);

				// Multiple names
				if (current.kind == Kind.COMMA) {
					consume(current.kind);
				}
			}
			consume(Kind.COLON);
			bool isMoved = true;

			// Modifier for duplicating data instead of moving
			if (current.kind == Kind.DUP) {
				if (hasMove) {
					// We cannot move values from within the stack, it must be at the
					// end. We can duplicate them all however.
					error("Cannot use dup modifier after move parameters");
				}

				consume(current.kind);
				isMoved = false;
			} else {
				hasMove = true;
			}

			immutable string typeName = current.lexeme;
			consume(Kind.TYPEID);

			// FIXME: Use isProc flag to check whether we're in a procedure list or struct

			int procIdx = indexOfProc(id);
			// Add all parameters to the current proc param list
			foreach(paramid; ids) {
				auto kind = getFromString(typeName);

				if (kind == ValueKind.VOID) {
					error(format("Cannot use type '%s' in parameter list", kind));
				}
				env.defs.procedures[procIdx].parameters[paramid] = Parameter(kind, isMoved);
			}

			// Multiple parameters
			if (current.kind == Kind.COMMA) {
				consume(current.kind);

				if (current.kind == endType && isProc) {
					error(format("Unexpected character found '%s' after comma", current.kind));
				}
			}
		}
	}

	void codeBlock() {
		consume(Kind.LCURLY);

		// Consume statements until we are at the end of the block
		while(current.kind != Kind.RCURLY) {
			statement();
		}
		consume(Kind.RCURLY);
	}

	void parameterList(string id) {
		consume(Kind.LBRACKET);
		typeList(id, true);
		consume(Kind.RBRACKET);
	}

	void usingStatement() {
		writeln("TODO: Unimplemented");
	}

	void structDefinition() {
		writeln("TODO: Unimplemented");
	}

	void procedureDefinition() {
		consume(current.kind);
		immutable string procName = current.lexeme;
		consume(Kind.ID);

		if (indexOfProc(procName) > -1) {
			error(format("Procedure '%s' has been redefined", procName));
		}

		size_t idx = addProcDef(procName);

		parameterList(procName);
		consume(Kind.ARROW);

		bool containsVoid = false;

		// Check for multiple return types
		while(current.kind == Kind.TYPEID) {
			immutable string returnTypeName = current.lexeme;
			consume(Kind.TYPEID);

			auto returnType = getFromString(returnTypeName);

			// Only add if it is not void
			if (returnType != ValueKind.VOID) {
				env.defs.procedures[idx]
					.returnTypes[env.defs.procedures[idx].returnTypes.length++] = returnType;
			} else {
				containsVoid = true;
			}

			// Void exists in return list
			if (containsVoid && env.defs.procedures[idx].returnTypes.length > 0) {
				error(format("'%s' has multiple return types and contains a void", procName));
			}

			// Comma
			if (current.kind == Kind.COMMA) {
				consume(current.kind);

				if (current.kind == Kind.LCURLY) {
					error(format("'%s' return list contains a trailing comma", procName));
				}
			}
		}

		codeBlock();

		// Implicit return added
		pushBytes(ByteCode.RETURN, idx);
	}

	void program() {
		while(current.kind != Kind.EOF) {
			switch(current.kind) {
				case Kind.USING: {
					usingStatement();
					break;
				}

				case Kind.STRUCT: {
					structDefinition();
					break;
				}

				case Kind.PROC: {
					procedureDefinition();
					break;
				}

				default: {
					// FIXME: Use exceptions instead, since this will exit the program
					error(format("Unexpected token found '%s'", current.kind));
				}
			}
		}
	}


	public:

	Environment parse() {
		env.defs = new Definitions();
		program();
		return env;
	}
}
