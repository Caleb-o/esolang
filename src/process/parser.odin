package process

import "core:os"
import "core:strconv"

import "../shared"
import "../misc"
import "../info"

Parser :: struct {
	_env : ^shared.Environment,
	
	_skip_main : bool,
	_current_token : ^Token,
	_flags : misc.Cfg_Flags,

	_lexers : [dynamic]Lexer,
	_hashes : [dynamic]u16,

	// Temporary tracking to make life a little easier
	_ids : map[string]u16,
	_proc_ids : map[string]u16,

	// -- Procedure bound
	_params : map[string]shared.ValueFlag,
	_returns : [dynamic]shared.ValueFlag,
}


@(private="file")
PARSER := Parser{}


parser_init :: proc(file_path : string, flags : misc.Cfg_Flags) -> misc.Eso_Status {
	data, was_read := os.read_entire_file_from_filename(file_path)

	// Failed to read file
	if !was_read {
		info.log(info.Log_Level_Flag.Error, "Could not read file")
		return .PreProcessErr
	}

	PARSER._env = new(shared.Environment)
	PARSER._flags = flags
	PARSER._skip_main = false
	PARSER._proc_ids = make(map[string]u16)
	
	// Append to lexers and hashes
	source := string(data)

	append(&PARSER._hashes, shared.util_hash(source))
	append(&PARSER._lexers, Lexer{ 1, 1, 0, source })

	status : misc.Eso_Status
	PARSER._current_token, status = get_token(&PARSER._lexers[len(PARSER._lexers)-1])

	return status
}


parser_cleanup :: proc() {
	free(PARSER._current_token)
	lexer_cleanup(&PARSER._lexers[0])
	delete(PARSER._lexers)
	delete(PARSER._hashes)
	delete(PARSER._ids)
	delete(PARSER._proc_ids)
}

@(private="file")
push_byte_u32 :: proc(op : u16) {
	append(&PARSER._env.code, op)
}

@(private="file")
push_byte_op :: proc(op : shared.Byte_Code) {
	append(&PARSER._env.code, u16(op))
}

// Overload
push_byte :: proc{push_byte_u32, push_byte_op}


@(private="file")
code_idx :: proc() -> u16 {
	return u16(len(PARSER._env.code)-1)
}


@(private="file")
top_lexer :: proc() -> ^Lexer {
	return &PARSER._lexers[len(PARSER._lexers)-1]
}

@(private="file")
top_procedure :: proc() -> ^shared.Procedure_Def {
	return &PARSER._env.defs.procedures[len(PARSER._env.defs.procedures)-1]
}

@(private="file")
invalid_token :: proc() -> misc.Eso_Status {
	lexer := top_lexer()
	info.log(
		info.Log_Level_Flag.Error,
		"Unknown token found in top-level '%s' of type %s",
		lexer._line, lexer._col,
		PARSER._current_token.lexeme, PARSER._current_token.kind,
	)
	
	return misc.Eso_Status.ParserErr
}

@(private="file")
add_identifier :: proc(id : string) -> u16 {
	if idx, ok := PARSER._ids[id]; ok {
		// Return existing index
		return u16(idx)
	} else {
		// Add a new identifier and cache its index for later
		append(&PARSER._env.identifiers, id)
		iidx := u16(len(PARSER._env.identifiers)-1)
		PARSER._ids[id] = iidx
		return iidx
	}
}

// Pushes an identifier, using an existing index if the ID exists
@(private="file")
push_identifier :: proc(id : string) {
	push_byte(add_identifier(id))
}

@(private="file")
push_value :: proc() -> misc.Eso_Status {
	using shared

	push_byte(Byte_Code.Push)

	#partial switch PARSER._current_token.kind {
		case Token_Type.Int_Lit:
			value := strconv.atoi(PARSER._current_token.lexeme)
			append(&PARSER._env.values, Value{ value, shared.ValueFlag.Integer })

		case Token_Type.Float_Lit:
			value := f32(strconv.atof(PARSER._current_token.lexeme))
			append(&PARSER._env.values, Value{ value , shared.ValueFlag.Float })

		case Token_Type.Bool_Lit:
			if value, ok := strconv.parse_bool(PARSER._current_token.lexeme); ok {
				append(&PARSER._env.values, Value{ value, shared.ValueFlag.Boolean })
			} else {
				info.log(info.Log_Level_Flag.Error, "Could not pasrse boolean value")
				return .ParserErr
			}

		case Token_Type.String_Lit:
			append(&PARSER._env.values, Value{ PARSER._current_token.lexeme, shared.ValueFlag.String })
	}

	push_byte(u16(len(PARSER._env.values)-1))
	return .Ok
}


@(private = "file")
consume :: proc(expected : Token_Type) -> misc.Eso_Status {
	if PARSER._current_token.kind == expected {
		free(PARSER._current_token)
		status : misc.Eso_Status
		PARSER._current_token, status = get_token(top_lexer())
		return status
	} else {
		return invalid_token()
	}
}

@(private="file")
expr :: proc() -> misc.Eso_Status {
	#partial switch PARSER._current_token.kind {
		case Token_Type.Int_Lit: fallthrough
		case Token_Type.Float_Lit: fallthrough
		case Token_Type.Bool_Lit: fallthrough
		case Token_Type.String_Lit:
			status := push_value()
			consume(PARSER._current_token.kind)
			return status

		case: // Default
			info.log(info.Log_Level_Flag.Error, "Unknown token found")
			return .ParserErr
	}

	return .Ok
}

