module lexing.token;

import std.stdio : writefln;

final enum Kind : uint {
	// Operators
	PLUS, MINUS, SLASH, STAR, COLON,
	ARROW, BANG, GREATER, LESS, EQUAL,
	COMMA, DOT, LCURLY, RCURLY, LBRACKET, RBRACKET,
	
	// Keywords
	PROC, IF, ELIF, ELSE, POP, CONVERT, BIND, BIND_MOVE, PRINT, PRINTLN,
	DUP, SWAP, INPUT, ASSERT, USING, RETURN, STRUCT,

	// Types
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
			// Singles
			case Kind.PLUS: .. case Kind.STRUCT:
				writefln("%s", kind);
				break;

			// Types + value
			case Kind.BOOL: .. case Kind.FLOAT:
				writefln("%s(%s)", kind, lexeme);
				break;

			// Named / Strings
			default:
				// String/Named types
				writefln("%s('%s')", kind, lexeme);
				break;
		}
	}
}