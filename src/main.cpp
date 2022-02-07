#include <iostream>
#include "process/lexer.hpp"

int main(int argc, char **argv) {
	Process::Lexer lexer("proc name void int string float dup swap true false using {()} :.+-*/->>=<==!");
	Process::Token *token = lexer.get_token();

	while(token->kind != Process::TokenKind::ENDOFFILE) {
		std::cout << token->kind << " " << token->lexeme << std::endl;
		delete token;
		token = lexer.get_token();
	}

	delete token;

	return 0;
}