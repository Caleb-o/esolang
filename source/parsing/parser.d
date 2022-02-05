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
				Value value = Value(ValueKind.BOOl, data);
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
	
	void expr() {
		switch(current.kind) {
			case Kind.BOOL: .. case Kind.STRING: {
				pushBytes(ByteCode.PUSH, addLiteral());
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

		if (procName !in env.defs.procedures) {
			error(format("Trying to call procedure that has not been defined yet '%s'", procName));
		}

		// Push proc call + idx of procedure
		pushBytes(ByteCode.PROCCALL, countUntil(env.defs.procedures.byKey, procName));
	}

	void statement() {
		switch(current.kind) {
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

			// Add all parameters to the current proc param list
			foreach(paramid; ids) {
				auto kind = getFromString(typeName);

				if (kind == ValueKind.VOID) {
					error(format("Cannot use type '%s' in parameter list", kind));
				}
				env.defs.procedures[id].parameters[paramid] = Parameter(kind, isMoved);
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

	void parameterList(string id) {
		consume(Kind.LBRACKET);
		typeList(id, true);
		consume(Kind.RBRACKET);
	}

	void usingStatement() {
		assert(0, "TOOD: Unimplemented");
	}

	void structDefinition() {
		assert(0, "TOOD: Unimplemented");
	}

	void procedureDefinition() {
		consume(current.kind);
		immutable string procName = current.lexeme;
		consume(Kind.ID);

		if (procName in env.defs.procedures) {
			error(format("Procedure '%s' has been redefined", procName));
		}

		env.defs.procedures[procName] = ProcedureDef();
		env.defs.procedures[procName].startIdx = env.code.length;


		parameterList(procName);
		consume(Kind.ARROW);

		// FIXME: Allow multiple return types, seperated by comma
		immutable string returnTypeName = current.lexeme;
		consume(Kind.TYPEID);

		env.defs.procedures[procName]
			.returnTypes[env.defs.procedures[procName].returnTypes.length++] = getFromString(returnTypeName);

		consume(Kind.LCURLY);

		// Consume statements until we are at the end of the block
		while(current.kind != Kind.RCURLY) {
			statement();
		}

		// Implicit return added
		pushByte(ByteCode.RETURN);

		consume(Kind.RCURLY);
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
