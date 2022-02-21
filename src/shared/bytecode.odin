package shared

// Refer to Spec for meanings
Byte_Code :: enum u16 {
	Halt,
	Print, Println,
	Push,
	Drop,
	Duplicate,
	Swap,
	Rotate,
	Bind,
	Unbind,
	Load_Binding,
	Proc_Call,
	Builtin_Call,
	Return,
	Goto,
	Condition,
	Arithmetic,
	Comparison,
	Boolean,
}