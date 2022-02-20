package process

import "core:os"
import "core:fmt"
import "../shared"
import "../misc"
import "../info"

Parser :: struct {
	_env : ^shared.Environment,
	
	_skip_main : bool,
	_current_token : ^Token,
	_flags : misc.Cfg_Flags,

	_lexers : [dynamic]Lexer,
	_hashes : [dynamic]u32,
}

parser_init :: proc(parser: ^Parser, file_path : string, flags : misc.Cfg_Flags) -> misc.Eso_Status {
	data, was_read := os.read_entire_file_from_filename(file_path)

	// Failed to read file
	if !was_read {
		info.log(info.Log_Level_Flag.Error, "Could not read file")
		return .PreProcessErr
	}

	parser._env = new(shared.Environment)
	parser._flags = flags
	parser._skip_main = false
	
	// Append to lexers and hashes
	source := string(data)

	append(&parser._hashes, shared.util_hash(source))
	append(&parser._lexers, Lexer{ 1, 1, 0, source })

	status : misc.Eso_Status
	parser._current_token, status = get_token(&parser._lexers[len(parser._lexers)-1])

	return status
}


parser_cleanup :: proc(parser : ^Parser) {
	free(parser._current_token)
	lexer_cleanup(&parser._lexers[0])
	delete(parser._lexers)
	delete(parser._hashes)
}

@(private="file")
top_lexer :: proc(parser : ^Parser) -> ^Lexer {
	return &parser._lexers[len(parser._lexers)-1]
}

@(private="file")
invalid_token :: proc(parser : ^Parser) -> misc.Eso_Status {
	formatted := fmt.aprintf("Unknown token found in top-level '%s' of type %s", parser._current_token.lexeme, parser._current_token.kind)
			
	lexer := top_lexer(parser)
	info.log(info.Log_Level_Flag.Error, formatted, lexer._line, lexer._col)
	
	delete(formatted)
	return misc.Eso_Status.ParserErr
}


@(private = "file")
consume :: proc(parser : ^Parser, expected : Token_Type) -> misc.Eso_Status {
	if parser._current_token.kind == expected {
		free(parser._current_token)
		status : misc.Eso_Status
		parser._current_token, status = get_token(top_lexer(parser))
		return status
	} else {
		return invalid_token(parser)
	}
}

@(private="file")
procedure_def :: proc(parser : ^Parser) -> misc.Eso_Status {
	consume(parser, Token_Type.Proc) or_return

	return .Ok
}

@(private="file")
program :: proc(parser : ^Parser) -> misc.Eso_Status {
	#partial switch parser._current_token.kind {
		case Token_Type.Proc:		return procedure_def(parser)
		case: // Default
			return invalid_token(parser)
	}
}

parse :: proc(parser : ^Parser) -> ^shared.Environment {
	// An error occured
	if program(parser) != misc.Eso_Status.Ok {
		info.log(info.Log_Level_Flag.Error, "Failed to parse")
	}
	return parser._env
}