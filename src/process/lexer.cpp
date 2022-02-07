#include <iostream>
#include <string>
#include "lexer.hpp"
#include "util.hpp"


namespace Process {
	// Helper functions
	static bool is_alpha(char c) {
		return  (c >= 'a' && c <= 'z') ||
				(c >= 'A' && c <= 'Z') ||
				(c == '_');
	}

	static bool is_numeric(char c) {
		return c >= '0' && c <= '9';
	}

	static bool is_alphanum(char c) {
		return is_alpha(c) || is_numeric(c);
	}

	// Lexer
	Lexer::Lexer(const char *source)
		:m_source(source) {}

	// Helper methods
	void Lexer::skip_whitespace() {
		while(m_ip < m_source.size()) {
			switch(m_source[m_ip]) {
				// Comments
				case '#': {
					m_ip++;
					
					while(m_ip < m_source.size() && m_source[m_ip++] != '\n');
					break;
				}

				case '\n': {
					m_col = 1;
					m_line++;
					m_ip++;
					break;
				}

				case '\t':
				case '\b':
				case '\r':
				case ' ': {
					m_col++;
					m_ip++;
					break;
				}

				default: return;
			}
		}
	}

	char Lexer::peek(size_t offset) {
		int idx = m_ip + offset;

		if (idx >= m_source.size() || idx < 0) return '\0';
		return m_source[idx];
	}

	// Main

	Token *Lexer::make_identifier() {
		size_t start_ip = m_ip++;

		while(m_ip < m_source.size() && is_alphanum(m_source[m_ip])) m_ip++;

		std::string lexeme(m_source.substr(start_ip, m_ip - start_ip));
		return new (Token){ Util::get_keyword_kind(lexeme), m_line, m_col, lexeme };
	}

	Token *Lexer::make_number() {
		size_t start_ip = m_ip;
		TokenKind kind = TokenKind::INT_LIT;
		bool isFloat = false;

		while(m_ip < m_source.size() && is_numeric(m_source[m_ip])) {
			m_ip++;

			if (m_source[m_ip] == '.') {
				if (isFloat) {
					// TODO: Throw exception, trying to add multiple decimal points
				}
				m_ip++;

				isFloat = true;
				kind = TokenKind::FLOAT_LIT;
			}
		}

		std::string lexeme(m_source.substr(start_ip, m_ip - start_ip));
		return new (Token){ Util::get_keyword_kind(lexeme), m_line, m_col, lexeme };
	}

	Token *Lexer::make_single(TokenKind kind) {
		m_ip++;
		std::string lexeme(m_source.substr(m_ip - 1, 1));
		return new (Token){ kind, m_line, m_col, lexeme };
	}


	Token *Lexer::get_token() {
		while(m_ip < m_source.size()) {
			skip_whitespace();

			// End of file reached when skipping whitespace
			if (m_ip >= m_source.size()) break;


			if (is_alpha(m_source[m_ip])) {
				return make_identifier();
			}

			if (is_numeric(m_source[m_ip])) {
				return make_number();
			}

			// Single character tokens
			switch(m_source[m_ip]) {
				case '+': return make_single(TokenKind::PLUS);
				case '-': {
					if (peek(1) == '>') {
						return make_single(TokenKind::ARROW);
					} else {
						return make_single(TokenKind::MINUS);
					}
				}
				case '/': return make_single(TokenKind::SLASH);
				case '*': return make_single(TokenKind::STAR);
				case ':': return make_single(TokenKind::COLON);
				case ',': return make_single(TokenKind::COMMA);
				case '.': return make_single(TokenKind::DOT);
				case '!': return make_single(TokenKind::BANG);
				case '=': return make_single(TokenKind::EQUAL);
				case '>': {
					if (peek(1) == '=') {
						return make_single(TokenKind::GREATER_EQ);
					} else {
						return make_single(TokenKind::GREATER);
					}
				}
				case '<': {
					if (peek(1) == '=') {
						return make_single(TokenKind::LESS_EQ);
					} else {
						return make_single(TokenKind::LESS);
					}
				}
				case '(': return make_single(TokenKind::LBRACKET);
				case ')': return make_single(TokenKind::RBRACKET);
				case '{': return make_single(TokenKind::LCURLY);
				case '}': return make_single(TokenKind::RCURLY);
				// case '\'': return makeString();
				default: {
					m_ip++;
					// FIXME: Use Exceptions here instead of straight up aborting
					break;
				}
			}
		}

		return new (Token){ TokenKind::ENDOFFILE, m_line, m_col, "EOF" };
	}
}