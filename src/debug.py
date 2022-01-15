from parser import ByteCode, Environment
from bytecode import op_as_str
import operator

DEBUG = False
IGNORE_OUTPUT = False # Only evaluated when debug is true

# Debug symbols
op_count = 0
op_max = 0
ops: dict[str, int] = {}

def reset_debug():
    global op_count, ops
    op_count = 0
    ops.clear()


def print_debug_symbols():
    operations = dict( sorted(ops.items(), key=operator.itemgetter(1),reverse=True))

    print(f'{"OpCode Usage".ljust(16)} :: {op_count}')
    for op, count in operations.items():
        print(f'{op_as_str(op).ljust(16)} :: {count}')


def print_env(env: Environment):
    if not DEBUG:
        return

    idx = 0

    def get_code(index: int) -> int:
        return env.byte_code[index]

    print('OP_POS   OPERATION')

    while idx < len(env.byte_code):
        op = get_code(idx)

        print('{:>4}     '.format(idx), end='')

        if op == ByteCode.OP_PUSH:
            value = env.contants[get_code(idx + 1)]
            idx += 1
            print(f'{op_as_str(op)}<value: {value}>')

        elif op == ByteCode.OP_BREAK:
            value = get_code(idx + 1)
            idx += 1
            print(f'{op_as_str(op)}<jump_to: {value}>')

        elif op == ByteCode.OP_PROC_CALL:
            arg_c = get_code(idx + 1)
            ret_c = get_code(idx + 2)
            idx +=  2
            print(f'{op_as_str(op)}<args: {arg_c}, returning: {ret_c}>')
        
        elif op == ByteCode.OP_LOOP_END:
            jump_to = get_code(idx + 1)
            idx += 1
            print(f'{op_as_str(op)}<jump_to: {jump_to}>')
        
        elif op == ByteCode.OP_IF:
            jump_to = get_code(idx + 1)
            idx += 1
            print(f'{op_as_str(op)}<jump_to: {jump_to}>')

        else:
            # One byte operations
            print(op_as_str(op))
        
        idx += 1