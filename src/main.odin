package main

import "info"
import "process"

main :: proc() {
	info.enable()

	using info.Log_Level_Flag

	lexer := process.Lexer { 1, 1, 0, "Swap" }

	token := process.get_token(&lexer)
	info.log(Debug, token.lexeme)
	free(token)
	
	process.cleanup()
	info.disable()
}