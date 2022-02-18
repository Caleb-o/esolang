package process

import "core:fmt"
import "core:strings"
import "../misc"
import "../info"


Lexer :: struct {
	_line, _col, _ip : int,
	source : string,
}

@(private)
is_alpha :: proc(ch : u8) -> bool {
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_'
}

@(private)
is_digit :: proc(ch : u8) -> bool {
	return ch >= '0' && ch <= '9'
}

@(private)
is_alpha_num :: proc(ch : u8) -> bool {
	return is_alpha(ch) || is_digit(ch)
}

@(private)
advance :: proc(lexer : ^Lexer) {
	lexer._ip += 1
	lexer._col += 1
}

@(private)
make_token :: proc(lexer : ^Lexer, kind : Token_Type, lexeme : string) -> ^Token {
	token := new(Token)
	token^ = Token { kind, lexer._line, lexer._col, lexeme }
	return token
}

// Skips all whitespace characters including comments
@(private)
skip_whitespace :: proc(lexer : ^Lexer) {
	src_len := len(lexer.source)

	outer: for lexer._ip < src_len {
		switch lexer.source[lexer._ip] {
			// Comment
			case '#':
				advance(lexer)
				for lexer._ip < src_len && lexer.source[lexer._ip] != '\n' do advance(lexer)

			case '\n':
				lexer._ip += 1
				lexer._line += 1
				lexer._col = 1
			
			case '\t'..'\r', ' ':
				advance(lexer)
			
			case: // None of the above
				break outer
		}
	}
}

@(private)
make_identifier :: proc(lexer : ^Lexer) -> ^Token {
	start_idx := lexer._ip
	
	// Skip initial item
	advance(lexer)

	for lexer._ip < len(lexer.source) && is_alpha_num(lexer.source[lexer._ip]) do advance(lexer)

	lexeme := string(lexer.source[start_idx:lexer._ip])

	if elem, ok := RESERVED[lexeme]; ok {
		return make_token(lexer, elem, lexeme)
	} else {
		// Try to check for typo
		lower_lexeme := strings.to_lower(lexeme)

		// To lower version matches a keyword
		// TODO: Use flag that disables this check (On by default)
		if lower_lexeme in RESERVED {
			formatted := fmt.aprintf("'%s' matches a keyword. Did you mean '%s'?", lexeme, lower_lexeme)
			info.log(info.Log_Level_Flag.Warning, formatted, lexer._line, lexer._col)
			delete(formatted)
		}
		delete(lower_lexeme)

		return make_token(lexer, .Id, lexeme)
	}
}

@(private)
make_string :: proc(lexer : ^Lexer) -> (^Token, misc.Eso_Status) {
	// Skip initial quote
	advance(lexer)

	start_idx := lexer._ip

	for lexer._ip < len(lexer.source) && lexer.source[lexer._ip] != '\'' do advance(lexer)

	// Consume other quote
	advance(lexer)

	// Unterminated string
	if lexer._ip >= len(lexer.source) && lexer.source[lexer._ip-1] != '\'' {
		info.log(info.Log_Level_Flag.Error, "Unterminated string", lexer._line, lexer._col)
		return make_token(lexer, .Eof, "Error"), .LexerErr
	}

	return make_token(lexer, .String_Lit, string(lexer.source[start_idx:lexer._ip-1])), .Ok
}

@(private)
make_numeric :: proc(lexer : ^Lexer) -> (^Token, misc.Eso_Status) {
	start_idx := lexer._ip
	kind := Token_Type.Int_Lit
	has_floating_point := false

	for lexer._ip < len(lexer.source) && is_digit(lexer.source[lexer._ip]) {
		advance(lexer)

		// Decimal place found, we can assume floating point
		if lexer.source[lexer._ip] == '.' {
			if has_floating_point {
				// We already have a floating point and we found another
				info.log(info.Log_Level_Flag.Error, "Floating point number found a second decimal place", lexer._line, lexer._col)
				return make_token(lexer, .Eof, "Error"), .LexerErr
			}

			// Set the numeric type to floating point
			has_floating_point = true
			kind = .Float_Lit
			
			advance(lexer)
		}
	}

	return make_token(lexer, kind, string(lexer.source[start_idx:lexer._ip])), .Ok
}

