module lexing.token;

import std.stdio : writefln;

final enum Kind : uint {
	PLUS, MINUS, SLASH, STAR, SEMICOLON, COLON,
	FUNCTION_DECL, PRINT, PRINTLN, EQUAL,
	COMMA, DOT, LCURLY, RCURLY, LBRACKET, RBRACKET,

	BOOL, INT, FLOAT, STRING, ID, TYPEID,

	EOF,
}

class Token {
	public {
		immutable uint line, col;
		immutable string lexeme;
		immutable Kind kind;	
	}

	@disable this();
	this(int line, int col, string lexeme, Kind kind) {
		this.line = line;
		this.col = col;
		this.lexeme = lexeme;
		this.kind = kind;
	}

	void print() {
		switch(kind) {
			case Kind.PLUS: .. case Kind.RBRACKET:
				writefln("%s", kind);
				break;

			case Kind.INT: .. case Kind.FLOAT:
				writefln("%s(%s)", kind, lexeme);
				break;

			default:
				// String/Named types
				writefln("%s('%s')", kind, lexeme);
				break;
		}
	}
}