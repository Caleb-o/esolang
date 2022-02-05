module interpreter.vm;

import std.stdio;
import core.stdc.stdlib : abort;

import parsing.environment;
import parsing.bytecode : ByteCode;
import parsing.value;


final class VM {
	private {
		Environment env;
		size_t ip;
		// TODO: Make this dependent on callframe
		Value[] stack;
	}

	@disable this();
	this(Environment env) {
		this.env = env;
	}

	public:
	void interpret() {
		writeln("Running...");

		// Check for main function
		if ("main" !in env.defs.procedures) {
			writeln("Could not find main symbol");
			abort();
		}

		// Fetch entry point and start there
		ip = env.defs.procedures["main"].startIdx;

		while (ip < env.code.length) {
			switch(env.code[ip]) {
				case ByteCode.PUSH: {
					stack[stack.length++] = env.literals[env.code[++ip]];
					break;
				}

				case ByteCode.PRINT: {
					if (stack.length > 0) {
						writeValue(stack[stack.length-1u]);
					}
					break;
				}

				case ByteCode.PRINTLN: {
					if (stack.length > 0) {
						writeValue(stack[stack.length-1u]);
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