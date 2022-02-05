module parsing.parser;

import std.stdio;
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
	
	void expr() {

	}

	void statement() {
		
	}

	void typeList(string id, bool isProc) {
		auto expected = (isProc) ? Kind.RBRACKET : Kind.RCURLY;
		
		while(current.kind != expected) {
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
				consume(current.kind);
				isMoved = false;
			}

			immutable string typeName = current.lexeme;
			consume(Kind.TYPEID);

			// FIXME: Use isProc flag to check whether we're in a procedure list or struct

			// Add all parameters to the current proc param list
			foreach(paramid; ids) {
				env.defs.procedures[id].parameters[paramid] = Parameter(getFromString(typeName), isMoved);
			}

			// Multiple parameters
			if (current.kind == Kind.COMMA) {
				consume(current.kind);
			}
		}
	}

	void parameterList(string id) {
		consume(Kind.LBRACKET);
		typeList(id, true);
		consume(Kind.RBRACKET);
	}

	void usingStatement() {

	}

	void structDefinition() {

	}

	void procedureDefinition() {
		consume(current.kind);
		immutable string procName = current.lexeme;

		env.defs.procedures[procName] = ProcedureDef();
		
		consume(Kind.ID);
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
					writeln("Unexpected token found '%s' on line %d at pos %d", current.kind, current.line, current.col);
					// FIXME: Use exceptions instead, since this will exit the program
					abort();
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
