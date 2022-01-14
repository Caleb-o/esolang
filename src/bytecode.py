from utils import iota
from enum import Enum


class ByteCode(Enum):
    OP_PUSH = iota(True)
    OP_POP = iota()
    OP_ADD = iota()
    OP_SUB = iota()
    OP_MUL = iota()
    OP_DIV = iota()
    OP_PRINT = iota()
    OP_SWAP = iota()
    OP_DUPLICATE = iota()
    OP_LOOP_START = iota()
    OP_LOOP_END = iota()
    OP_RETURN = iota()