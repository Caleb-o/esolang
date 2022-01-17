from utils import auto
from enum import IntEnum


class ByteCode(IntEnum):
    OP_PUSH = auto(True)
    OP_POP = auto()
    OP_ADD = auto()
    OP_SUB = auto()
    OP_MUL = auto()
    OP_DIV = auto()
    OP_STR = auto()
    OP_LESS = auto()
    OP_GREATER = auto()
    OP_EQUAL_TO = auto()
    OP_NEGATE = auto()
    OP_INPUT = auto()
    OP_IF = auto()
    OP_BREAK = auto()
    OP_PRINT = auto()
    OP_PRINT_CHAR = auto()
    OP_SWAP = auto()
    OP_PROC_CALL = auto()
    OP_TEST_CALL = auto()
    OP_DUPLICATE = auto()
    OP_LOOP_START = auto()
    OP_LOOP_END = auto()
    OP_RETURN = auto()
    OP_ASSERT = auto()
    OP_REVERSE = auto()


def op_as_str(op: ByteCode) -> str:
    return str(op).replace('ByteCode.OP_', '').lower()