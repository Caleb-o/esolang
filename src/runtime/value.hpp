#pragma once
#include <string>


namespace Runtime {
	enum class ValueKind {
		INT, FLOAT, BOOL, STRING, STRUCT
	};

	union ValueData {
		int integer;
		float floating;
		bool boolean;
		char *string;
	};
	
	struct Value {
		ValueKind kind;
		ValueData* data;
		bool read_only;
	};
}