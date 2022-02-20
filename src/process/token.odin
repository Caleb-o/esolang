package process

Token_Type :: enum {
	// Operators
	Plus, Minus, Slash, Star, Modulus, Colon, Colon_Colon,
	Greater, Greater_Eq, Less, Less_Eq, Equal, Or, And, Not,
	Comma, Dot, Bind, Unbind,

	// Misc
	L_Curly, R_Curly, L_Paren, R_Paren, L_Square, R_Square, Return,
	
	// Keywords
	Proc, If, Elif, Else, Then, Drop, Print, Print_Ln,
	Dup, Swap, Rot, Using, Loop,

	// Types
	Id, String_Lit, Bool_Lit, Int_Lit, Float_Lit, Type_Id,

	// Modifiers
	Strict,

	Eof,
}

Token :: struct {
	kind : Token_Type,
	line, col : int,
	lexeme : string,
}


// Global Reserved
RESERVED := map[string]Token_Type {
	"proc" = .Proc,
	"using" = .Using,
	"loop" = .Loop,

	"if" = .If,
	"elif" = .Elif,
	"else" = .Else,
	"then" = .Then,

	"print" = .Print,
	"println" = .Print_Ln,

	"drop" = .Drop,
	"swap" = .Swap,
	"dup" = .Dup,
	"rot" = .Rot,

	"and" = .And,
	"or" = .Or,
	"not" = .Not,

	"strict" = .Strict,
	"void" = .Type_Id,
	"int" = .Type_Id,
	"float" = .Type_Id,
	"bool" = .Type_Id,
	"string" = .Type_Id,
}

// Frees the memory held by reserved map
cleanup_reserved :: proc() {
	delete(RESERVED)
}