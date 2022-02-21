package shared

Procedure_Def :: struct {
	identifier : string, // For debugging purposes
	op_start : u16, // Where the procedure starts
	params : [dynamic]ValueFlag,
	returns : [dynamic]ValueFlag,
}

Definitions :: struct {
	// TODO: Fill procedures builtins before parsing
	procedures : [dynamic]Procedure_Def,
}

@(private="file")
procedure_def_cleanup :: proc(proc_def : ^Procedure_Def) {
	delete(proc_def.params)
	delete(proc_def.returns)
}

// Cleanup all definitions
defs_cleanup :: proc(def : ^Definitions) {
	// Cleanup procedure definitions
	for _, idx in def.procedures {
		procedure_def_cleanup(&def.procedures[idx])
	}
	delete(def.procedures)
}