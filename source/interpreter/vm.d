module interpreter.vm;

import std.stdio;

import parsing.environment;
import parsing.bytecode : ByteCode;
import parsing.value;


final class VM {
	private {
		Environment env;
		size_t ip;
		Value[] stack;
	}

	@disable this();
	this(Environment env) {
		this.env = env;
	}

	public:
	void interpret() {
		writeln("Running...");

		while (ip < env.code.length) {
			switch(env.code[ip]) {
				case ByteCode.PUSH: {
					immutable int idx = env.code[++ip];
					stack[stack.length++] = env.literals[idx];
					break;
				}

				case ByteCode.STORE: {
					ip++;
					break;
				}

				case ByteCode.LOAD: {
					immutable int idx = env.code[++ip];
					int i;

					foreach(var; env.variables) {
						if (i++ < idx) {
							continue;
						} else {
							stack[stack.length++] = var;
							break;
						}
					}
					break;
				}

				case ByteCode.LOAD_LIT: {
					immutable int idx = env.code[++ip];
					stack[stack.length++] = env.literals[idx];
					break;
				}

				case ByteCode.PRINT: {
					if (stack.length > 0) {
						writeValue(stack[$-1]);
					}
					break;
				}

				case ByteCode.PRINTLN: {
					if (stack.length > 0) {
						writeValue(stack[$-1]);
					}
					writeln();
					break;
				}

				default: {
					break;
				}
			}
			ip++;
		}
	}
}