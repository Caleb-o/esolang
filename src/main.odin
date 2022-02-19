package main

import "core:os"
import "core:fmt"

import "info"
import "process"
import "misc"

main :: proc() {
	info.enable()

	defer {
		process.cleanup_reserved()
		info.disable()
		delete(os.args)
	}

	if len(os.args) < 2 {
		info.log(info.Log_Level_Flag.Error, "Usage: eso [script] <flags>")
		return
	}

	// Get the filename
	file_name := os.args[1]
	cfg := misc.Cfg_Flags.Default

	// Parse cmd flags
	arg_idx := 2
	for arg_idx < len(os.args) {
		defer arg_idx += 1

		/*
			No_Log = 0x02, 					// Disables all logging
			Debug = 0x04,					// Enables/overrides some flags for debugging purposes
			Show_Defs_Bytecode = 0x08,		// Shows bytecode, IDs and procedure definitions | Debug will enable this
			Warn_Id_Keywords = 0x16,		// Checks identifiers against keywords and warns for potential typo
			Warn_Id_Proc_Id = 0x32,			// Similar to ID Keywords, it will warn if a binding is similar to a procedure name
			Warn_Pedantic = 0x64,			// Enables all warnings
		*/

		switch os.args[arg_idx] {
			case "--no-logs":
				// Error if Pedantic is set
				if (cfg & (misc.Cfg_Flags.Warn_Id_Proc_Id | misc.Cfg_Flags.Warn_Id_Keywords) == (misc.Cfg_Flags.Warn_Id_Proc_Id | misc.Cfg_Flags.Warn_Id_Keywords)) {
					info.log(info.Log_Level_Flag.Error, "Pedantic cannot be set with No Logs flag")
					return
				}
				cfg |= misc.Cfg_Flags.No_Logs

			case "--debug":			cfg |= misc.Cfg_Flags.Debug
			case "--bytecode":		cfg |= misc.Cfg_Flags.Show_Defs_Bytecode
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
				formatted := fmt.aprintf("Unknown flag found '%s'", os.args[arg_idx])
				info.log(info.Log_Level_Flag.Warning, formatted)
				delete(formatted)
		}
	}

	// Give logger flags
	info.check_run_flags(cfg)


	if data, status := os.read_entire_file_from_filename(file_name); status {
		defer delete(data)
		lexer := process.Lexer { 1, 1, 0, string(data) }
		info.log(info.Log_Level_Flag.Info, "Lexing")

		for {
			if token, status := process.get_token(&lexer); status == misc.Eso_Status.Ok {
				if token.kind == process.Token_Type.Eof {
					free(token)
					break
				}

				info.log(info.Log_Level_Flag.Debug, token.lexeme)
				free(token)
			} else {
				info.log(status, "Parser encountered an error")
				free(token)
				break
			}
		}
	} else {
		info.log(info.Log_Level_Flag.Error, "Could not read file")
		delete(data)
	}
}