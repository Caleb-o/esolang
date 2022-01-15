from utils import iota
from enum import IntEnum


class ByteCode(IntEnum):
    OP_PUSH = iota(True)
    OP_POP = iota()
    OP_ADD = iota()
    OP_SUB = iota()
    OP_MUL = iota()
    OP_DIV = iota()
    OP_LESS = iota()
    OP_GREATER = iota()
    OP_EQUAL_TO = iota()
    OP_WHILE = iota()
    OP_IF = iota()
    OP_END = iota()
    OP_PRINT = iota()
    OP_PRINT_CHAR = iota()
    OP_SWAP = iota()
    OP_PROC_CALL = iota()
    OP_DUPLICATE = iota()
    OP_LOOP_START = iota()
    OP_LOOP_END = iota()
    OP_RETURN = iota()


def op_as_str(op: ByteCode) -> str:
    return str(op).replace('ByteCode.OP_', '').lower()