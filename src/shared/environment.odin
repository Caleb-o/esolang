package shared

Environment :: struct {
	defs : Definitions,
	code : [dynamic]u16,
	identifiers : [dynamic]string,
	globals : [dynamic]u16,		// Points to a position in the values table
	values : [dynamic]Value,
}

env_free :: proc(env : ^Environment) {
	delete(env.code)
	delete(env.identifiers)
	delete(env.values)
	delete(env.globals)

	defs_cleanup(&env.defs)
	
	free(env)
}