# esolang
Simple esolang written in Python


## What is it?
This is a bytecode interpreted, stack-based language that I've written in Python. I have been reading about and exploring new languages, so I decided that I would like to develop one of my own. Initially, it was going to be something similar to BrainF*ck but using numbers and words to have a better visualisation. I came across Tsoding's Porth language, which is also a stack-based language written in Python and I've taken a lot of inspiration from it. I don't know much about compilation/assembly, so my version is interpreted.


## Language Spec

### Comments
Coments use the '#' and are single line only.
```
# This is a comment
1 2 3 4
```

### Types
Eso has two types currently:
- `int` : int, int literal
- `const string` : string literal

Only integers can be created from within the code, so there isn't an easy way to construct a string using code*


### Example
```
# String literal
'This is a string'
# Int literal
1 2 3 4 5 6
```
**You could emulate a string using an operator and integers, which will print a character*

### Operators
- `+ - * /` Mathmatical operators
- `= < >` Comparison
- `.` Print current value on stack / Print string
- `,` Print current value on stack as a char / Print string as ascii codes
- '&' Swaps the top two numbers on the stack
- '[' Start of a loop
- ']' End of a loop
- `;` (Optional) end of statement
- `!` invoke macro

### Example
```
1 2 +
# Create a loop, with an optional semicolon
[ 1 - . ];
# Push a two and swap with the last value
2 &
```

### Keywords
- `if` Checks if the current value on the stack is `0` or `1`
- `dup` Duplicates the current value on the stack
- `pop` Pops the current value from the stack
- `macro` Starts a macro
- `proc` Starts a procedure
- `undef` Undefines a proc
- `end` End of a if/proc/macro block
- `impl` Import statement
- `break` exit out of a loop

### Example
```
# Check if 1 == 1 and print hello if true
1 1 = if
    'Hello!' .
end
```

### Impl
Impl is used to import other eso scripts. It will import the entire script including macros and procs.

```
impl 'std.experimental'

# Call a macro from the std experimental script
# Here the print macro wraps the dot '.' operator
10 !print
```

### Macros
An eso macro is a way to copy-paste code without having to write specific operations everywhere. They are currently used to make some operators into callable tokens in the std library. Since we may want to use the same macro with different code, we can also undefine it.

### Example
```
macro one_plus_one;
    1 1 +
end

# Call our new macro
!one_plus_one

# Undefine the macro
undef one_plus_one;

# We can now create a new one_plus_one macro :^)
```

### Procedures
Similar to macros, we can call blocks of code using a single token. Unlike a macro, we can tell the procedure to accept or return a certain amount of values. These values follow the name and are called the argument count and return count. An error will occur if it cannot be provided with or return the correct amount of values.

### Example
```
# Define a procedure that takes one value and returns none
proc my_print_proc 1 0;
    .
end

# Call our proc (It will print 10)
10 my_print_proc
```
*Note: Procedures are currently expected to end with the correct amount of values. So when it returns, it will copy the first N amount of values back to the previous stack, instead of from the end.*

### Conditionals
There are currently three conditional operators, which will push a `0` or `1` to the stack depending on the evaluation. When we use that with the `if` keyword, we can execute code based on a condition.

### Example
```
# Does 1 == 1?
1 1 = if
    pop
    '1 = 1? True' .
end

# Is 1 > 2?
1 2 > if
    pop
    # This will not print
    '1 > 2? True' .
end
```

### Loops
A single type of loop exists in Eso, which essentially acts like a while loop. It will evaluate code until the current value on the stack reaches `0`. There is an example macro of a count down within std experimental.

### Example
```
# Count down from current number on the stack and fill the stack
# with descending numbers
macro count-down; 
    [ dup !swap !print 1 !sub ]
    pop
end
```

### Style and Operation
In Eso operations are parsed and executed linearly, so there is no required syntax. This means no expected space/tab count, or line spacing etc. The next example will show two methods of writing the same code. As long as the order and logic is sound, then it will parse and run. You could technically have an entire program written in one line*.


### Example

```
macro my_macro; 1 1 1 + - end
# vs
macro my_macro;
    1 1 1
    + 
    -
end
```
*Note: The std and examples are written in a mixture of both styles, depending on context.*

**The repl currently only works using a single line, entering code will evaluate that single line and will forget previous entries.*


## How it Works
Eso is a stack based language, that gives you direct access to stack manipulation. Every operation directly pushes to or pops from the stack. A series of numbers will add that value directly to the stack. This also includes operations like `+ - * /` and comparison with `> < =`.

*Note: The only value that never touches the stack is a string literal, it is stored in global space.*
### Example
```
# stack before []
1 2 3 4 5 6
# stack after [ 1 2 3 4 5 6 ]
```

### Scopes and Stacks
Currently there is only one method of creating scopes (which have their own stack) and that's by using procedures. When a procedure is called, a new scope is created and that comes with its own stack. This is so you can start with a fresh stack and not interfere with other values you may not want to modify. Procedures allow for "arguments" which copy values from the previous stack into the procedures' stack.

### Example
```
# Create a procedure that takes on argument and returns non
proc my_proc 1 0;
    # Print and pop the value
    . pop
end

# Push 10 onto the stack and call my_proc
# 10 will be copied into the procedure
10 my_proc
```

A procedure can also return values from its stack, which works the same way as arguments. As soon as the procedure ends, it will copy N number of values back into the previous stack.

### Example
```
# Create a procedure that takes one argument and returns one
proc plus_one 1 1;
    1 +
end

# Pass 20 to plus_one and print return value
20 plus_one .
```