@(private)
make_single :: proc(lexer : ^Lexer, kind : Token_Type) -> (^Token, misc.Eso_Status) {
	// Check for end of file
	if lexer._ip + 1 > len(lexer.source) {
		info.log(info.Log_Level_Flag.Error, "Unexpected end of file", lexer._line, lexer._col)
		return make_token(lexer, .Eof, "Error"), .LexerErr
	}

	start_idx := lexer._ip
	advance(lexer)
	return make_token(lexer, kind, string(lexer.source[start_idx:lexer._ip])), .Ok
}

@(private)
make_double :: proc(lexer : ^Lexer, kind : Token_Type) -> (^Token, misc.Eso_Status) {
	// Check for end of file
	if lexer._ip + 2 > len(lexer.source) {
		info.log(info.Log_Level_Flag.Error, "Unexpected end of file", lexer._line, lexer._col)
		return make_token(lexer, .Eof, "Error"), .LexerErr
	}

	start_idx := lexer._ip
	advance(lexer)
	advance(lexer)
	return make_token(lexer, kind, string(lexer.source[start_idx:lexer._ip])), .Ok
}

@(private)
peek :: proc(lexer : ^Lexer) -> u8 {
	if lexer._ip + 1 >= len(lexer.source) do return '\e'
	return lexer.source[lexer._ip + 1]
}


get_token :: proc(lexer : ^Lexer) -> (^Token, misc.Eso_Status) {
	if lexer._ip < len(lexer.source) {
		// Skip whitespace first
		skip_whitespace(lexer)

		// Sanity check
		if lexer._ip >= len(lexer.source) {
			// Return an EOF token
			return make_token(lexer, .Eof, "EOF"), .Ok
		}

		// Create an Identifier
		if is_alpha(lexer.source[lexer._ip]) {
			return make_identifier(lexer), .Ok
		}

		// Create a Numeric value
		if is_digit(lexer.source[lexer._ip]) {
			return make_numeric(lexer)
		}

		// TODO: Single token
		switch single := lexer.source[lexer._ip]; single {
			case '+':	return make_single(lexer, .Plus)
			case '-':	return make_single(lexer, .Minus)
			case '*':	return make_single(lexer, .Star)
			case '/':	return make_single(lexer, .Slash)
			case '%':	return make_single(lexer, .Modulus)

			case '(':	return make_single(lexer, .L_Paren)
			case ')':	return make_single(lexer, .R_Paren)
			case '[':	return make_single(lexer, .L_Square)
			case ']':	return make_single(lexer, .R_Square)
			case '{':	return make_single(lexer, .L_Curly)
			case '}':	return make_single(lexer, .R_Curly)

			case '$':	return make_single(lexer, .Bind)
			case '!':	return make_single(lexer, .Unbind)

			case '.':	return make_single(lexer, .Dot)
			case ',':	return make_single(lexer, .Comma)

			case ':':
				if peek(lexer) == ':' {
					return make_double(lexer, .Colon_Colon)
				} else {
					return make_single(lexer, .Colon)
				}

			case '>':
				if peek(lexer) == '=' {
					return make_double(lexer, .Greater_Eq)
				} else {
					return make_single(lexer, .Greater)
				}
			case '<':
				if peek(lexer) == '=' {
					return make_double(lexer, .Less_Eq)
				} else {
					return make_single(lexer, .Less)
				}
			case '=':
				return make_single(lexer, .Equal)
			
			case '\'':
				return make_string(lexer)

			case: // Does not match
				info.log(info.Log_Level_Flag.Error, "Unknown token found", single, lexer._line, lexer._col)
				return make_token(lexer, .Eof, "Error"), .LexerErr
		}
	}

	// Return an EOF token
	return make_token(lexer, .Eof, "EOF"), .Ok
}