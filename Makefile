DEFS 	:=
OPS		:=-w -std=c++2a
OUT		:=eso

.DEFAULT_GOAL := debug

# Get OS type
ifeq ($(OS), Windows_NT)
	SRC		:= $(shell dir /s /b *.cpp)
	OS_NAME := 'Windows'
	OUT :=$(OUT).exe
else
	# Assume linux
	SRC		:= $(shell find src -type f -iregex ".*\.cpp")
	OS_NAME := 'Linux'
endif

# Commands
debug:
	@echo Building [DEBUG] "$(OUT)" for $(OS_NAME)...
	g++ -g $(OPS) $(DEFS) $(SRC) -o bin/debug_$(OUT)
	@echo Done.

release:
	@echo Building "$(OUT)" for $(OS_NAME)...
	g++ -O3 $(OPS) $(DEFS) $(SRC) -o bin/$(OUT)
	@echo Done.
