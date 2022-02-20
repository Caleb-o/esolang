package shared

Environment :: struct {
	defs : Definitions,
	code : [dynamic]u32,
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