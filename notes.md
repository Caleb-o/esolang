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


## Tests:
There should be a test flag which allows parsing and running tests. A polymorphic Runner type should be
available, which can be switched out with the interpreter. All ops should become their own function in
the base runner (virtual, but not pure virtual), so we can have one definition of the function. The 
test runner would just be the Runner, with some new functions to run all tests and track success/failures.

Based on the defined virtual functions, the interpreter could just be the runner, with some slight modifications.


## Native Procedures:
These will be procedures that are automatically added before the parser starts. They are not in bytecode form, they are just
identifiers that are linked to a function (similar to how the VM works). To distinguish them from the regular procedures,
there will be a different symbol before the call. They also live in a seperate map, so they do not touch the procedures.

NATIVE_CALL IDX

eg.
```
# 5 will be pushed to the stack, this does not consume hello
'hello' @str_len
```

### List of Native Procedures:
* [ ] `write_file` : Writes to a file, using the previous string as a file path and the string prior as the buffer


### Top-level code
If we simply ran the VM top to bottom, we would run into procedures and run their code, which is not desired. One solution is, we capture all top-level code and insert it all at the bottom and set the ip to the start of the top-level code.

Issue: If main exists, do we jump to main? That would mean the top-level code is never ran.
Solution: We always take precedence over top-level code position, and run from top level. We would have to insert a GOTO with the index of main for cases where the top-level code ends (if main exists), this will stop it from running random code from procedures.