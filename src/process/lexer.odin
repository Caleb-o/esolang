package process

import "core:fmt"
import "core:strings"
import "../misc"
import "../info"


Lexer :: struct {
	_line, _col, _ip : int,
	source : string,
}

// char peek(size_t);

// std::shared_ptr<Token> make_string();
// std::shared_ptr<Token> make_single(TokenKind);

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
				info.log(info.Log_Level_Flag.Debug, "whitespace")
			
			case: // None of the above
				break outer
		}
	}
}

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
		}
	}

	return make_token(lexer, kind, string(lexer.source[start_idx:lexer._ip])), .Ok
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
			case: // Does not match
				info.log(info.Log_Level_Flag.Error, "Unknown token found", single, lexer._line, lexer._col)
				return make_token(lexer, .Eof, "Error"), .LexerErr
		}
	}

	// Return an EOF token
	return make_token(lexer, .Eof, "EOF"), .Ok
}