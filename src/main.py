import sys
from parser import Parser
from interpreter import Interpreter


def main():
    argv_len = len(sys.argv)

    if argv_len <= 1:
        while True:
            user_input = input('Repl> ')

            if len(user_input) == 0 or user_input == 'q':
                break

            try:
                parser = Parser('repl', user_input)
                interpreter = Interpreter(parser.parse())
                
                interpreter.interpret()
            except Exception as e:
                print(e)
    elif argv_len == 2:
        try:
            parser = Parser(sys.argv[1], open(sys.argv[1]).read())
            interpreter = Interpreter(parser.parse())
            
            interpreter.interpret()
        except Exception as e:
            print(e)
    else:
        print('usage: lang [file]')


if __name__ == '__main__':
    main()