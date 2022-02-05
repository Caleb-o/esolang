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
			"fn": 		Kind.FUNCTION_DECL,
			"print": 	Kind.PRINT,
			"println": 	Kind.PRINTLN,
			"true":		Kind.BOOL,
			"false":	Kind.BOOL,
			"void" : 	Kind.TYPEID,
			"int":		Kind.TYPEID,
			"float":	Kind.TYPEID,
			"string":	Kind.TYPEID,
			"bool": 	Kind.TYPEID,
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

	auto makeSingle(Kind kind) {
		ip++;
		col++;
		return new Token(line, col, source[ip-1..ip], kind);
	}

	public:

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
				case '+': return makeSingle(Kind.PLUS);
				case '-': return makeSingle(Kind.MINUS);
				case '/': return makeSingle(Kind.SLASH);
				case '*': return makeSingle(Kind.STAR);
				case '=': return makeSingle(Kind.EQUAL);
				case ';': return makeSingle(Kind.SEMICOLON);
				case ':': return makeSingle(Kind.COLON);
				case ',': return makeSingle(Kind.COMMA);
				case '.': return makeSingle(Kind.DOT);
				case '(': return makeSingle(Kind.LBRACKET);
				case ')': return makeSingle(Kind.RBRACKET);
				case '{': return makeSingle(Kind.LCURLY);
				case '}': return makeSingle(Kind.RCURLY);
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