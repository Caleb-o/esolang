# Eso Spec
*This is a document that will explain the ins and outs of esolang*

## Eso Flavours
There are currently two "flavours" of Eso. Each flavour can be accessed and used, but they will not be supported anymore.
* [Saturn](https://github.com/Caleb-o/esolang/tree/saturn) - Original Python implementation
* [Jupiter](https://github.com/Caleb-o/esolang/tree/jupiter) - C++ Rewrite which added more syntax and features

Both flavours offer different syntaxes and features, at different speeds. Saturn is very rough around the edges and was very limited in its usability. It stood as a foundation which would soon become the Jupiter flavour. In Jupiter, there was simpler syntax and more features, which made it look and feel more like a "modern" scripting language. There is typechecking, overloaded procedures, native procedures and much more! Since it was implemented using C++, it offered an incredible speed increase when compared to Saturn.

## What's different from the last flavour?
"Pluto" is the new flavour in the collection of Eso. Like the last, it will include many changes to features, syntax and all-round look and feel. To begin, here is a broad overview of what differs:

New:
* Written in Odin instead of C++
	* C++ is great! But it can easily become annoying to manage and write
	* Odin offers high-level features and syntax for a better workflow
* Arrays. It is difficult to program without some sort of data storage, so arrays are required.
```
# Create an array of size 4 (Take)
$arr[4 : int]

# We could also tell the array to capture N amount of values while binding
1 2 $arr[2 : int](2)

# Index an array
arr[1] println drop

# Assign a value
3 $arr[2]
```
Note: *To create an array, an index and type annotation is required. Arrays will be bounds checked and will throw an error if indexing out of bounds. This can be done at compile-time since we know the size.*

* Type properties/procedures over native procedures
	* Reasoning: It is a much nicer and cleaner syntax. The old native calls just would not work with the new style/flow.
```
# Old
str ' ' @str_split

# New
str.split(' ')
```
* Some syntax modifications
* Using abstraction to allow for better error handling
	* Store the hash in a table with its filepath
	* Push the File obj on top of the stack (Allows us to unwind include stack to see where it's called from)
* Single logger for info, warnings and errors
	* Overloads for each procedure, so we can supply relevant information to the logger
* Quality of life changes
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
	* Note: Breaks and continues are just a GOTO op which points to either the end or start of the loop
```
# Old
true loop { false }

# New
loop { break }
```

Removed:
* Procedure overloading
	* Made it easy to have the same functionality with multiple signatures, but you would not know which it would call.
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

## Considerations
* Type as an expr/statement
```
# Check if type of foo matches int
int foo.type =
```