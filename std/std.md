# Std Library Features
*Note: The standard library is slowly growing and is not guaranteed to be stable/include the same macros or procedures every patch.*

### Wrappers
The standard contains a few macros that wrap around single-character tokens. This can make code a bit easier to read.

Misc
- `print` => `.`
- `print-char` => `,`
- `swap` => `&`

Maths
- `add` => `+`
- `sub` => `-`
- `mul` => `*`
- `div` => `/`

### Macros
Macros are used when a scope would not be necessary. Std macros generally handle no items on the stack, so you don't have to worry if you call a macro without items on the stack.

- `square` => Takes the top of the stack and multiplies it by itself
- `print-stack-drop` => Prints all items on the stack, dropping each item
- `drop-stack` => Drops all items on the stack


### Procedures
If a seperate stack is important and we would like to modify data without modifying the stack, we can use procedures.

- `count_down` 1 arg, any return => Counts down from top of stack, pushes each to the stack
- `double` 1 arg, 1 return => Returns the top of the stack multiplied by 2
- `slope_input_print` 1 arg, 0 return => Takes the top of the stack and prints N-1 -> 0 -> N eg. 2 1 0 1 2 3