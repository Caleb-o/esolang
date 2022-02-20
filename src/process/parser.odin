package process

import "core:os"
import "../shared"
import "../misc"
import "../info"

Parser :: struct {
	_env : shared.Environment,
	
	_skip_main : bool,
	_current_token : ^Token,
	_flags : misc.Cfg_Flags,

	_lexers : [dynamic]Lexer,
	_hashes : [dynamic]u32,
}

parser_init :: proc(parser: ^Parser, file_path : string, flags : misc.Cfg_Flags) -> misc.Eso_Status {
	data, was_read := os.read_entire_file_from_filename(file_path)
	defer delete(data)

	// Failed to read file
	if !was_read {
		info.log(info.Log_Level_Flag.Error, "Could not read file")
		return .PreProcessErr
	}

	parser._env = shared.Environment{}
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
	shared.env_free(&parser._env)
}