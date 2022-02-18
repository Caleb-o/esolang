package Info

import "core:fmt"
import "core:time"

// The level of the log 
Log_Level_Flag :: enum {
	Debug, Info, Warning, Error,
}

// Flags to change the behaviour of the logger
Log_Flags :: enum {
	None, Show_Timings,
}

@(private)
Timer := time.Stopwatch{}
@(private)
Logger_Flags := Log_Flags.None


@(private)
COLOUR_MAP := map[Log_Level_Flag]string {
	.Debug = "\u001b[32;1m",
	.Info = "\u001b[37m",
	.Warning = "\u001b[33;1m",
	.Error = "\u001b[31;1m",
}


// Initialise the logger. This starts the timer if the flag is passed in.
enable :: proc(flags := Log_Flags.None) {
	Logger_Flags = flags

	if Logger_Flags == Log_Flags.Show_Timings {
		time.stopwatch_start(&Timer)
	}
}

// Cleanup and stop the timer of the logger
disable :: proc() {
	if Logger_Flags == Log_Flags.Show_Timings {
		time.stopwatch_stop(&Timer)
	}
	log_message(Log_Level_Flag.Info, "Finished")

	delete(COLOUR_MAP)
}


@(private)
// Prints the coloured log level and timing (if flag is enabled)
print_header :: proc(flag : Log_Level_Flag) {
	// Print initial flag with its colour
	fmt.printf("[%s%s\u001b[37m] ", COLOUR_MAP[flag], flag)

	if Logger_Flags == Log_Flags.Show_Timings {
		_, mm, ss := time.clock_from_stopwatch(Timer)
		ms := cast(int)time.duration_milliseconds(time.stopwatch_duration(Timer))
		fmt.printf("[%d:%d:%d] ", mm, ss, ms)
	}
}


// General log procedure
log_message :: proc(flag : Log_Level_Flag, message : string) {
	print_header(flag)
	fmt.printf("%s\n", message)
}

log_lexer :: proc(flag : Log_Level_Flag, message : string, line, col : int) {
	print_header(flag)
	fmt.printf("%s on line %d at pos %d\n", message, line, col)
}

log_lexer_char :: proc(flag : Log_Level_Flag, message : string, unknown : u8, line, col : int) {
	print_header(flag)
	fmt.printf("%s '%c' on line %d at pos %d\n", message, unknown, line, col)
}

log :: proc{log_message, log_lexer, log_lexer_char}