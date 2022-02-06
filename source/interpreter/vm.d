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
	Value[string] bindings;

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
		if (callStack.length > 0) {
			writeln("== Callstack ==");
			
			foreach_reverse(idx, frame; callStack) {
				writef("depth %d \"%s\" [ ", idx, frame.procID);

				foreach(id, _; frame.bindings) {
					writef("'%s' ", id);
				}
				writeln("] stack");

				if (frame.stack.length == 0) {
					writeln("-- EMPTY --\n");
					continue;
				}

				foreach_reverse(val_idx, val; frame.stack) {
					writef("[%d] ", val_idx);
					writeValue(val);
					writeln();
				}
				writeln();
			}
		}
	}

	void error(string message) {
		writeln("Runtime Error: ", message);
		unwindStack();
		abort();
	}

	int indexOfProc(string name) {
		foreach(idx, proc; env.defs.procedures) {
			if (proc.name == name) return cast(int)idx;
		}

		return -1;
	}

	Value popStack() {
		if (callStack[$-1].stack.length == 0) {
			error("Could not pop an empty stack");
		}

		scope(exit) --callStack[$-1].stack.length;
		return callStack[$-1].stack[callStack[$-1].stack.length-1u];
	}

	void pushStack(Value value) {
		callStack[$-1].stack[callStack[$-1].stack.length++] = value;
	}

	void arithmeticOp() {
		auto rhs = popStack();
		auto lhs = popStack();

		// Type check left and right side kinds
		if (lhs.kind != rhs.kind) {
			error(format("Trying to operate on different value types. Lhs '%s', Rhs '%s'", lhs.kind, rhs.kind));
		}

		auto op = env.code[ip++];

		switch(lhs.kind) {
			case ValueKind.INT: {
				switch(op) {
					case ByteCode.ADD:	pushStack(createValue(lhs.data.idata + rhs.data.idata)); break;
					case ByteCode.SUB:	pushStack(createValue(lhs.data.idata - rhs.data.idata)); break;
					case ByteCode.MUL:	pushStack(createValue(rhs.data.idata * lhs.data.idata)); break;
					case ByteCode.DIV:	pushStack(createValue(rhs.data.idata / lhs.data.idata)); break;

					default:	error(format("Unknown operation '%s'", op)); break;
				}
				break;
			}

			case ValueKind.FLOAT: {
				switch(op) {
					case ByteCode.ADD:	pushStack(createValue(lhs.data.fdata + rhs.data.fdata)); break;
					case ByteCode.SUB:	pushStack(createValue(lhs.data.fdata - rhs.data.fdata)); break;
					case ByteCode.MUL:	pushStack(createValue(rhs.data.fdata * lhs.data.fdata)); break;
					case ByteCode.DIV:	pushStack(createValue(rhs.data.fdata / lhs.data.fdata)); break;

					default:	error(format("Unknown operation '%s'", op)); break;
				}
				break;
			}

			case ValueKind.BOOL: .. case ValueKind.STRUCT: {
				error(format("Cannot use arithmetic operations on type '%s'", lhs.kind));
				break;
			}

			default: break;
		}
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
					pushStack(env.literals[env.code[++ip]]);
					ip++;
					break;
				}

				case ByteCode.POP: {
					popStack();
					ip++;
					break;
				}

				case ByteCode.ADD: .. case ByteCode.DIV: {
					arithmeticOp();
					break;
				}

				case ByteCode.BINDING: {
					immutable int index = env.code[++ip];
					pushStack(callStack[$-1].bindings[env.idLiterals[index]]);
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
						callStack[$-1].bindings[key] = callStack[$-2].stack[stackIdx++];
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
					size_t stackSize = callStack[$-1].stack.length;

					// Check stack size against return size
					if (stackSize > retLen) {
						error(format("'%s' returned with %d items on the stack, return count is %d.",
							procName, stackSize, retLen
						));
					}

					if (stackSize < retLen) {
						error(format("'%s' requires %d return values, but only %d was returned.", procName, retLen, stackSize));
					}

					// Non-void
					foreach(idx, retType; env.defs.procedures[index].returnTypes) {
						// Check return values against required types
						if (callStack[$-1].stack[idx].kind != retType) {
							error(format("'%s' expected return type %s but got %s",
									procName, retType,
									callStack[$-1].stack[idx].kind
								));
						} else {
							callStack[$-2].stack[callStack[$-2].stack.length++] = callStack[$-1].stack[idx];
						}
					}

					ip = callStack[$-1].returnIdx;
					callStack.length--;
					break;
				}

				case ByteCode.PRINT: {
					if (callStack[$-1u].stack.length == 0) {
						error("Cannot print an empty stack");
					}
					writeValue(callStack[$-1u].stack[$-1u]);
					ip++;
					break;
				}

				case ByteCode.PRINTLN: {
					if (callStack[$-1u].stack.length == 0) {
						error("Cannot print an empty stack");
					}
					writeValue(callStack[$-1u].stack[$-1u]);
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