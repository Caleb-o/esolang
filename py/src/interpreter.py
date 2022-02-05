from parser import Environment, ByteCode, Scope
from bytecode import op_as_str
import debug


RUN_TESTS = False


class Interpreter:
    def __init__(self, env: Environment) -> None:
        self.env = env
        self.cur_op = 0
        self.cur_scope = 0
        self.in_test = False
        self.test_counter = -1


    def error_msg(self, msg):
        try:
            op_str = op_as_str(ByteCode(self.env.byte_code[self.cur_op]))
        except:
            op_str = f'Unknown \'{self.cur_op}\''

        if self.cur_op < len(self.env.byte_code):
            raise Exception(f'[Interpreter] {msg} on op <{self.cur_op}:{op_str}>')
        else:
            raise Exception(f'[Interpreter] {msg}')
    

    def assertion(self, msg):
        print(self.env.scopes[-1].stack)
        raise Exception(f'Assertion Failed: {msg}' if not self.in_test else f'[Test "{self.env.test_names[self.test_counter]}"] Assertion Failed{msg}')


    def get_op(self, idx: int = 0) -> ByteCode:
        return self.env.byte_code[self.cur_op] if idx == 0 else self.env.byte_code[self.cur_op + idx]


    def push_value(self, value: int):
        self.env.scopes[self.cur_scope].stack.append(value)


    def push_constant(self, idx: int):
        self.env.scopes[self.cur_scope].stack.append(self.env.constants[idx])

    def push_neg_constant(self, idx: int):
        self.env.scopes[self.cur_scope].stack.append(-self.env.constants[idx])


    def try_pop(self):
        if len(self.env.scopes[self.cur_scope].stack) > 0:
            return self.env.scopes[self.cur_scope].stack.pop()
        else:
            self.error_msg('Cannot pop stack. Stack is empty!')

    
    # Returns if bytecode at offset (from current)
    def peek_op_is_type(self, idx: int, type: ByteCode) -> bool:
        try:
            return self.env.byte_code[self.cur_op + idx] == type
        except:
            return False


    def try_peek(self) -> int:
        try:
            return self.env.scopes[self.cur_scope].stack[-1]
        except:
            self.error_msg('Trying to peek into empty stack')

    
    def interpret(self):
        debug.print_env(self.env)
        debug.reset_debug()

        if debug.DEBUG and not debug.IGNORE_OUTPUT:
            print('\n=== Output ===')

        while self.cur_op < len(self.env.byte_code):
            operation = self.get_op()

            if debug.DEBUG:
                if debug.op_max > 0 and debug.op_count >= debug.op_max:
                    print('debug exit')
                    break

                if op_as_str(operation) not in debug.ops:
                    debug.ops[op_as_str(operation)] = 0

                debug.op_count += 1
                debug.ops[op_as_str(operation)] += 1


            if operation == ByteCode.OP_PUSH:
                self.cur_op += 1

                if self.get_op(-2) == ByteCode.OP_NEGATE:
                    self.push_neg_constant(self.get_op())
                else:
                    self.push_constant(self.get_op())
            elif operation == ByteCode.OP_POP:
                if len(self.env.scopes[-1].stack) > 0:
                    self.try_pop()

            elif operation == ByteCode.OP_ADD:
                val_b = self.try_pop()
                val_a = self.try_pop()
                self.push_value(val_a + val_b)

            elif operation == ByteCode.OP_SUB:
                val_b = self.try_pop()
                val_a = self.try_pop()
                self.push_value(val_a - val_b)

            elif operation == ByteCode.OP_MUL:
                val_b = self.try_pop()
                val_a = self.try_pop()
                self.push_value(val_a * val_b)

            elif operation == ByteCode.OP_DIV:
                val_b = self.try_pop()
                val_a = self.try_pop()

                if val_b == 0:
                    self.error_msg("Cannot divide by 0")

                self.push_value(val_a / val_b)
            elif operation == ByteCode.OP_GREATER:
                val_b = self.try_pop()
                val_a = self.try_pop()
                self.push_value(int(val_a > val_b))
            
            elif operation == ByteCode.OP_LESS:
                val_b = self.try_pop()
                val_a = self.try_pop()
                self.push_value(int(val_a < val_b))
            
            elif operation == ByteCode.OP_EQUAL_TO:
                val_b = self.try_pop()
                val_a = self.try_pop()
                self.push_value(int(val_a == val_b))
            
            elif operation == ByteCode.OP_IF:
                condition = self.try_pop()
                self.cur_op += 1

                if not condition:
                    self.cur_op = int(self.get_op())

            elif operation == ByteCode.OP_BREAK:
                self.cur_op += 1

                # Error from parser
                if int(self.get_op()) < 0:
                    self.error_msg('Break jump point was not set')

                self.cur_op = int(self.get_op())
            
            elif operation == ByteCode.OP_INPUT:
                user_input = input()
                value_count = len(user_input)

                if len(user_input) > 0:
                    values = user_input.split(' ')

                    for value in values:
                        # Push value
                        if value.isdigit():
                            self.push_value(int(value))
                        # Push length if string
                        elif value.isalnum():
                            [self.push_value(ord(c)) for c in value]

                # Push amount of values on the stack
                self.push_value(value_count)

            elif operation == ByteCode.OP_PRINT:
                if not debug.DEBUG or debug.DEBUG and not debug.IGNORE_OUTPUT:
                    if not self.in_test:
                        # Currently strings can only be print from constants/literals,
                        # Since strings cannot be constructed
                        if self.peek_op_is_type(-2, ByteCode.OP_STR) and not self.peek_op_is_type(-3, ByteCode.OP_ASSERT):
                            print(self.env.strings[self.get_op(-1)], end='')
                        else:
                            print(f'{self.try_peek()} ', end='')

            elif operation == ByteCode.OP_PRINT_CHAR:
                if not debug.DEBUG or debug.DEBUG and not debug.IGNORE_OUTPUT:
                    if not self.in_test:
                        if self.peek_op_is_type(-2, ByteCode.OP_STR) and not self.peek_op_is_type(-3, ByteCode.OP_ASSERT):
                            # Print each character as ascii code
                            for c in self.env.strings[self.get_op(-1)]:
                                print(f'{ord(c)} ', end='')
                            print()
                        else:
                            print(chr(self.try_peek()), end='')

            elif operation == ByteCode.OP_SWAP:
                if len(self.env.scopes[-1].stack) > 1:
                    val_b = self.try_pop()
                    val_a = self.try_pop()
                    self.push_value(val_b)
                    self.push_value(val_a)
                    
            elif operation == ByteCode.OP_DUPLICATE:
                self.push_value(self.try_peek())

            elif operation == ByteCode.OP_REVERSE:
                value_count = self.try_pop()

                values = []

                for _ in range(value_count):
                    values.append(self.try_pop())
                
                for value in values:
                    self.push_value(value)

            elif operation == ByteCode.OP_LOOP_END:
                self.cur_op += 1

                # Error from parser
                if int(self.get_op()) < 0:
                    self.error_msg('Loop end jump point was not set')

                # Go back to the start
                if len(self.env.scopes[-1].stack) > 0 and self.try_peek() > 0:
                    self.cur_op = int(self.get_op())

            elif operation == ByteCode.OP_PROC_CALL:
                self.cur_op += 1
                arg_count = int(self.get_op())

                self.cur_op += 1
                return_count = int(self.get_op())

                # Create a new scope
                self.env.scopes.append(Scope(self.cur_scope + 1, []))

                # Check correct arg count
                if len(self.env.scopes[self.cur_scope].stack) < arg_count:
                    self.error_msg(f'{"-" * (self.cur_scope * 4)} Procedure expected {arg_count} argument(s) but got {len(self.env.scopes[self.cur_scope].stack)}')

                # Add arguments to scope
                self.env.scopes[self.cur_scope + 1].stack.extend(self.env.scopes[self.cur_scope].stack[-arg_count:])

                for _ in range(-arg_count, 0):
                    self.env.scopes[self.cur_scope].stack.pop()

                # Push Return count onto stack
                self.env.scopes[self.cur_scope].stack.append(return_count)

                self.cur_scope += 1

            elif operation == ByteCode.OP_RETURN:            
                # Get return value from previous stack and remove it    
                return_count = self.env.scopes[self.cur_scope-1].stack[-1]
                self.env.scopes[self.cur_scope-1].stack.pop()

                # -1 will define any amount
                if return_count > 0:
                    if len(self.env.scopes[self.cur_scope].stack) < return_count:
                        self.error_msg(f'Return expected {return_count} argument(s) but got {len(self.env.scopes[self.cur_scope].stack)}')
                    elif len(self.env.scopes[self.cur_scope].stack) > return_count:
                        self.error_msg(f'Return expected {return_count} argument(s) but got {len(self.env.scopes[self.cur_scope].stack)}')
                
                    # Add items to stack
                    self.env.scopes[self.cur_scope - 1].stack.extend(self.env.scopes[self.cur_scope].stack[-return_count:])
                elif return_count < 0:
                    # Copy the entire stack
                    self.env.scopes[self.cur_scope - 1].stack.extend(self.env.scopes[self.cur_scope].stack)
                    
                self.cur_scope -= 1
                self.env.scopes.pop()

                if self.in_test:
                    self.in_test = False
                    print(f'"{self.env.test_names[self.test_counter]}" passed!')
            
            elif operation == ByteCode.OP_ASSERT:
                condition = self.try_pop()
                self.cur_op += 1

                if condition == 0:
                    self.assertion(f'{self.env.strings[self.get_op(1)]}')
                else:
                    self.cur_op += 1
            
            elif operation == ByteCode.OP_TEST_CALL:
                self.cur_op += 1

                if not RUN_TESTS:
                    self.cur_op = int(self.get_op())
                else:
                    self.in_test = True
                    self.test_counter += 1

                    # Append return value
                    self.env.scopes[self.cur_scope].stack.append(0)

                    self.cur_scope += 1
                    self.env.scopes.append(Scope(self.cur_scope, []))

            # Operations with no functionality without context
            elif operation == ByteCode.OP_STR:
                # Don't really care for anything here
                self.cur_op += 1

            elif operation in [ ByteCode.OP_LOOP_START, ByteCode.OP_NEGATE ]:
                # Don't really care for anything here
                pass

            else:
                self.error_msg(f'Operation not implemented : \'{operation}\'')
            
            self.cur_op += 1

        
        # Exit contains values on stack
        if len(self.env.scopes[0].stack) > 0:
            print(f'\nstack :: {self.env.scopes[0].stack}')
            self.error_msg('Stack is not empty on exit')


        if debug.DEBUG:
            print(f'\nstack :: {self.env.scopes[0].stack}')
            
            print()
            debug.print_debug_symbols()
        
        self.env.scopes.clear()
        self.env.byte_code.clear()