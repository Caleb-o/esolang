#pragma once
#include <string>

namespace Process {
	enum TokenKind {
		// Operators
		PLUS, MINUS, SLASH, STAR, COLON,
		ARROW, BANG, GREATER, LESS, GREATER_EQ, LESS_EQ, EQUAL,
		COMMA, DOT, LCURLY, RCURLY, LBRACKET, RBRACKET,
		
		// Keywords
		PROC, IF, ELIF, ELSE, POP, CONVERT, BIND, BIND_MOVE, PRINT, PRINTLN,
		DUP, SWAP, INPUT, ASSERT, USING, RETURN, STRUCT,

		// Types
		BOOL_LIT, INT_LIT, FLOAT_LIT, STRING_LIT, ID, TYPEID,

		ENDOFFILE,
	};
	
	struct Token {
		TokenKind kind;
		int line, col;
		std::string lexeme;
	};
}