package misc

Cfg_Flags :: enum u32 {
	Default = 0x01,					// No special behaviours
	Show_Defs_Bytecode = 0x02,		// Shows bytecode, IDs and procedure definitions | Debug will enable this
	Debug = 0x04,					// Enables/overrides some flags for debugging purposes
	Warn_Id_Keywords = 0x08,		// Checks identifiers against keywords and warns for potential typo
	Warn_Id_Proc_Id = 0x16,			// Similar to ID Keywords, it will warn if a binding is similar to a procedure name
	Warn_Pedantic = 0x32,			// Enables all warnings
	No_Logs = 0x64, 				// Disables all logging
}