module lexing.lexer;

import lexing.token : Token, Kind;
import std.stdio;
import std.ascii;
import core.stdc.stdlib : abort;


final class Lexer {
	private {
		Token[] tokens;
		uint line, col;
		string source;
		size_t ip;
	}

	@disable this();
	this(string source) {
		this.source = source;
		line = 1;
	}

	private:

	void skipWhitespace() {
		skipping: while(ip < source.length) {
			switch(source[ip]) {
				// Skip comments
				case '#': {
					ip++;
					col++;
					while(source[ip++] != '\n') {}
					break;
				}

				// Whitespace
				case '\n': {
					col = 0;
					line++;
					ip++;
					break;
				}

				case '\b': goto case ' ';
				case '\t': goto case ' ';
				case '\r': goto case ' ';
				case ' ': ip++; break;

				// Non-whitespace found
				default: break skipping;
			}
		}
	}

	auto makeIdentifier() {
		immutable size_t start = ip++;
		col++;

		while(ip < source.length && (source[ip].isAlpha || source[ip] == '_')) { ip++; col++; }

		// Hack: Must be here, since it does not evaluate as a constant and cannot be static
		immutable Kind[string] KEYWORDS = [
			"proc":		Kind.PROC,
			"dup":		Kind.DUP,
			"swap":		Kind.SWAP,
			"input":	Kind.INPUT,
			"pop":		Kind.POP,
			"conv":		Kind.CONVERT,
			"bind":		Kind.BIND,
			"bindmove": Kind.BIND_MOVE,
			"using":	Kind.USING,
			"assert":	Kind.ASSERT,
			"return":	Kind.RETURN,
			"if":		Kind.IF,
			"elif":		Kind.ELIF,
			"else":		Kind.ELSE,
			"print": 	Kind.PRINT,
			"println": 	Kind.PRINTLN,
			"true":		Kind.BOOL,
			"false":	Kind.BOOL,
			"void" : 	Kind.TYPEID,
			"int":		Kind.TYPEID,
			"float":	Kind.TYPEID,
			"string":	Kind.TYPEID,
			"bool": 	Kind.TYPEID,
			"struct":	Kind.TYPEID,
		];

		immutable string word = source[start..ip];
		return new Token(line, col, word, (word in KEYWORDS) ? KEYWORDS[word] : Kind.ID);
	}

	auto makeString() {
		immutable size_t start = ++ip;
		col++;

		while(ip < source.length && source[ip] != '\'') { ip++; col++; }

		ip++;
		col++;

		return new Token(line, col, source[start..ip-1], Kind.STRING);
	}

	auto makeNumber() {
		immutable size_t start = ip++;
		auto kind = Kind.INT;
		auto isFloat = false;

		auto checkFloat = {
			if (source[ip] == '.') {
				if (isFloat) {
					writeln("Trying to use multiple decimals in a single number");
					abort();
				}

				isFloat = true;
				kind = Kind.FLOAT;
				ip++;
				col++;
			}
		};

		checkFloat();


		while(ip < source.length && source[ip].isDigit) {
			if (source[ip] == '.') {
				checkFloat();
				continue;
			}
			col++;
			ip++;
		}

		return new Token(line, col, source[start..ip], kind);
	}

	auto makeCharToken(Kind kind, int count = 1) {
		ip += count;
		col += count;
		return new Token(line, col, source[ip-count..ip], kind);
	}

	public:

	char peek(int depth) {
		if (ip + depth >= 0 && ip + depth >= source.length) return '\0';
		return source[ip + depth];
	}

	Token getNext() {
		while (ip < source.length) {
			skipWhitespace();

			if (ip >= source.length) break;

			if (source[ip].isAlpha || source[ip] == '_') {
				return makeIdentifier();
			}

			if (source[ip].isDigit) {
				return makeNumber();
			}

			// Single token or special types
			switch(source[ip]) {
				case '+': return makeCharToken(Kind.PLUS);
				case '-': {
					if (peek(1) == '>') {
						scope(exit) ip++;
						return makeCharToken(Kind.ARROW);
					} else {
						return makeCharToken(Kind.MINUS);
					}
				}
				case '/': return makeCharToken(Kind.SLASH);
				case '*': return makeCharToken(Kind.STAR);
				case '=': return makeCharToken(Kind.EQUAL);
				case ':': return makeCharToken(Kind.COLON);
				case ',': return makeCharToken(Kind.COMMA);
				case '.': return makeCharToken(Kind.DOT);
				case '!': return makeCharToken(Kind.BANG);
				case '>': return makeCharToken(Kind.GREATER);
				case '<': return makeCharToken(Kind.LESS);
				case '(': return makeCharToken(Kind.LBRACKET);
				case ')': return makeCharToken(Kind.RBRACKET);
				case '{': return makeCharToken(Kind.LCURLY);
				case '}': return makeCharToken(Kind.RCURLY);
				case '\'': return makeString();
				default: {
					// FIXME: Use Exceptions here instead of straight up aborting
					writefln("Found unknown character '%c' on line %d at position %d", source[ip], line, col);
					abort();
				}
			}
		}

		return new Token(line, col, "EOF", Kind.EOF);
	}
}