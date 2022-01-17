import os
from Token import *
from bytecode import ByteCode
from lexer import Lexer
from dataclasses import dataclass
from enum import IntEnum


@dataclass
class Scope:
    depth: int
    stack: list[int]


@dataclass
class Environment:
    scopes: list[Scope]
    constants: list[int]
    strings: list[str]
    byte_code: list[ByteCode]


# Type of space the parser is within
# eg. We're parsing within a macro
class SpaceType(IntEnum):
    GLOBAL  = iota(True)
    PROC    = iota()
    MACRO   = iota()


class Parser:
    def __init__(self, file_name: str, source: str) -> None:
        self.file_name = file_name
        self.lexer = Lexer(file_name, source)
        self.cur_token = self.lexer.get_next()
        self.env: Environment = Environment([], [], [], [])

        self.is_loop = False
        # Better handling of spaces (eg. Global, Macro or Proc)
        self.current_space: list[tuple[str, SpaceType]] = [ ('global', SpaceType.GLOBAL) ]
        self.current_space_code: dict[tuple[str, SpaceType], list[ByteCode]] = { ('global', SpaceType.GLOBAL): [] }


    def set_source(self, source: str):
        self.lexer.set_source(source)
        self.cur_token = self.lexer.get_next()


    def error_msg(self, msg: str):
        raise Exception(f'[\'{self.file_name}\'][Parser] {msg} on line {self.cur_token.line} at pos {self.cur_token.column}')

    def get_next(self):
        self.cur_token = self.lexer.get_next()


    def push_byte(self, byte: ByteCode):
        self.current_space_code[self.current_space[-1]].append(byte)
    
    
    def push_bytes(self, bytes: list[ByteCode]):
        self.current_space_code[self.current_space[-1]].extend(bytes)

    
    def get_current_code_op(self):
        return self.current_space_code[self.current_space[-1]][-1]
    
    def get_code_op_from(self, idx: int):
        return self.current_space_code[self.current_space[-1]][idx]

    
    def set_code_op_at(self, idx: int, op: ByteCode):
        self.current_space_code[self.current_space[-1]][idx] = op
    

    def get_current_code_loc(self):
        return len(self.current_space_code[self.current_space[-1]])


    def insert_byte_at(self, idx: int, byte: ByteCode):
        self.current_space_code[self.current_space[-1]].insert(idx, byte)


    def remove_bytes_from(self, idx: int):
        i = idx

        while i < self.get_current_code_loc():
            self.current_space_code[self.current_space[-1]].remove(i)
            i += 1

    
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
                self.push_byte(ByteCode.OP_ADD)
            
            elif self.cur_token.ttype == TokenType.MINUS:
                self.push_byte(ByteCode.OP_SUB)

            elif self.cur_token.ttype == TokenType.STAR:
                self.push_byte(ByteCode.OP_MUL)

            elif self.cur_token.ttype == TokenType.SLASH:
                self.push_byte(ByteCode.OP_DIV)
            
            self.consume(self.cur_token.ttype)
    

    def comparison(self):
        while self.cur_token.ttype in ( TokenType.LESS_THAN, TokenType.GREATER_THAN, TokenType.EQUAL_TO ):
            if self.cur_token.ttype == TokenType.LESS_THAN:
                self.push_byte(ByteCode.OP_LESS)
            
            elif self.cur_token.ttype == TokenType.GREATER_THAN:
                self.push_byte(ByteCode.OP_GREATER)

            elif self.cur_token.ttype == TokenType.EQUAL_TO:
                self.push_byte(ByteCode.OP_EQUAL_TO)

            self.consume(self.cur_token.ttype)
    

    def expr(self):
        if self.cur_token.ttype == TokenType.INT:
            value = int(self.cur_token.lexeme)
            value_idx = -1

            self.consume(self.cur_token.ttype)

            # Check if value exists in table
            if value in self.env.constants:
                # Fetch index of value
                value_idx = self.env.constants.index(value)
            else:
                # New value
                self.env.constants.append(value)
                value_idx = len(self.env.constants) - 1


            self.push_bytes([ ByteCode.OP_PUSH, value_idx ])

        elif self.cur_token.ttype == TokenType.STR:
            value = self.cur_token.lexeme
            value_idx = -1

            self.consume(self.cur_token.ttype)

            # Check if value exists in table
            if value in self.env.strings:
                # Fetch index of value
                value_idx = self.env.strings.index(value)
            else:
                # New value
                self.env.strings.append(value)
                value_idx = len(self.env.strings) - 1


            self.push_bytes([ ByteCode.OP_STR, value_idx ])

        elif self.cur_token.ttype in ( TokenType.PLUS, TokenType.MINUS, TokenType.STAR, TokenType.SLASH ):
            self.arithmetic()
        
        elif self.cur_token.ttype in ( TokenType.LESS_THAN, TokenType.GREATER_THAN, TokenType.EQUAL_TO ):
            self.comparison()

        elif self.cur_token.ttype == TokenType.DUPLICATE:
            self.consume(self.cur_token.ttype)
            self.push_byte(ByteCode.OP_DUPLICATE)
        elif self.cur_token.ttype == TokenType.POP:
            self.consume(self.cur_token.ttype)
            self.push_byte(ByteCode.OP_POP)

    
    def proc_decl(self):
        proc_name = self.cur_token.lexeme
        self.consume(TokenType.ID)
        arg_count = int(self.cur_token.lexeme)
        self.consume(TokenType.INT)
        return_count = -1

        if self.cur_token.ttype == TokenType.INT:
            return_count = int(self.cur_token.lexeme)
            self.consume(TokenType.INT)
        
        self.consume(TokenType.SEMICOLON)

        if (proc_name, SpaceType.PROC) in self.current_space:
            self.error_msg(f'\'{proc_name}\' has already been defined')

        # Define proc key
        proc_key = (proc_name, SpaceType.PROC)
        self.current_space.append(proc_key)
        self.current_space_code[proc_key] = []
        
        # Append argc and return count
        self.push_bytes([ arg_count, return_count ])

        while self.cur_token.ttype != TokenType.END:
            self.statment()
        
        self.consume(TokenType.END)
        self.push_byte(ByteCode.OP_RETURN)
        self.current_space.pop()


    def macro_decl(self):
        macro_name = self.cur_token.lexeme
        self.consume(TokenType.ID)
        self.consume(TokenType.SEMICOLON)

        if (macro_name, SpaceType.MACRO) in self.current_space_code:
            self.error_msg(f'Macro \'{macro_name}\' has already been defined')

        # Define macro space
        macro_key = (macro_name, SpaceType.MACRO)
        self.current_space.append(macro_key)
        self.current_space_code[macro_key] = []

        
        while self.cur_token.ttype != TokenType.END:
            self.statment()
        
        self.consume(TokenType.END)
        self.current_space.pop()

    
    # Re-evaluate start position on the loop_end bytecode
    # This needs to occur when we expand procs and macros into the main code
    def reevaluate_loop(self):
        ip = 0
        loop_start_ip = [ ]

        while ip < len(self.env.byte_code):
            operation = self.get_code_op_from(ip)

            if operation in [ ByteCode.OP_PUSH, ByteCode.OP_IF ]:
                ip += 1
            elif operation in [ ByteCode.OP_PROC_CALL ]:
                ip += 2
            elif operation == ByteCode.OP_LOOP_START:
                loop_start_ip.append(ip)
            elif operation == ByteCode.OP_LOOP_END:
                if len(loop_start_ip) > 0:
                    self.env.byte_code[ip + 1] = loop_start_ip.pop()
                
            ip += 1


    def reevaluate_break(self):
        ip = 0
        loop_break_ip = [ ]
        depth = 0

        while ip < len(self.env.byte_code):
            operation = self.get_code_op_from(ip)

            if operation in [ ByteCode.OP_PUSH, ByteCode.OP_IF ]:
                ip += 1

            elif operation in [ ByteCode.OP_PROC_CALL ]:
                ip += 2

            elif operation == ByteCode.OP_BREAK:
                ip += 1
                loop_break_ip.append(ip)

            elif operation == ByteCode.OP_LOOP_START:
                depth += 1

            elif operation == ByteCode.OP_LOOP_END:
                ip += 1
                depth -= 1

                if depth == 0:
                    while len(loop_break_ip) > 0:
                        self.env.byte_code[loop_break_ip.pop()] = ip
                

            ip += 1
        
        assert len(loop_break_ip) == 0, 'Undetermined loop break'


    def statment(self):
        if self.cur_token.ttype == TokenType.NEGATE:
            self.consume(self.cur_token.ttype)

            if self.cur_token.ttype == TokenType.INT:
                self.consume(self.cur_token.ttype)
                self.expr()
            else:
                self.error_msg('Cannot negate non int')
            
        elif self.cur_token.ttype == TokenType.BREAK:
            if not self.is_loop:
                self.error_msg('Cannot use break outside of a loop')

            self.consume(self.cur_token.ttype)
            self.push_bytes([ ByteCode.OP_BREAK, -1 ])

        elif self.cur_token.ttype == TokenType.IF:
            # If condition
            self.consume(self.cur_token.ttype)
            self.push_bytes([ ByteCode.OP_IF, -1 ])
            if_next = self.get_current_code_loc() - 1

            while self.cur_token.ttype != TokenType.END:
                self.statment()

            self.consume(TokenType.END)

            # Insert end of block if condition is false
            block_loc = self.get_current_code_loc() - 1
            self.set_code_op_at(if_next, block_loc)
        
        elif self.cur_token.ttype == TokenType.BANG:
            # Macro call
            self.consume(self.cur_token.ttype)
            macro_name = self.cur_token.lexeme
            self.consume(TokenType.ID)

            if len(self.current_space) > 0:
                mac_name, mac_space = self.current_space[-1]

                if mac_name == macro_name and mac_space == SpaceType.MACRO:
                    self.error_msg(f'Cannot recursively call macro \'{macro_name}\'')

            try:
                self.push_bytes(self.current_space_code[(macro_name, SpaceType.MACRO)])
            except:
                self.error_msg(f'Macro is not defined \'{macro_name}\'')

        elif self.cur_token.ttype == TokenType.ID:
            # Proc call
            proc_name = self.cur_token.lexeme
            self.consume(self.cur_token.ttype)

            self.push_byte(ByteCode.OP_PROC_CALL)

            if len(self.current_space) > 0:
                procedure_name, proc_space = self.current_space[-1]

                # Check if procedure has been 
                if procedure_name == proc_name and proc_space == SpaceType.PROC:
                    self.error_msg(f'Cannot recursively call procedure \'{proc_name}\'')

            try:
                self.push_bytes(self.current_space_code[(proc_name, SpaceType.PROC)])
            except:
                self.error_msg(f'Proc is not defined \'{proc_name}\'')


        elif self.cur_token.ttype == TokenType.UNDEF:
            self.consume(self.cur_token.ttype)
            macro_name = self.cur_token.lexeme
            self.consume(TokenType.ID)

            if (macro_name, SpaceType.MACRO) in self.current_space:
                del self.current_space[(macro_name, SpaceType.MACRO)]
            else:
                self.error_msg(f'Macro \'{macro_name}\' is not defined or might have been undefined previously {self.cur_token.line}')
        
        elif self.cur_token.ttype == TokenType.PROC:
            self.consume(self.cur_token.ttype)
            self.proc_decl()

        elif self.cur_token.ttype == TokenType.MACRO:
            # Macro definition
            self.consume(self.cur_token.ttype)
            self.macro_decl()

        elif self.cur_token.ttype == TokenType.LSQUARE:
            # Loop
            self.consume(self.cur_token.ttype)
            self.push_bytes([ByteCode.OP_LOOP_START])

            loop_start = self.get_current_code_loc()
            self.is_loop = True
            
            while self.cur_token.ttype != TokenType.RSQUARE:
                self.statment()

            loop_end = self.get_current_code_loc()
            self.is_loop = False

            # Check if empty
            if loop_start == loop_end:
                # Remove beginning
                self.current_space_code[self.current_space[-1]].pop()
            else:
                # Push the start index of the loop for if we need to go back
                self.push_bytes([ByteCode.OP_LOOP_END, loop_start])
            
            self.consume(TokenType.RSQUARE)
        
        elif self.cur_token.ttype == TokenType.SWAP:
            # Swap top 2 items on the stack
            self.consume(self.cur_token.ttype)
            self.push_byte(ByteCode.OP_SWAP)
        
        elif self.cur_token.ttype == TokenType.DOT:
            # Print the last item on the stack
            self.consume(self.cur_token.ttype)
            self.push_byte(ByteCode.OP_PRINT)

        elif self.cur_token.ttype == TokenType.QMARK:
            # Ask the user for something and put it on the stack
            self.consume(self.cur_token.ttype)
            self.push_byte(ByteCode.OP_INPUT)

        elif self.cur_token.ttype == TokenType.COMMA:
            # Print the last item on the stack
            self.consume(self.cur_token.ttype)
            self.push_byte(ByteCode.OP_PRINT_CHAR)

        else:
            # Evaluate an expression if no statement matches
            self.expr()
        
        # Consume a semicolon, if it exists
        self.optional_consume(TokenType.SEMICOLON)


    def program(self):
        self.env.scopes.append(Scope(0, []))

        included: list[str] = []

        # Impl can only be at the top of the file
        while self.cur_token.ttype == TokenType.IMPL:
            self.consume(TokenType.IMPL)
            import_name = self.cur_token.lexeme.replace('.', '/')

            # Add extension if not there
            if '.eso' not in import_name:
                import_name += '.eso'

            self.consume(TokenType.STR)

            # Don't import a file that has already been included
            # Todo: Make this global so we don't include them within other files
            if import_name in included:
                continue

            # Add to included
            included.append(import_name)

            if os.path.isfile(import_name):
                try:
                    impl_parser = Parser(import_name, open(import_name).read())
                    env = impl_parser.parse()

                    # Add environment to local
                    self.env.byte_code.extend(env.byte_code)
                    self.env.constants.extend(env.constants)

                    # Import macros/procs
                    self.current_space_code |= impl_parser.current_space_code
                except Exception as e:
                    self.error_msg(f'[{import_name}]: {e}')
            else:
                self.error_msg(f'file does not exist :: "{import_name}"')

        while self.cur_token.ttype != TokenType.EOF:
            self.statment()

    
    def parse(self) -> Environment:
        self.program()
        
        self.env.byte_code.extend(self.current_space_code[('global', SpaceType.GLOBAL)])
        # Re-evaluate all loops
        self.reevaluate_loop()
        self.reevaluate_break()
        return self.env