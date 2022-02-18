package main

import "info"
import "process"
import "misc"

main :: proc() {
	info.enable()

	lexer := process.Lexer { 1, 1, 0, "100.0" }
	info.log(info.Log_Level_Flag.Info, "Parsing")

	if token, status := process.get_token(&lexer); status != misc.Eso_Error.Ok {
		info.log(info.Log_Level_Flag.Error, "Parser encountered an error")
		free(token)
	} else {
		info.log(info.Log_Level_Flag.Debug, token.lexeme)
		free(token)
	}
	
	process.cleanup_reserved()
	info.disable()
}