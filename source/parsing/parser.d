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
			writefln("Expected type '%s' but got '%s' on line %d at pos %d", kind, current.kind, current.line, current.col);
			abort();
		}
	}

	void addVoidValue(string id, bool literal) {
		ValueData data = { bdata:false };
		if (literal)
			env.literals[env.literals.length++] = Value(ValueKind.VOID, data);
		else
			env.variables[id] = Value(ValueKind.VOID, data);
	}

	void addIntValue(string id, int val, bool literal) {
		ValueData data = { idata:val };
		if (literal)
			env.literals[env.literals.length++] = Value(ValueKind.INT, data);
		else
			env.variables[id] = Value(ValueKind.INT, data);
	}

	void addFloatValue(string id, float val, bool literal) {
		ValueData data = { fdata:val };
		if (literal)
			env.literals[env.literals.length++] = Value(ValueKind.FLOAT, data);
		else
			env.variables[id] = Value(ValueKind.FLOAT, data);
	}

	void addStringValue(string id, string val, bool literal) {
		ValueData data = { sdata:val };
		if (literal)
			env.literals[env.literals.length++] = Value(ValueKind.STRING, data);
		else
			env.variables[id] = Value(ValueKind.STRING, data);
	}

	void addBoolValue(string id, bool val, bool literal) {
		ValueData data = { bdata:val };
		if (literal)
			env.literals[env.literals.length++] = Value(ValueKind.BOOl, data);
		else
			env.variables[id] = Value(ValueKind.BOOl, data);
	}

	void addLiteral() {
		switch(current.kind) {
			case Kind.INT: 		addIntValue("int", to!int(current.lexeme), true); break;
			case Kind.FLOAT: 	addFloatValue("float", to!float(current.lexeme), true); break;
			case Kind.STRING: 	addStringValue("str", current.lexeme, true); break;
			case Kind.BOOL: 	addBoolValue("bool", to!bool(current.lexeme), true); break;
			default:			break;
		}
	}

	void expr() {
		switch(current.kind) {
			case Kind.BOOL: .. case Kind.STRING: {
				addLiteral();
				consume(current.kind);
				pushBytes(ByteCode.PUSH, env.literals.length-1u);
				break;
			}

			default: {
				writefln("Unexpected statement token found '%s' on line %d at pos %d", current.kind, current.line, current.col);
				abort();
			}
		}
	}

	// TODO: Add parameter checking
	void parameterList() {
		consume(Kind.LBRACKET);

		// TOOD: Double-up as parameter decl (typeidentifier identifier [,])
		while (current.kind != Kind.RBRACKET) {
			if (current.kind == Kind.ID) {
				immutable string id = current.lexeme;

				if (id !in env.variables) {
					writefln("Trying to access invalid variable '%s' on line %d at pos %d", id, current.line, current.col);
					abort();
				}

				consume(current.kind);
				pushBytes(ByteCode.LOAD, 0);
			} else {
				addLiteral();
				pushBytes(ByteCode.LOAD_LIT, env.literals.length-1u);
			}
			consume(current.kind);

			if (current.kind == Kind.COMMA)
				consume(Kind.COMMA);
		}

		consume(Kind.RBRACKET);
	}

	void varDecl() {
		auto typeToken = current;
		immutable string type = current.lexeme;
		consume(Kind.TYPEID);
		immutable string name = current.lexeme;
		consume(Kind.ID);

		if (current.kind == Kind.EQUAL) {
			consume(Kind.EQUAL);
			expr();
		}

		// Store variable
		switch(type) {
			case "void": 		addVoidValue(name, false); break;
			case "int":			addIntValue(name, env.literals[$-1].data.idata, false); break;
			case "float":		addFloatValue(name, env.literals[$-1].data.fdata, false); break;
			case "bool":		addBoolValue(name, env.literals[$-1].data.bdata, false); break;
			case "string":		addStringValue(name, env.literals[$-1].data.sdata, false); break;

			default: {
				writefln("Unexpected typename provided '%s' on line %d at pos %d", type, typeToken.line, typeToken.col);
				abort();
			}
		}
		pushBytes(ByteCode.STORE, env.literals.length-1u);

		consume(Kind.SEMICOLON);
	}

	void functionDecl() {
		consume(Kind.FUNCTION_DECL);
		immutable string funcName = current.lexeme;
		consume(Kind.ID);

		env.functions[funcName] = env.code.length-1u;

		parameterList();

		codeBlock();
	}

	void printStatement(bool line) {
		consume(current.kind);

		if (current.kind == Kind.ID) 
			pushBytes(ByteCode.LOAD, countUntil(env.literals, env.variables[current.lexeme]));
		else { 
			addLiteral(); 
			pushBytes(ByteCode.LOAD_LIT, env.literals.length-1u);
		}
		consume(current.kind);

		pushByte((line) ? ByteCode.PRINTLN : ByteCode.PRINT);

		consume(Kind.SEMICOLON);
	}

	void statement() {
		switch(current.kind) {
			case Kind.FUNCTION_DECL: {
				functionDecl();
				break;
			}

			case Kind.TYPEID: {
				varDecl();
				break;
			}

			case Kind.PRINT: {
				printStatement(false);
				break;
			}

			case Kind.PRINTLN: {
				printStatement(true);
				break;
			}

			default: {
				expr();
				break;
			}
		}
	}

	void codeBlock() {
		consume(Kind.LCURLY);
		while (current.kind != Kind.RCURLY) {
			statement();
		}
		consume(Kind.RCURLY);
	}
	
	public:

	Environment parse() {
		statement();
		return env;
	}
}
