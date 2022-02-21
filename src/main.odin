package main

import "core:os"

import "info"
import "process"
import "misc"
import "shared"

main :: proc() {
	info.enable()

	defer {
		process.cleanup_reserved()
		info.disable()
		delete(os.args)
	}

	// Incorrect usage
	if len(os.args) < 3 {
		info.log(info.Log_Level_Flag.Error, "Usage: eso [run|test|check] [script] <flags>")
		return
	}

	// Get the filename
	file_name := os.args[2]
	cfg := misc.Cfg_Flags.Default
	mode := misc.Run_Mode.Run

	switch os.args[1] {
		case "run":		mode = misc.Run_Mode.Run
		case "test":	mode = misc.Run_Mode.Test
		case "check":	mode = misc.Run_Mode.Check

		case: // Default
			info.log(info.Log_Level_Flag.Warning, "Unknown mode found '%s'", os.args[1])
			return
	}

	// Parse cmd flags
	for arg_idx := 3; arg_idx < len(os.args); arg_idx += 1 {
		switch os.args[arg_idx] {
			case "--no-logs":
				// Error if Pedantic is set
				if (cfg & (misc.Cfg_Flags.Warn_Id_Proc_Id | misc.Cfg_Flags.Warn_Id_Keywords) == (misc.Cfg_Flags.Warn_Id_Proc_Id | misc.Cfg_Flags.Warn_Id_Keywords)) {
					info.log(info.Log_Level_Flag.Error, "Pedantic cannot be set with No Logs flag")
					return
				}
				cfg |= misc.Cfg_Flags.No_Logs

			case "--debug":			cfg |= misc.Cfg_Flags.Debug
			case "--warn-id-key":	cfg |= misc.Cfg_Flags.Warn_Id_Keywords
			case "--warn-id-proc":	cfg |= misc.Cfg_Flags.Warn_Id_Proc_Id
			case "--pedantic":
				// Error if NoLogs is set
				if (cfg & misc.Cfg_Flags.No_Logs == misc.Cfg_Flags.No_Logs) {
					info.log(info.Log_Level_Flag.Error, "Pedantic cannot be set with No Logs flag")
					return
				}
				cfg |= misc.Cfg_Flags.Warn_Id_Proc_Id | misc.Cfg_Flags.Warn_Id_Keywords

			case: // Unknown
				info.log(info.Log_Level_Flag.Warning, "Unknown flag found '%s'", os.args[arg_idx])
		}
	}

	// Give logger flags
	info.check_run_flags(cfg)
	defer process.parser_cleanup()

	// Failed
	if status := process.parser_init(file_name, cfg); status != misc.Eso_Status.Ok {
		return
	}

	env := process.parse()
	defer shared.env_free(env)
	
	if cfg & misc.Cfg_Flags.Debug == misc.Cfg_Flags.Debug {
		shared.print_env(env)
	}
}