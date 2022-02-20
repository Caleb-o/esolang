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

// TODO:
// Consider making parser global, since we always have one instance
// It would save passing a pointer around

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
push_byte_u32 :: proc(parser : ^Parser, op : u32) {
	append(&parser._env.code, op)
}

@(private="file")
push_byte_op :: proc(parser : ^Parser, op : shared.Byte_Code) {
	append(&parser._env.code, u32(op))
}

// Overload
push_byte :: proc{push_byte_u32, push_byte_op}


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

@(private="file")
add_identifier :: proc(parser : ^Parser, id : string) -> u32 {
	// Check if ID is unique, fetch ID otherwise add
	if idx, ok := parser._env.id_loc[id]; ok {
		return idx
	} else {
		// Add to the IDs list
		append(&parser._env.identifiers, id)
		return u32(len(parser._env.identifiers)-1)
	}
}

// Pushes an identifier, using an existing index if the ID exists
@(private="file")
push_identifier :: proc(parser : ^Parser, id : string) {
	push_byte(parser, add_identifier(parser, id))
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
type_list :: proc(parser : ^Parser) -> misc.Eso_Status {
	for parser._current_token.kind != Token_Type.R_Paren {
		ids : [dynamic]string
		defer delete(ids)

		// Get all IDs
		for parser._current_token.kind != Token_Type.Colon {
			append(&ids, parser._current_token.lexeme)
			_ = add_identifier(parser, parser._current_token.lexeme)	

			consume(parser, Token_Type.Id)
		}

		// Consume colon
		consume(parser, Token_Type.Colon)
	}
	return .Ok
}

@(private="file")
parameter_list :: proc(parser : ^Parser) -> misc.Eso_Status {
	consume(parser, Token_Type.L_Paren) or_return
	type_list(parser)
	consume(parser, Token_Type.R_Paren) or_return
	return .Ok
}

@(private="file")
procedure_def :: proc(parser : ^Parser) -> misc.Eso_Status {
	consume(parser, Token_Type.Proc) or_return
	
	proc_id := parser._current_token.lexeme
	consume(parser, Token_Type.Id) or_return

	// Push call op and identifier
	push_byte(parser, shared.Byte_Code.Proc_Call)
	push_identifier(parser, proc_id)

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