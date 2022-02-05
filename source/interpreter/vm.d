module interpreter.vm;

import std.stdio;
import core.stdc.stdlib : abort;

import parsing.environment;
import parsing.bytecode : ByteCode;
import parsing.value;


struct CallFrame {
	string procID;
	Value[] stack;

	this(string procID) {
		this.procID = procID;
	}
}


final class VM {
	private {
		Environment env;
		size_t ip;
		CallFrame[] callStack;
	}

	@disable this();
	this(Environment env) {
		this.env = env;
	}

	void unwindStack() {
		writeln("== Callstack ==");
		size_t i = callStack.length-1u;
		do {
			writefln("depth %d \"%s\" stack", i, callStack[i].procID);

			foreach_reverse(idx, val; callStack[i].stack) {
				writef("[%d] ", idx);
				writeValue(val);
				writeln();
			}
			writeln();
		} while(i > 0);
	}

	void error(string message) {
		writeln("Error: ", message);
		unwindStack();
		abort();
	}

	public:
	void interpret() {
		writeln("Running...");

		// Check for main function
		if ("main" !in env.defs.procedures) {
			error("Could not find main symbol");
		}

		// Warn about non-supported/unused values
		if (env.defs.procedures["main"].parameters.length > 0 ||
			env.defs.procedures["main"].returnTypes[0] != ValueKind.VOID) {
			writefln("Warning: Main contains arguments or a non-void return type");
		}

		// Create a new stack item
		callStack[callStack.length++] = CallFrame("main");

		// Fetch entry point and start there
		ip = env.defs.procedures["main"].startIdx;


		while (ip < env.code.length) {
			switch(env.code[ip]) {
				case ByteCode.PUSH: {
					callStack[callStack.length-1u].stack[callStack[callStack.length-1u].stack.length++] = env.literals[env.code[++ip]];
					break;
				}

				case ByteCode.POP: {
					if (callStack[callStack.length-1u].stack.length == 0) {
						error("Trying to drop an empty stack");
					}
					callStack[callStack.length-1u].stack.length--;
					break;
				}

				case ByteCode.RETURN: {
					if (callStack[callStack.length-1u].stack.length > 0) {
						error("Values must be dropped from the stack on exit");
					}

					callStack.length--;
					break;
				}

				case ByteCode.PRINT: {
					if (callStack[callStack.length-1u].stack.length > 0) {
						writeValue(callStack[callStack.length-1u].stack[callStack[callStack.length-1u].stack.length-1u]);
					}
					break;
				}

				case ByteCode.PRINTLN: {
					if (callStack[callStack.length-1u].stack.length > 0) {
						writeValue(callStack[callStack.length-1u].stack[callStack[callStack.length-1u].stack.length-1u]);
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