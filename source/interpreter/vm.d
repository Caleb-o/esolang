module interpreter.vm;

import std.stdio;
import std.format;
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

			if (callStack[i].stack.length == 0) {
				writeln("-- EMPTY --");
			}

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

	int indexOfProc(string name) {
		foreach(idx, proc; env.defs.procedures) {
			if (proc.name == name) return cast(int)idx;
		}

		return -1;
	}

	public:

	void interpret() {
		writeln("Running...");

		// Check for main function
		int mainIdx = indexOfProc("main");
		if (mainIdx == -1) {
			error("Could not find main symbol");
		}

		// Warn about non-supported/unused values
		if (env.defs.procedures[mainIdx].parameters.length > 0 ||
			(env.defs.procedures[mainIdx].returnTypes.length > 0 &&
			env.defs.procedures[mainIdx].returnTypes[0] != ValueKind.VOID)) {
			writefln("Warning: Main contains arguments or a non-void return type");
		}

		// Create a new stack item
		callStack[callStack.length++] = CallFrame("main", -1);

		// Fetch entry point and start there
		ip = env.defs.procedures[mainIdx].startIdx;


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
					string procName = env.defs.procedures[index].name;
					callStack[callStack.length++] = CallFrame(procName, ip+1);
					
					// Check arguments
					size_t stackSize = callStack[$-2].stack.length;
					size_t arity = env.defs.procedures[index].parameters.length;
					if (stackSize < arity) {
						error(format("'%s' expected %d arguments, but got %d.", procName, arity, stackSize));
					}


					// Check parameter types
					int stackIdx = cast(int)(stackSize-arity);
					int stackMoveFrom = -1;

					foreach(key, param; env.defs.procedures[index].parameters) {
						if (callStack[$-2].stack[stackIdx].kind != param.kind) {
							error(format("'%s' at param '%s' expected type %s but got %s", 
								procName, key,
								param.kind, callStack[$-2].stack[stackIdx].kind
							));
						}

						if (param.isMoved) {
							if (stackMoveFrom == -1) stackMoveFrom = stackIdx;
						}

						// Copy values to frame
						callStack[$-1].stack[callStack[$-1].stack.length++] = callStack[$-2].stack[stackIdx];

						stackIdx++;
					}

					if (stackMoveFrom >= 0) {
						callStack[$-2].stack.length -= (stackIdx - stackMoveFrom);
					}

					ip = env.defs.procedures[index].startIdx;
					break;
				}

				case ByteCode.RETURN: {
					immutable int index = env.code[++ip];
					string procName = env.defs.procedures[index].name;
					size_t retLen = env.defs.procedures[index].returnTypes.length;

					// Check stack size against return size
					if (callStack[$-1].stack.length > retLen) {
						error("Values must be dropped from the stack on exit");
					}

					// Non-void
					if (env.defs.procedures[index].returnTypes.length > 0) {
						auto retType = env.defs.procedures[index].returnTypes[0];

						if (callStack[$-1].stack[0].kind != retType) {
							error(format("'%s' expected return type %s but got %s",
									procName, retType,
									callStack[$-1].stack[0].kind
								));
						} else {
							callStack[$-2].stack[callStack[$-2].stack.length++] = callStack[$-1].stack[0];
						}
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