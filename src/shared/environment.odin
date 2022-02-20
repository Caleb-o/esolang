package shared

Environment :: struct {
	code : [dynamic]u32,
	defs : Definitions,

	id_loc : map[string]u32,
	identifiers : [dynamic]string,
}

env_free :: proc(env : ^Environment) {
	delete(env.code)
	delete(env.id_loc)
	delete(env.identifiers)

	defs_cleanup(&env.defs)
	
	free(env)
}