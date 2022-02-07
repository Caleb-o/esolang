#pragma once
#include <string>

namespace Process {
	enum class TokenKind {
		// Operators
		PLUS, MINUS, SLASH, STAR, COLON,
		ARROW, BANG, GREATER, LESS, GREATER_EQ, LESS_EQ, EQUAL,
		COMMA, DOT, LCURLY, RCURLY, LPAREN, RPAREN,
		
		// Keywords
		PROC, IF, ELIF, ELSE, POP, CONVERT, BIND, BIND_MOVE, PRINT, PRINTLN,
		DUP, SWAP, INPUT, ASSERT, USING, RETURN, STRUCT,

		// Types
		BOOL_LIT, INT_LIT, FLOAT_LIT, STRING_LIT, ID, TYPEID,

		ENDOFFILE,
	};

	static const char *get_token_name(TokenKind kind) {
		switch(kind) {
			case TokenKind::PLUS:	return "plus";
			case TokenKind::MINUS:	return "minus";
			case TokenKind::STAR:	return "star";
			case TokenKind::SLASH:	return "slash";

			case TokenKind::ARROW:	return "arrow";
			case TokenKind::BANG:	return "bang";

			case TokenKind::PROC:	return "proc";
			case TokenKind::ID:		return "ID";
			case TokenKind::TYPEID:	return "typeid";
			case TokenKind::LPAREN:	return "lparen";
			case TokenKind::RPAREN:	return "rparen";
			case TokenKind::LCURLY:	return "lcurly";
			case TokenKind::RCURLY:	return "rcurly";

			default: return "Unknown";
		}
	}

	struct Token {
		TokenKind kind;
		int line, col;
		std::string lexeme;
	};
}