from Token import *


SINGLE_CHARS = {
    '+' : TokenType.PLUS,
    '-' : TokenType.MINUS,
    '*' : TokenType.STAR,
    '/' : TokenType.SLASH,
    '>' : TokenType.GREATER_THAN,
    '<' : TokenType.LESS_THAN,
    '=' : TokenType.EQUAL_TO,
    ';' : TokenType.SEMICOLON,
    '.' : TokenType.DOT,
    ',' : TokenType.COMMA,
    '!' : TokenType.BANG,
    '&' : TokenType.SWAP,
    '[' : TokenType.LSQUARE,
    ']' : TokenType.RSQUARE,
}


KEYWORDS = {
    'if'    : TokenType.IF,
    'break' : TokenType.BREAK,
    'neg'   : TokenType.NEGATE,
    'impl'  : TokenType.IMPL,
    'dup'   : TokenType.DUPLICATE,
    'pop'   : TokenType.POP,
    'macro' : TokenType.MACRO,
    'undef' : TokenType.UNDEF,
    'end'   : TokenType.END,
    'proc'  : TokenType.PROC
}


class Lexer:
    def __init__(self, file_name: str, source: str) -> None:
        self.file_name = file_name
        self.source = source
        self.ip: int = 0
        self.col = 1
        self.line = 1
        self.cur_char =  source[0]


    def reset(self):
        self.ip: int = 0
        self.cur_tok = None
        self.col = 1
        self.line = 1

    
    def set_source(self, source: str):
        self.reset()
        self.source = source
        self.cur_char = source[0]


    def error_msg(self, msg: str):
        raise Exception(f'[\'{self.file_name}\'][Lexer] {msg} on line {self.line} at pos {self.col} \'{self.cur_char}\'')


    def advance(self):
        self.ip += 1
        if self.ip < len(self.source):
            self.cur_char = self.source[self.ip]
        else:
            self.cur_char = '\0'
    

    def skip_whitespace(self):
        while self.cur_char in (' ', '\t', '\b', '\r', '\n', '#'):
            if self.cur_char == '#':
                while self.cur_char != '\0' and self.source[self.ip] != '\n':
                    self.col += 1
                    self.advance()
            
            elif self.cur_char == '\n':
                self.line += 1
                self.col = 1
                self.advance()
            else:
                self.col += 1
                self.advance()

    
    def make_identifier(self) -> Token:
        start_pos = self.ip
        start_col = self.col

        while self.cur_char != '\0' and self.cur_char.isalnum() or self.cur_char == '_' or self.cur_char == '-':
            self.col += 1
            self.advance()

        lexeme = self.source[start_pos:self.ip]

        return Token(self.line, start_col, KEYWORDS[lexeme], lexeme) if lexeme in KEYWORDS else Token(self.line, start_col, TokenType.ID, lexeme)
    

    def make_string(self) -> Token:
        start_pos = self.ip
        start_col = self.col

        while self.cur_char != '\0' and self.cur_char != '\'':
            self.col += 1
            self.advance()
        
        self.col += 1
        self.advance()

        return Token(self.line, start_col, TokenType.STR, self.source[start_pos:self.ip-1])
    

    def make_number(self) -> Token:
        start_pos = self.ip
        start_col = self.col

        while self.cur_char != '\0' and self.cur_char.isdigit():
            self.col += 1
            self.advance()
        
        return Token(self.line, start_col, TokenType.INT, self.source[start_pos:self.ip])
    

    def get_next(self) -> Token:
        if self.ip < len(self.source):
            self.skip_whitespace()

            if self.cur_char == '\'':
                self.col += 1
                self.advance()
                return self.make_string()
                
            if self.cur_char.isalpha():
                return self.make_identifier()

            if self.cur_char.isdigit():
                return self.make_number()
            
            if self.cur_char in SINGLE_CHARS:
                token = Token(self.line, self.col, SINGLE_CHARS[self.cur_char], self.cur_char)
                self.col += 1
                self.advance()
                return token
            
        return Token(self.line, self.col, TokenType.EOF, 'EOF')