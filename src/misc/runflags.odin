package misc

Run_Flags :: enum {
	None, 					// Nothing special
	Debug,					// Enables/overrides some flags for debugging purposes
	Warn_Id_Keywords,		// Checks identifiers against keywords and warns for potential typo
}