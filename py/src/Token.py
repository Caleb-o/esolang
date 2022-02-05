from enum import Enum
from dataclasses import dataclass
from utils import auto

class TokenType(Enum):
    INT = auto(True)
    IMPL = auto()
    ASSERT = auto()
    STR = auto()
    PLUS = auto()
    MINUS = auto()
    STAR = auto()
    SLASH = auto()
    EQUAL_TO = auto()
    LESS_THAN = auto()
    GREATER_THAN = auto()
    PROC = auto()
    BANG = auto()
    ID = auto()
    SEMICOLON = auto()
    DOT = auto()
    COMMA = auto()
    SWAP = auto()
    POP = auto()
    QMARK = auto()
    DUPLICATE = auto()
    LSQUARE = auto()
    RSQUARE = auto()
    MACRO = auto()
    UNDEF = auto()
    IF = auto()
    BREAK = auto()
    NEGATE = auto()
    END = auto()
    REVERSE = auto()
    TEST = auto()
    EOF = auto()


@dataclass
class Token:
    line: int
    column: int
    ttype: TokenType
    lexeme: str