module parsing.environment;
import parsing.bytecode : ByteCode;
import parsing.value : Value;

final class Environment {
	public:
	
	ByteCode[] code;
	Value[] literals;
	Value[string] variables;
	size_t[string] functions;
}