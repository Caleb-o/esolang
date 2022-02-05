module interpreter.vm;

import std.stdio;
import core.stdc.stdlib : abort;

import parsing.environment;
import parsing.bytecode : ByteCode;
import parsing.value;


struct CallFrame {
	string procID;
	int returnIdx;
	Value[] stack;

	this(string procID, ulong returnIdx) {
		this.procID = procID;
		this.returnIdx = cast(int)returnIdx;
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
		int i = cast(int)callStack.length-1u;
		do {
			writefln("depth %d \"%s\" stack", i, callStack[i].procID);

			foreach_reverse(idx, val; callStack[i].stack) {
				writef("[%d] ", idx);
				writeValue(val);
				writeln();
			}
			writeln();

			i--;
		} while(i >= 0);
	}

	void error(string message) {
		writeln("Error: ", message);
		unwindStack();
		abort();
	}

	string getProcName(size_t idx) {
		size_t i;
		foreach(key; env.defs.procedures.byKey) {
			if (idx == i) {
				return key;
			}
			
			i++;
		}

		return "None";
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
		callStack[callStack.length++] = CallFrame("main", -1);

		// Fetch entry point and start there
		ip = env.defs.procedures["main"].startIdx;


		while (ip < env.code.length) {
			switch(env.code[ip]) {
				case ByteCode.PUSH: {
					callStack[callStack.length-1u].stack[callStack[callStack.length-1u].stack.length++] = env.literals[env.code[++ip]];
					ip++;
					break;
				}

				case ByteCode.POP: {
					if (callStack[callStack.length-1u].stack.length == 0) {
						error("Trying to drop an empty stack");
					}
					callStack[callStack.length-1u].stack.length--;
					ip++;
					break;
				}

				case ByteCode.PROCCALL: {
					immutable int index = env.code[++ip];
					string procName = getProcName(index);

					callStack[callStack.length++] = CallFrame(procName, ip+1);
					ip = env.defs.procedures[procName].startIdx;
					break;
				}

				case ByteCode.RETURN: {
					if (callStack[callStack.length-1u].stack.length > 0) {
						error("Values must be dropped from the stack on exit");
					}

					ip = callStack[$-1].returnIdx;
					callStack.length--;
					break;
				}

				case ByteCode.PRINT: {
					if (callStack[callStack.length-1u].stack.length > 0) {
						writeValue(callStack[callStack.length-1u].stack[callStack[callStack.length-1u].stack.length-1u]);
					}
					ip++;
					break;
				}

				case ByteCode.PRINTLN: {
					if (callStack[callStack.length-1u].stack.length > 0) {
						writeValue(callStack[callStack.length-1u].stack[callStack[callStack.length-1u].stack.length-1u]);
					}
					writeln();
					ip++;
					break;
				}

				default: {
					ip++;
					break;
				}
			}
		}
	}
}