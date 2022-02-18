# Eso Spec
*This is a document that will explain the ins and outs of esolang*

## Eso Flavours
There are currently two "flavours" of Eso. Each flavour can be accessed and used, but they will not be supported anymore.
* [Saturn](https://github.com/Caleb-o/esolang/tree/saturn) - Original Python implementation
* [Jupiter](https://github.com/Caleb-o/esolang/tree/jupiter) - C++ Rewrite which added higher level syntax and features

Both flavours offer different syntaxes and features, at different speeds. Saturn is very rough around the edges and was very limited in its usability. It stood as a foundation which would soon become the Jupiter flavour. In Jupiter, there was high-level syntax and more features, which made it look and feel more like a "modern" scripting language. There is typechecking*, overloaded procedures, native procedures and much more! Since it was implemented using C++, it offered an incredible speed increase when compared to Saturn.

***Note: Typechecking worked in most cases, but was not fully realised and threw errors where syntax and flow was valid.**

## What's different from the last flavour?
"Pluto" is the new flavour in the Eso collection. Like the last, it will include many changes to features, syntax and all-round look and feel. To begin, here is a broad overview of what differs:

New:
* Written in Odin instead of C++
	* C++ is great! But it can easily become annoying to manage and write
	* Odin offers high-level features and syntax for a better workflow
* Arrays. It is difficult to program without some sort of data storage, so arrays are required.
	```
	# Create an array of size 4 using type int
	$arr[4 : int]

	# We could also tell the array to capture N amount of values while binding
	1 2 $arr[2 : int](2)

	# Index an array
	arr[1] println drop

	# Assign a value
	3 $arr[0]
	```
***Note: To create an array, an index and type annotation is required. Arrays will be bounds checked and will throw an error if indexing out of bounds. This can be done at compile-time since we know the size.**

* Type properties/procedures over native procedures
	* Reasoning: It is a much nicer and cleaner syntax. The old native calls just would not work with the new style/flow.
	* More information later
	```
	# Old
	str ' ' @str_split

	# New
	str.split(' ')
	```
* Some syntax modifications
* [**Internal**] Using abstraction over files to allow for better error handling
	* Store the hash in a table with its filepath
	* Push the File obj on top of the stack (Allows us to unwind include stack to see where it's called from)
* [**Internal**] Single logger for info, warnings and errors
	* Overloads for each procedure, so we can supply relevant information to the logger
	* Each log will automatically have a timestamp relative to when the program began
	```
	# If a debug flag is set, we will tell the logger to also show all debugging based logs
	log.show_debug_logs()

	log(Log.ERROR, "Invalid symbol found", token)
	log(Log.INFO, "Finished parsing")
	log(Log.DEBUG, "Skipping using on existing file", file_info)
	```
* [**Internal**] Runners (Refer to notes)
* Tests (Refer to notes)
* Bit operations - Bit shifts, or, and
* Quality of life changes
	* Better binding syntax, which allows simpler usage with plain and strict bindings. Allows the bindings to be more explicit with a single syntax.
	```
	# Old
	1 2 bind | a, b |
	3 strict | c |

	# New
	# Using modifiers, type annotations and inference
	1 2 3 4 $a : strict, b : int, c : strict int, d

	# Unbinding values
	!a, b, d
	```
* Top-level constants
	```
	# Types will be inferred since it is constant, we only allow an expression after
	PI :: 3.14
	```
* Compile-time checks
	* Type checking
	* Bound accesses in arrays
* Explicit procedure calls
	```
	# Old
	| 1 2 | !add_two println drop

	# New
	add_two(1, 2) println drop
	```
* Loop break and continue
	* ***Note: Breaks and continues are just a GOTO op which points to either the end or start of the loop.**
	```
	# Old
	true loop { false }

	# New
	loop { break }
	```
* if/elif/else with new syntax
	```
	# Old
	true if {
	# Body
	} else {
	# Body
	}

	# New
	if 2 1 > then {
	# Body
	# Goto end
	} elif 3 2 > then {
	# Body
	# Goto end
	} else {
	# Body
	# Fallthrough
	}
	```
* Type as an expr
	```
	# Check if type of foo matches int
	int foo.type =
	```

Removed:
* Procedure overloading
	* Overloading made it easy to have the same functionality with multiple signatures, but you would not know which it would call.
	* In a language like this, it's difficult to really know whether a type is correct.
	* I would like more explicit behaviours in Eso
* No native procedures
	* Native procedures are being replaced with type procedures, properties and builtin procedures.
	* Builtin procedures are basically the same, however, there is no special syntax to invoke them.
	```
	# Old
	'hello.eso' @read_file
	str ' ' @str_split

	# New
	read_file('hello.eso')
	str.split(' ')
	```
* Captures
	* Captures served the purpose of passing data into a procedure. Since we're now passing them directly, we don't need this feature.


## Revised ByteCode / Operations
The "Bytecode" may need to use u16/u32 instead of a typical u8. Primarily for the fact that operations like GOTO, PUSH, and PROC_CALL/BUILTIN_CALL will not point to the correct value, since u8 can only hold 256 values.

* `HALT`					- Halts the execution of code
* `PRINT, PRINTLN`			- Prints the current value on the stack
* `PUSH [IDX]`				- Pushes a value onto the stack
* `DROP`					- Drops the current value on the stack
* `DUPLICATE`				- Duplicates the top item on the stack
* `SWAP`					- Swaps the top two items on the stack
* `ROTATE`					- Rotates the top 3 items on the stack `a b c => c b a`
* `BIND [FLAG] [ID_IDX]`	- Binds a value using a certain flag, with an identifier
* `UNBIND [COUNT] [ID_IDX] <SEMI ID_IDX>` - Unbinds COUNT amount of bindings with ID
* `LOAD_BINDING [ID_IDX]`	- Pushes a value onto the stack, using the value from a binding
* `PROC_CALL [DEF_IDX]`		- Calls the procedure at index of its definition
* `BUILTIN_CALL [DEF_IDX]`	- Calls a builtin procedure at index of its definition
* `GOTO [POS]`				- Moves the instruction pointer to the position specified
* `CONDITION [FALSE_POS]` 	- If the current value is false, jump to position after body
* `RETURN`					- Flag for the VM to pop the current frame (A goto will handle the jump back, which follows the return)
* `ARITHMETIC [FLAG]`		- ADD|SUB|MUL|DIV maths operations
* `COMPARISON [FLAG]`		- GREATER|GREATER_EQ|LESS|LESS_EQ|EQUAL comparison operations
* `BOOLEAN [FLAG]`			- OR|AND|NOT boolean operations