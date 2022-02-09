#pragma once
#include <string>

namespace Process {
	enum class TokenKind {
		// Operators
		PLUS, MINUS, SLASH, STAR, COLON, CAPTURE,
		ARROW, BANG, AT, GREATER, LESS, GREATER_EQ, LESS_EQ, EQUAL,
		COMMA, DOT, LCURLY, RCURLY, LPAREN, RPAREN,
		
		// Keywords
		PROC, IF, ELSE, POP, CONVERT, BIND, BIND_STRICT, PRINT, PRINTLN,
		DUP, SWAP, ROT, INPUT, ASSERT, USING, RETURN, STRUCT, LOOP,

		// Types
		BOOL_LIT, INT_LIT, FLOAT_LIT, STRING_LIT, ID, TYPEID,

		ENDOFFILE,
	};

	static const char *get_token_name(TokenKind kind) {
		switch(kind) {
			case TokenKind::PLUS:		return "plus";
			case TokenKind::MINUS:		return "minus";
			case TokenKind::STAR:		return "star";
			case TokenKind::SLASH:		return "slash";

			case TokenKind::INT_LIT:	return "int literal";
			case TokenKind::FLOAT_LIT:	return "float literal";
			case TokenKind::BOOL_LIT:	return "bool literal";
			case TokenKind::STRING_LIT:	return "string literal";

			case TokenKind::ARROW:		return "arrow";
			case TokenKind::BANG:		return "bang";
			case TokenKind::AT:			return "at";

			case TokenKind::USING:		return "using";

			case TokenKind::IF:			return "if";
			case TokenKind::ELSE:		return "else";
			case TokenKind::LOOP:		return "loop";
			case TokenKind::DUP:		return "dup";
			case TokenKind::POP:		return "drop";
			case TokenKind::ROT:		return "rot";
			case TokenKind::PRINT:		return "print";
			case TokenKind::PRINTLN:	return "println";

			case TokenKind::PROC:		return "proc";
			case TokenKind::BIND:		return "bind";
			case TokenKind::BIND_STRICT:	return "bind-move";
			case TokenKind::CAPTURE:	return "capture";
			case TokenKind::ID:			return "ID";
			case TokenKind::COMMA:		return "comma";
			case TokenKind::TYPEID:		return "typeid";
			case TokenKind::LPAREN:		return "lparen";
			case TokenKind::RPAREN:		return "rparen";
			case TokenKind::LCURLY:		return "lcurly";
			case TokenKind::RCURLY:		return "rcurly";

			case TokenKind::ENDOFFILE: 	return "EOF";

			default: return "Unknown";
		}
	}

	struct Token {
		TokenKind kind;
		int line, col;
		std::string lexeme;
	};
}