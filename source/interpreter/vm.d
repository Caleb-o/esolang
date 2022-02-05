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
				

				default: {
					break;
				}
			}
			ip++;
		}
	}
}