package shared

// Flags tell the value how to be handled and allows many modifiers to be added at once
//? Note: We use type IDs here so we can use this enum for definitions etc
//?		  It includes modifiers to make it easier to check for them and keeps them all contained
ValueFlag :: enum {
	Array = 0x01,
	Strict = 0x02,
	Void = 0x04,
	Integer = 0x08,
	Float = 0x16,
	Boolean = 0x32,
	String = 0x64,
}

// Internal data of a value
ValueData :: union {
	bool,
	int,
	f32,
	string,
}

// Value wraps the data and flags
Value :: struct {
	data : ValueData,
	flags : ValueFlag,
}


str_to_vflag :: proc(str : string) -> ValueFlag {
	switch str {
		case "void":		return ValueFlag.Void
		case "int":			return ValueFlag.Integer
		case "float":		return ValueFlag.Float
		case "bool":		return ValueFlag.Boolean
		case "string":		return ValueFlag.String
		case:				return ValueFlag.Void
	}
}