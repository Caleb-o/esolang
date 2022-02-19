package shared

// Flags tell the value how to be handled and allows many modifiers to be added at once
//? Note: We use type IDs here so we can use this enum for definitions etc
//?		  It includes modifiers to make it easier to check for them and keeps them all contained
ValueFlag :: enum {
	Array = 0x01,
	Strict = 0x02,
	Integer = 0x04,
	Float = 0x08,
	Boolean = 0x16,
	String = 0x32,
}

// Internal data of a value
ValueData :: union {
	bool,
	i32,
	f32,
	string,
}

// Value wraps the data and flags
Value :: struct {
	data : ValueData,
	flags : ValueFlag,
}