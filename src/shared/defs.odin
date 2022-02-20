package shared

Procedure_Def :: struct {
	op_start : u16, // Where the procedure starts
	params : []ValueFlag,
	returns : []ValueFlag,
}

Definitions :: struct {
	// TODO: Fill procedures builtins before parsing
	procedures : [dynamic]Procedure_Def,
}


// Cleanup all definitions
defs_cleanup :: proc(def : ^Definitions) {
	delete(def.procedures)
}