package shared

Environment :: struct {
	code : [dynamic]Byte_Code,
	defs : Definitions,
}

env_free :: proc(env : ^Environment) {
	delete(env.code)
	defs_cleanup(&env.defs)
	free(env)
}