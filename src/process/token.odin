package process

Token_Type :: enum {
	// Operators
	Plus, Minus, Slash, Star, Modulus, Colon, Colon_Colon,
	Greater, Greater_Eq, Less, Less_Eq, Equal, Or, And, Not,
	Comma, Dot, Bind, Unbind,

	// Misc
	L_Curly, R_Curly, L_Paren, R_Paren, Return,
	
	// Keywords
	Proc, If, Elif, Else, Then, Drop, Print, Print_Ln,
	Dup, Swap, Rot, Using, Loop,

	// Types
	Id, String_Lit, Bool_Lit, Int_Lit, Float_Lit, Type_Id,

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
	"if" = .If,
	"elif" = .Elif,
	"else" = .Else,
	"then" = .Then,
	"print" = .Print,
	"println" = .Print_Ln,
	"drop" = .Drop,
	"swap" = .Swap,
	"rot" = .Rot,
	"using" = .Using,
	"loop" = .Loop,
	"and" = .And,
	"or" = .Or,
	"not" = .Not,
}

// Frees the memory held by reserved map
cleanup_reserved :: proc() {
	delete(RESERVED)
}