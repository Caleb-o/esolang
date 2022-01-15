from parser import Environment, ByteCode


class Interpreter:
    def __init__(self, env: Environment) -> None:
        self.env = env
        self.cur_op = 0
        self.cur_scope = 0


    def error_msg(self, msg):
        raise Exception(f'[Interpreter] {msg}')


    def get_op(self) -> ByteCode:
        return self.env.byte_code[self.cur_op]


    def push_value(self, value: int):
        self.env.scopes[self.cur_scope].stack.append(value)


    def push_constant(self, idx: int):
        self.env.scopes[self.cur_scope].stack.append(self.env.contants[idx]) 


    def try_pop(self):
        if len(self.env.scopes[self.cur_scope].stack) > 0:
            return self.env.scopes[self.cur_scope].stack.pop()
        else:
            self.error_msg('Cannot pop stack. Stack is empty!')


    def peek(self):
        return self.env.scopes[self.cur_scope].stack[-1]

    
    def interpret(self):
        while self.cur_op < len(self.env.byte_code):
            operation = self.get_op()

            if operation == ByteCode.OP_PUSH:
                self.cur_op += 1
                self.push_constant(self.get_op())
            elif operation == ByteCode.OP_POP:
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
            elif operation == ByteCode.OP_PRINT:
                print(self.peek())
            elif operation == ByteCode.OP_SWAP:
                val_b = self.try_pop()
                val_a = self.try_pop()
                self.push_value(val_b)
                self.push_value(val_a)
            elif operation == ByteCode.OP_DUPLICATE:
                self.push_value(self.peek())
            elif operation == ByteCode.OP_LOOP_START:
                # Don't really care for anything here
                pass
            elif operation == ByteCode.OP_LOOP_END:
                self.cur_op += 1

                # Go back to the start
                if self.peek() > 0:
                    self.cur_op = int(self.get_op())
            else:
                self.error_msg(f'Operation not implemented : \'{operation}\'')
            
            self.cur_op += 1


        print(f'stack :: {self.env.scopes[0].stack}')
        
        self.env.scopes.clear()
        self.env.byte_code.clear()