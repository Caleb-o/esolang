package shared

Environment :: struct {
	defs : Definitions,
	code : [dynamic]u16,
	identifiers : [dynamic]string,
	values : [dynamic]Value,
}

env_free :: proc(env : ^Environment) {
	delete(env.code)
	delete(env.identifiers)
	delete(env.values)

	defs_cleanup(&env.defs)
	
	free(env)
}