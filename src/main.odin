package main

import "core:os"
import "info"
import "process"
import "misc"

main :: proc() {
	// TODO: CLI flags and assign to Run_Flags
	info.enable()

	if data, status := os.read_entire_file_from_filename("examples/experimenting.eso"); status {
		lexer := process.Lexer { 1, 1, 0, string(data) }
		info.log(info.Log_Level_Flag.Info, "Lexing")

		for {
			if token, status := process.get_token(&lexer); status != misc.Eso_Status.Ok {
				info.log(status, "Parser encountered an error")
				free(token)
				break
			} else {
				if token.kind == process.Token_Type.Eof {
					free(token)
					break
				}

				info.log(info.Log_Level_Flag.Debug, token.lexeme)
				free(token)
			}
		}

		delete(data)
	} else {
		info.log(info.Log_Level_Flag.Error, "Could not read file")
	}
	
	process.cleanup_reserved()
	info.disable()
}