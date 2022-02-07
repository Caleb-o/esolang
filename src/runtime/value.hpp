#pragma once

namespace Runtime {
	enum ValueKind {
		INT, FLOAT, BOOL, STRING, STRUCT
	};
	
	class Value {
		ValueKind kind;
	};
}