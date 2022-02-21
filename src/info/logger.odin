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
	None = 0x01,
	Show_Timings = 0x02,
	No_Logs = 0x04,
	Debug = 0x08,
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
	if Logger_Flags & Log_Flags.No_Logs != Log_Flags.No_Logs {
		if Logger_Flags & Log_Flags.Show_Timings == Log_Flags.Show_Timings {
			time.stopwatch_stop(&Timer)
		}
		log(Log_Level_Flag.Info, "Finished")
	}

	delete(COLOUR_MAP)
}

check_run_flags :: proc(flags : misc.Cfg_Flags) {
	if (flags & misc.Cfg_Flags.No_Logs == misc.Cfg_Flags.No_Logs) {
		Logger_Flags |= Log_Flags.No_Logs
	}
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
log_detail_char :: proc(flag : Log_Level_Flag, message : string, unknown : u8, line, col : int) {
	if Logger_Flags & Log_Flags.No_Logs != Log_Flags.No_Logs {
		print_header(flag)
		fmt.printf("%s '%c' on line %d at pos %d\n", message, unknown, line, col)
	}
}

log_fmt :: proc(flag : Log_Level_Flag, format : string, args : ..any) {
	if Logger_Flags & Log_Flags.No_Logs != Log_Flags.No_Logs {
		print_header(flag)

		if len(args) > 0 {
			formatted := fmt.aprintf(format, args)
			fmt.println(formatted)
			delete(formatted)
		} else {
			fmt.println(format)
		}
	}
}

log_detail_fmt :: proc(flag : Log_Level_Flag, format : string, line, col : int, args : ..any) {
	if Logger_Flags & Log_Flags.No_Logs != Log_Flags.No_Logs {
		print_header(flag)

		if len(args) > 0 {
			formatted := fmt.aprintf(format, args)
			fmt.printf("%s on line %d at pos %d\n", formatted, line, col)
			delete(formatted)
		} else {
			fmt.printf("%s on line %d at pos %d\n", format, line, col)
		}
	}
}

log_eso_status :: proc(status : misc.Eso_Status, message : string) {
	if Logger_Flags & Log_Flags.No_Logs != Log_Flags.No_Logs {
		print_header(.Error)
		fmt.printf("%s with status '%s'\n", message, status)
	}
}

log :: proc{log_detail_char, log_detail_fmt, log_fmt, log_eso_status}