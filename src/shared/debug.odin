package shared

import "core:fmt"


@(private="file")
print_defs :: proc(env : ^Environment) {
	fmt.println("=== Procedure Definitions ===")

	for proc_def, idx in env.defs.procedures {
		fmt.printf("%3d '%s' [", 
			idx,
			proc_def.identifier,
		)

		for flag, fidx in proc_def.params {
			flag_str := vflag_as_str(flag)
			fmt.printf("(%s)", flag_str)
			delete(flag_str)

			if fidx < len(proc_def.params)-1 {
				fmt.print(" ")
			}
		}

		fmt.print("] :: ")

		for flag, fidx in proc_def.returns {
			flag_str := vflag_as_str(flag)
			fmt.printf("(%s)", flag_str)
			delete(flag_str)

			if fidx < len(proc_def.returns)-1 {
				fmt.print(" ")
			}
		}

		fmt.printf(" | starts at %d\n", proc_def.op_start)
	}
	fmt.println()
}

@(private="file")
print_ids :: proc(env : ^Environment) {
	fmt.println("=== Identifiers ===")

	for identifier, idx in env.identifiers {
		fmt.printf("%3d '%s'\n", idx, identifier)
	}
	fmt.println()
}

@(private="file")
print_values :: proc(env : ^Environment) {
	fmt.println("=== Values ===")

	for value, idx in env.values {
		flag_str := vflag_as_str(value.flags)
		fmt.printf("%3d '%v' %s\n", idx, value.data, flag_str)
		delete(flag_str)
	}
	fmt.println()
}

print_env :: proc(env : ^Environment) {
	// Temp type for printing
	Bind_Type :: enum {
		Plain, Strict, Param,
	}

	print_defs(env)
	print_ids(env)
	print_values(env)

	fmt.println("=== ByteCode ===")

	for idx := 0; idx < len(env.code); idx += 1 {
		code := env.code[idx]
		fmt.printf("%4d %s", idx, cast(Byte_Code)code)

		#partial switch cast(Byte_Code)env.code[idx] {
			case Byte_Code.Push:
				value := env.values[env.code[idx+1]]
				fmt.printf("<'%v' %s>", value.data, value.flags)
				idx += 1

			case Byte_Code.Return:
				proc_idx := env.code[idx+1]
				fmt.printf("<%d>\n", proc_idx)
				idx += 1

			case Byte_Code.Bind:
				type := env.code[idx+1]
				id := env.identifiers[env.code[idx+2]]
				pos := env.code[idx+3]
				fmt.printf("<'%s' %s %d>", id, cast(Bind_Type)type, pos)
				idx += 3
		}

		fmt.println()
	}
	fmt.println()
}