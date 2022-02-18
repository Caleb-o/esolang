package main

import "info"

main :: proc() {
	info.enable()

	using info.Log_Level_Flag

	info.log_message(Debug, "Debugging stuff")
	info.log_message(Info, "Hello!")
	info.log_message(Warning, "Something isn't right")
	info.log_message(Error, "ABORT!")

	info.disable()
}