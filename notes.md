## Bugs
Nothing notable found yet :^)


## Unique Literals
At the moment, literals are not unique when they are stored. ID literals are unique to save on space, we need to make the literals do the same. If we have 100 1's, there will currently be 100 copies of the number 1 stored, which is not very efficient.


## Bit Operations
Allow for bit operations
- left shift << 
- right shift >>
- and &&
- or ||


## Type Checking at Compile-time
Tsoding's video: https://www.youtube.com/watch?v=CYp99h-faa0
Check WASM Stack-Based type checking: https://binji.github.io/posts/webassembly-type-checking/
- Can probably type check based on the tokens, during the parsing phase.
- Could use bit flags to check which types are available and which are allowed
	eg. INT, INT, PLUS ( INT, FLOAT, STRING )
	- Two integers and Plus accepts: int, float and string
	- Ops could index a map with Values, where is has bit flags of what it allows
	eg. INT -> INT, FLOAT
	- Int can only be added onto integers or floats
- Type checker should be a class that the parser contains, so allow for some abstraction to move some code away from the parser.
	- Remember to remove run-time typechecking when done
	- Type check bindings, so we don't override bindings with different types


## Better logging and Tracking
A single method of logging should be implemented, rather than having an error log
as apart of each process. Logging should have levels (INFO, WARN, ERROR) with overloaded
functions to allow the lexer, parser and VM to call, which allows them to pass through
the relevant information.


## Dead code Elimination / Warning
During the parsing phase, we should track procedure usage and remove code or throw a
warning to say that "procedure (X) is unused on line (X) in file (X)". We need to keep
track of all files included, call count for each procedure and any other information
that will prove useful for logging and/or processing.


## Runners
A polymorphic Runner type should be available, which can be switched out with the interpreter. All ops should become their own function in the base runner (virtual, but not pure virtual), so we can have one definition of the function. With runners we could re-implement the VM and allow for other behaviours. The VM will most likely be the base runner, with some abstractions that turns each operation into its own function. Most functions will most likely have a default implementation.

## Tests:
There should be a test flag which allows parsing and running tests. The test runner would just be the Runner, with some new functions to run all tests and track success/failures.


## Test
We can parse tests similarly to a proc. !!This will actually require a seperate runner, since we need to handle errors differently. An error should trigger a failure, instead of immediately cutting execution. As the parser does not know much about native procedures, the assert might have to become a keyword OR the runner will assume it succeeded if no assertion was hit. As a test cannot be returned from, a HALT opcode will be appended.
```
test 'Testing numbers' {
	1 1 1 + +
	3 = @assert
}
```


## Circular dependencies:
If we were to allow Circular dependencies, we would have to do a final step after 
the last parse. Here would would have to patch all proc_calls, since the symbol may
not exist at the time.

## The problem:
If we call a procedure that doesn't exist in the file, but exists within another,
it will not be capable of resolving the ID. eg. Experimenting imports Fib, Fib imports
Experimenting. Since we parse Fib first, any call to Experimenting procedures will not
resolve because we haven't parsed it yet.

## Solution #1:
Store procedures as an id_literal (or proc_id_literal) and take the index from there,
this will require some adjustments to proc_call/procedure_def() because we expect it
to be added as a definition immediately. We can make proc_id_literal a temporary buffer,
just so we can resolve definitions.


## List of Native Procedures:
* [ ] `write_file` : Writes to a file, using the previous string as a file path and the string prior as the buffer


## Top-level code
If we simply ran the VM top to bottom, we would run into procedures and run their code, which is not desired. One solution is, we capture all top-level code and insert it all at the bottom and set the ip to the start of the top-level code.

**Solution:** Capture all top-level code and create a procedure called "boottop" that is inserted before main. Insert a procedure call to "boottop" before its body. We will have to fetch its starting index to know where to insert and the insert of booting code will have to be done last.

## STD library / Usings
The import system in Eso is very basic and does not consider resolutions etc. There also is no redirect if a std path is found (currently).

How should the standard library be written? What should be expected in an interpreted langauge that is concatenative and stack based? My only idea is to write wrappers around some keywords/native procedures that make them less error prone. Eg. if a file_read fails, return an empty string. Did a stoi/stof/stob fail? Return its default value using a "safe" procedure that handles related errors.

Maths functions - eg. cos, sin, tan, 


## Potential Features / Systems
* Namespacing / Packages instead of imports
	* This would require a "project" file of sorts or automatically compiling files in a directory, recursively.
	* Name resolution for procedures
* % / mod
* continue / break within loops
	* Requires re-evaluation of a block, to determine where the continue/break goes to
	* A continue/break can simply be a goto - continue to top, break to bottom
* More CLI arguments/flags:
	* -c : lexes + parses files only, checks for errors
	* -t : runs all tests within a file (Requires Runner system)