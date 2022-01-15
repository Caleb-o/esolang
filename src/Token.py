from enum import Enum
from dataclasses import dataclass
from utils import iota

class TokenType(Enum):
    INT = iota(True)
    IMPL = iota()
    STR = iota()
    PLUS = iota()
    MINUS = iota()
    STAR = iota()
    SLASH = iota()
    EQUAL_TO = iota()
    LESS_THAN = iota()
    GREATER_THAN = iota()
    PROC = iota()
    BANG = iota()
    ID = iota()
    SEMICOLON = iota()
    DOT = iota()
    COMMA = iota()
    SWAP = iota()
    POP = iota()
    DUPLICATE = iota()
    LSQUARE = iota()
    RSQUARE = iota()
    MACRO = iota()
    UNDEF = iota()
    END = iota()
    EOF = iota()


@dataclass
class Token:
    line: int
    column: int
    ttype: TokenType
    lexeme: str