@(private="file")
statement :: proc() -> misc.Eso_Status {
	using shared

	#partial switch PARSER._current_token.kind {
		case Token_Type.Print:			push_byte(Byte_Code.Print)
		case Token_Type.Print_Ln: 		push_byte(Byte_Code.Println)
		case Token_Type.Swap: 			push_byte(Byte_Code.Swap)
		case Token_Type.Drop: 			push_byte(Byte_Code.Drop)

		case:			return expr()
	}
	consume(PARSER._current_token.kind) or_return

	return .Ok
}

@(private="file")
statement_list :: proc() -> misc.Eso_Status {
	for PARSER._current_token.kind != Token_Type.R_Curly {
		statement() or_return
	}

	return .Ok
}

@(private="file")
code_block :: proc() -> misc.Eso_Status {
	consume(Token_Type.L_Curly) or_return
	statement_list() or_return
	consume(Token_Type.R_Curly) or_return

	return .Ok
}

@(private="file")
type_list :: proc() -> misc.Eso_Status {
	using shared
	
	for PARSER._current_token.kind != Token_Type.R_Paren {
		ids : [dynamic]string
		identifier_idxs : [dynamic]u16
		defer {
			delete(ids)
			delete(identifier_idxs)
		}

		// Get all IDs
		for PARSER._current_token.kind != Token_Type.Colon {
			append(&ids, PARSER._current_token.lexeme)
			append(&identifier_idxs, add_identifier(PARSER._current_token.lexeme))

			consume(Token_Type.Id) or_return

			// Parse a comma
			if PARSER._current_token.kind == Token_Type.Comma {
				consume(Token_Type.Comma) or_return

				if PARSER._current_token.kind == Token_Type.L_Curly {
					info.log(info.Log_Level_Flag.Error, "Trailing comma after parameter ID not allowed")
					return .ParserErr
				}
			}
		}

		// Consume colon
		consume(Token_Type.Colon) or_return

		// Get the current type
		type_str := PARSER._current_token.lexeme
		consume(Token_Type.Type_Id) or_return

		type_flag := shared.str_to_vflag(type_str)

		// Apply flag to all current IDs
		proc_def := top_procedure()

		for param_idx := 0; param_idx < len(ids); param_idx += 1 {
			push_byte(Byte_Code.Bind)
			push_byte(2) // Param
			push_byte(identifier_idxs[param_idx])
			push_byte(u16(param_idx))

			append(&proc_def.params, type_flag)
			PARSER._params[ids[param_idx]] = type_flag
		}
	}
	return .Ok
}

@(private="file")
parameter_list :: proc() -> misc.Eso_Status {
	consume(Token_Type.L_Paren) or_return
	type_list() or_return
	consume(Token_Type.R_Paren) or_return
	return .Ok
}

@(private="file")
procedure_def :: proc() -> misc.Eso_Status {
	using shared
	consume(Token_Type.Proc) or_return
	
	proc_id := PARSER._current_token.lexeme
	consume(Token_Type.Id) or_return

	if _, ok := PARSER._proc_ids[proc_id]; ok {
		// Check for duplicate main
		if proc_id == "main" {
			info.log(info.Log_Level_Flag.Error, "Cannot redefine the main procedure")
			return .ParserErr
		}
	} else {
		// Dummy
		PARSER._proc_ids[proc_id] = 0
	}

	// Push call op and identifier
	add_identifier(proc_id)
	append(&PARSER._env.defs.procedures, shared.Procedure_Def { 
		code_idx(),
		make([dynamic]shared.ValueFlag, 0),
		make([dynamic]shared.ValueFlag, 0),
	})

	// Setup for temporary data
	PARSER._params = make(map[string]shared.ValueFlag)
	PARSER._returns = make([dynamic]shared.ValueFlag)

	defer {
		// Cleanup temporary data
		delete(PARSER._params)
		delete(PARSER._returns)
	}

	// Parse the parameters
	parameter_list() or_return
	consume(Token_Type.Colon_Colon) or_return

	proc_def := top_procedure()
	// Parse return types
	for PARSER._current_token.kind != Token_Type.L_Curly {
		append(&proc_def.returns, shared.str_to_vflag(PARSER._current_token.lexeme))
		consume(Token_Type.Type_Id) or_return

		// Parse a comma
		if PARSER._current_token.kind == Token_Type.Comma {
			consume(Token_Type.Comma) or_return

			if PARSER._current_token.kind == Token_Type.L_Curly {
				info.log(info.Log_Level_Flag.Error, "Trailing comma after return type not allowed")
				return .ParserErr
			}
		}
	}

	// Parse the body
	code_block() or_return

	if proc_id == "main" {
		push_byte(Byte_Code.Halt)
	} else {
		push_byte(Byte_Code.Return)
		push_byte(u16(len(PARSER._env.defs.procedures)))
	}

	return .Ok
}

@(private="file")
program :: proc() -> misc.Eso_Status {
	for PARSER._current_token.kind != Token_Type.Eof {
		#partial switch PARSER._current_token.kind {
			case Token_Type.Proc:		procedure_def() or_return
			case: // Default
				return invalid_token()
		}
	}

	return .Ok
}

parse :: proc() -> ^shared.Environment {
	// An error occured
	if program() != misc.Eso_Status.Ok {
		info.log(info.Log_Level_Flag.Error, "Failed to parse")
	}
	return PARSER._env
}