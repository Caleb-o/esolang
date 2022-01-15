import os
from pickletools import optimize
from Token import *
from bytecode import ByteCode
from lexer import Lexer
from dataclasses import dataclass


@dataclass
class Scope:
    depth: int
    stack: list[int]


@dataclass
class Environment:
    scopes: list[Scope]
    contants: list[int]
    byte_code: list[ByteCode]


class Parser:
    def __init__(self, file_name: str, source: str) -> None:
        self.file_name = file_name
        self.lexer = Lexer(file_name, source)
        self.get_next()
        self.env: Environment = Environment([], [], [])
        self.macros: dict[str, list[ByteCode]] = {}
        self.current_macro = None
    

    def set_source(self, source: str):
        self.lexer.set_source(source)
        self.get_next()


    def error_msg(self, msg: str):
        raise Exception(f'[\'{self.file_name}\'][Parser] {msg} on line {self.cur_token.line} at pos {self.cur_token.column}')

    def get_next(self):
        self.lexer.get_next()
        self.cur_token = self.lexer.cur_tok
    

    def push_code(self, byte: ByteCode):
        if self.current_macro != None:
            self.macros[self.current_macro].append(byte)
        else:
            self.env.byte_code.append(byte)

    
    def push_block(self, bytes: list[ByteCode]):
        if self.current_macro != None:
            self.macros[self.current_macro].extend(bytes)
        else:
            self.env.byte_code.extend(bytes)

    
    # Consume token of type, otherwise, error
    def consume(self, ttype: TokenType):
        if self.cur_token.ttype == ttype:
            self.get_next()
        else:
            self.error_msg(f'Expected type {ttype} but got {self.cur_token.ttype}')
    

    # If a token does exist, consume it. Does not matter if it doesn't exist.
    def optional_consume(self, ttype: TokenType):
        if self.cur_token.ttype == ttype:
            self.get_next()


    def arithmetic(self):
        while self.cur_token.ttype in ( TokenType.PLUS, TokenType.MINUS, TokenType.STAR, TokenType.SLASH ):
            if self.cur_token.ttype == TokenType.PLUS:
                self.push_code(ByteCode.OP_ADD)
            
            elif self.cur_token.ttype == TokenType.MINUS:
                self.push_code(ByteCode.OP_SUB)

            elif self.cur_token.ttype == TokenType.STAR:
                self.push_code(ByteCode.OP_MUL)

            elif self.cur_token.ttype == TokenType.SLASH:
                self.push_code(ByteCode.OP_DIV)
            
            self.consume(self.cur_token.ttype)
    

    def expr(self):
        if self.cur_token.ttype == TokenType.INT:
            self.env.contants.append(int(self.cur_token.lexeme))
            self.consume(self.cur_token.ttype)

            self.push_block([ ByteCode.OP_PUSH, len(self.env.contants) - 1 ])

        elif self.cur_token.ttype in ( TokenType.PLUS, TokenType.MINUS, TokenType.STAR, TokenType.SLASH ):
            self.arithmetic()

        elif self.cur_token.ttype == TokenType.DUPLICATE:
            self.consume(self.cur_token.ttype)
            self.push_code(ByteCode.OP_DUPLICATE)
        elif self.cur_token.ttype == TokenType.POP:
            self.consume(self.cur_token.ttype)
            self.push_code(ByteCode.OP_POP)


    def statment(self):
        if self.cur_token.ttype == TokenType.ID:
            # ID is currently a call to a macro
            # TODO: proc call check
            macro_name = self.cur_token.lexeme
            self.consume(TokenType.ID)

            try:
                self.push_block(self.macros[macro_name])
            except:
                self.error_msg(f'Macro does not exist \'{macro_name}\'')

        elif self.cur_token.ttype == TokenType.UNDEF:
            self.consume(TokenType.UNDEF)
            macro_name = self.cur_token.lexeme
            self.consume(TokenType.ID)

            if macro_name in self.macros:
                del self.macros[macro_name]
            else:
                self.error_msg(f'Macro \'{macro_name}\' does not exist or might have been undefined previously {self.cur_token.line}')

        elif self.cur_token.ttype == TokenType.MACRO:
            # Macro definition
            self.consume(TokenType.MACRO)
            macro_name = self.cur_token.lexeme
            self.consume(TokenType.ID)
            self.consume(TokenType.SEMICOLON)

            if macro_name in self.macros:
                self.error_msg(f'Macro \'{macro_name}\' has already been defined at {self.cur_token.line}')
            else:
                self.current_macro = macro_name
                self.macros[macro_name] = []
            
            while self.cur_token.ttype != TokenType.END:
                self.statment()
            
            self.consume(TokenType.END)
            self.current_macro = None
        elif self.cur_token.ttype == TokenType.LSQUARE:
            # Loop
            self.consume(self.cur_token.ttype)
            self.push_code(ByteCode.OP_LOOP_START)

            loop_start = len(self.env.byte_code) - 1 if self.current_macro == None else len(self.macros[self.current_macro])
            
            while self.cur_token.ttype != TokenType.RSQUARE:
                self.statment()

            # Push the start index of the loop for if we need to go back
            self.consume(TokenType.RSQUARE)
            self.push_block([ByteCode.OP_LOOP_END, loop_start])
        
        elif self.cur_token.ttype == TokenType.SWAP:
            # Swap top 2 items on the stack
            self.consume(self.cur_token.ttype)
            self.push_code(ByteCode.OP_SWAP)
        
        elif self.cur_token.ttype == TokenType.DOT:
            # Print the last item on the stack
            self.consume(self.cur_token.ttype)
            self.push_code(ByteCode.OP_PRINT)
        else:
            # Evaluate an expression if no statement matches
            self.expr()
        
        # Consume a semicolon, if it exists
        self.optional_consume(TokenType.SEMICOLON)


    def program(self):
        self.env.scopes.append(Scope(0, []))

        # Impl can only be at the top of the file
        while self.cur_token.ttype == TokenType.IMPL:
            self.consume(TokenType.IMPL)
            import_name = self.cur_token.lexeme.replace('.', '/')
            self.consume(TokenType.STR)

            if os.path.isfile(import_name):
                try:
                    impl_parser = Parser(import_name, open(import_name).read())
                    env = impl_parser.parse()

                    print(f'file loaded :: {import_name}')
                    
                    # Add environment to local
                    self.env.byte_code.extend(env.byte_code)
                    self.env.contants.extend(env.contants)

                    self.macros |= impl_parser.macros
                except Exception as e:
                    self.error_msg(f'[{import_name}]: {e}')
            else:
                self.error_msg(f'file does not exist :: "{import_name}"')

        while self.cur_token.ttype != TokenType.EOF:
            self.statment()

    
    def parse(self) -> Environment:
        self.program()
        return self.env