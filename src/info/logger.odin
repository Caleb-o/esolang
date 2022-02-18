package Info

import "core:fmt"
import "core:time"
import "../misc"


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
Logger_Flags : Log_Flags


@(private)
COLOUR_MAP := map[Log_Level_Flag]string {
	.Debug = "\u001b[32;1m",
	.Info = "\u001b[37;1m",
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
	fmt.printf("\u001b[37;1m[%s%s\u001b[37;1m]", COLOUR_MAP[flag], flag)

	if Logger_Flags == Log_Flags.Show_Timings {
		_, mm, ss := time.clock_from_stopwatch(Timer)
		ms := time.duration_milliseconds(time.stopwatch_duration(Timer))

		fmt.printf("\u001b[38;5;23m ")

		if mm > 0 {
			fmt.printf("%dm-%ds-%fms", mm, ss, ms)
		} else if ss > 0 {
			fmt.printf("%ds-%fms", ss, ms)
		} else {
			fmt.printf("%fms", ms)
		}
	}
	fmt.printf("\u001b[37;1m ")
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

log_eso_status :: proc(status : misc.Eso_Status, message : string) {
	print_header(.Error)
	fmt.printf("%s with status '%s'\n", message, status)
}

log :: proc{log_message, log_lexer, log_lexer_char, log_eso_status}