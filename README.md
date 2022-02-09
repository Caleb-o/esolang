## What is it?
This is a bytecode interpreted, stack-based language that I've written in C++. I have been reading about and exploring new languages, so I decided that I would like to develop one of my own. Initially, it was going to be something similar to BrainF*ck but using numbers and words to have a better visualisation. I came across Tsoding's Porth language, which was a stack-based language written in Python (now self-hosted) and I've taken a lot of inspiration from it.


## Influenced By
- Porth
- C Family (including C#)
- Rust
- Pascal
- BrainFuck


## Backends
- Interpreter
- Transpiler (Desirable)
    - Pascal?


## Language Spec

### Comments
Coments use the '#' and are single line only.
```
# This is a comment
```

### Types
Eso has two types currently:
- `int` : int, int literal
- `float` : float, float literal
- `bool` : bool, bool literal
- `const string` : string literal
- `capture` : value that contains any number of other values

**All values are immutable, so you must create a new value in its place if you want to change it.*
**Only integers, floats and booleans can be created from within the code, so there isn't an easy way to construct a string using code.*


### Example
```
# String literal
'This is a string'
# Int literal
1 2 3 4 5 6
# Float literal
2.3 4.5 6.7
```

### Operators
- `+ - * /` Mathmatical operators
- `= < > <= >=` Comparison
- `!` invoke procedure/dynamic capture 
- `|` Starts and ends a capture

### Example
```
1 2 +
# Create a loop
dup 3 = loop {
    1 -
    dup 0 >
}

# Push a two and swap with the last value
2 swap
```

### Keywords
- `if` Checks if the current value on the stack is `0` or `1`
- `dup` Duplicates the current value on the stack
- `drop` Drops the current value from the stack
- `swap` Swaps top two values on the stack
- `rot` Rotates top and top-2 value eg. '1 2 3' = '3 2 1'
- `proc` Starts a procedure
- `using` Import statement
- `print` Prints the top value of the stack
- `println` Prints the top value of the stack with a newline
- `bind` Starts a bind statement
- `strict` Starts a strict bind statement
- `loop` Starts a loop
- `void` Void type
- `int` Integer type
- `float` Floating point type
- `bool` Boolean type
- `string` String type

### Example
```
# Check if 1 == 1 and print hello if true
1 1 = if {
    'Hello!' println drop
}
```

### Using
`using` is used to import other eso scripts. It will import the entire script (excluding its main).

```
using 'myscript'

proc main() -> void {
    # Call a procedure from the script, that takes an integer
    | 10 | !my_procedure
}
```

### Procedures
We can call blocks of code using a procedure. In Eso, a procedure can take and give values, but is not required to do either. The nice thing is, that we can also overload procedures. So we are able to use the same name, as long as the parameters are different. In some languages, you cannot return mulitple types or you have to create a tuple. Eso supports any number of return values.

### Example
```
# Define a procedure that takes one value and returns none
proc my_print_proc(x : int) -> int {
    # We print X but we also return it
    x println
}

proc main() -> void {
    # Call our proc (It will print 10)
    | 10 | !my_print_proc
}
```

The `| 10 |` syntax is a capture. This tells the virtual machine that we want to use the value `10` as an argument. It will proceed to find a procedure with the given name, that also takes an integer.

Parameters and return values are checked at run-time. With overloaded procedures, the VM will check the capture and its type(s), then choose the correct procedure (if one can be found).

If we could only write captures explicitly, we would not be able to pass our own data into procedures. A dynamic capture is what we need for the job. We will use the same example from before, but using a dynamic capture.

### Example
```
# Define a procedure that takes one value and returns none
proc my_print_proc(x : int) -> int {
    # We print X but we also return it
    x println
}

proc main() -> void {
    # Call our proc (It will print 10)
    10 
    # This denotes that we want 1 value from the stack to be passed in
    | !1 | !my_print_proc
}
```

As many languages, a `main` procedure is required for the entry point. Eso does not currently support CLI arguments, so we cannot take
any arguments OR return values.

### Conditionals
There are currently 5 conditional operators, which will push a `true` or `false` to the stack depending on the evaluation. When we use that with the `if` keyword, we can execute code based on a condition.

### Example
```
# Does 1 == 1?
1 1 = if {
    '1 = 1? True' println drop
}

1 2 > if {
    # This will not print
    '1 > 2? True' println drop
}
```

### Loops
A single type of loop exists in Eso, which essentially acts like a while loop. It will evaluate code until the top value on the stack is false.

### Example
```
# Count down from current number on the stack and print each value
proc count_down(n : int) -> void {
    n

    # We must have a boolean on the stack, since we don't have a conditional
    # we can just push a true (the loop will consume the value)
    true loop {
		# Print the current value
		println
		
		# Remove one from our index
		1 -

		# Push boolean onto the stack for the loop
		# to be evaluated
		dup 0 >
	}

	# Drop the initial value
	drop
}
```

### Bindings
A binding is a way of lifting a value from the stack and assigning it to a name. This is similar to a variable*. Once a binding is assigned, it can be used throughout the current scope*. Bindings will be removed as soon as a procedure exits. Note that the bindings will be in reverse order. Procedure parameters use bindings that are strict, this means that they cannot be rebound. All standard bindings can be overwritten, so they act somewhat like variables. As a user, you can also use strict bindings, which cannot be overwritten. You could imagine this like a constant.

### Example
```
# Create standard bindings
1 2
bind | a, b |

# Create a strict binding
3
strict | c |

# Print all values
a println drop
b println drop
c println drop

# 2 1 3
```

**Eso only has immutable values*
**There isn't an idea of a scope. However, procedures live in their own space and will have their own bindings available.*

### Style and Operation
In Eso operations are parsed and executed linearly, so there is no required syntax. This means no expected space/tab count, or line spacing etc. The next example will show two methods of writing the same code. As long as the order and logic is sound, then it will parse and run. You could technically have an entire program written in one line*.


### Example

```
proc my_proc(x : int) -> int { x 1 1 + - }
# vs
proc my_proc(x : int) -> int { 
    x 1 1 
    + - 
}
```
*Note: The examples are written in a mixture of both styles, depending on context.*


### Naming conventions
This is totally up to you! Since the only things you can name are procedures and bindings, there isn't really any issue here.

```
# A binding convention
bind | valA, valB |

# A proc convention
proc my_proc() -> void {}
```


## How it Works
Eso is a stack based language, that gives you direct access to stack manipulation. Every operation directly pushes to or pops from the stack. A series of numbers will add that value directly to the stack. This also includes operations like `+ - * /` and comparison with `> < = >= <=`.

### Example
```
# stack before [ ]
1 2 3 4 5 6
# stack after [ 1 2 3 4 5 6 ]
```


### I/O
IO in Eso is limited, it can only print values on the stack*.
- `print` Print value or string

**Input methods and other output methods are planned*


### Scopes and Stacks
Currently there is only one method of creating scopes and that's by using procedures. When a procedure is called, a new scope is created and that comes with its own stack space*. This is so you can start with a fresh stack and not interfere with other values you may not want to modify. Procedures allow for "arguments" which move values from the previous stack into the procedures' stack.

### Example
```
# Create a procedure that takes on argument and returns none
proc my_proc(a : int) -> void {
    # Print and pop the value
    println drop
}

# Push 10 onto the stack and call my_proc
# 10 will be copied into the procedure
| 10 | !my_proc drop
```

A procedure can also return values from its stack, which works the same way as arguments. As soon as the procedure ends, it will move N number of values back into the previous stack.

A stack is required to be empty, or the size of the return count (if a procedure). This means the interpreter will throw an error if the stack size is incorrect. If a proc requires an argument or set of arguments, the stack must contain that many values. This is the same for return count of procedures.

**All procedures use the same stack. Once a procedure is called, the "start" is set to the current space - arguments*


## Limitations / Restrictions / Caveats
- This is a toy language and wasn't meant to get as big as it is now. A lot has been hacked on or rewritten to support new features.
- No CLI args passed through
- No string manipulation/construction
- No file I/O
- No namespacing. Importing a file may cause conflicts with other files. I hope you're good at unique names, or using C-like naming that acts like namespacing.
- No circular dependency support or resolution when encountering a possibly valid procedure, when it hasn't been declared yet.
    - No resolution means that import order will matter. Eso does skip existing imports, if one has been encountered. This doesn't necessarily mean that file has been parsed yet, but it might have been added.