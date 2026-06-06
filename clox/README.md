# clox Architecture

`clox` is a bytecode virtual-machine implementation of the Lox language written in C. Unlike `jlox`, it does not keep and walk an AST at runtime. Instead, source code is scanned and compiled in one pass into bytecode stored in chunks, then executed by a stack-based VM.

## Execution pipeline

```text
source file or REPL line
        │
        ▼
scanner.c
        │  Token stream pulled by compiler
        ▼
compiler.c
        │  ObjFunction containing Chunk bytecode + constants
        ▼
vm.c
        │  ObjClosure wrapping the top-level function
        ▼
stack-based bytecode execution
        │
        ▼
printed output, side effects, or reported errors
```

`src/main.c` provides the command-line interface. With no arguments it runs a REPL; with one argument it reads and executes the file; compile errors exit with code `65`, runtime errors with code `70`, and command-line/file errors with the conventional codes used by the book.

## Directory and module map

| Path | Responsibility |
| --- | --- |
| `CMakeLists.txt` | CMake project definition and executable target. |
| `src/main.c` | CLI, REPL, file reading, process exit behavior. |
| `src/scanner.c`, `src/scanner.h` | Lexical scanner that produces tokens on demand. |
| `src/compiler.c`, `src/compiler.h` | Pratt parser and single-pass bytecode compiler. |
| `src/chuck.h`, `src/chunk.c` | Bytecode chunk storage, constants, opcodes, and compressed line metadata. |
| `src/vm.h`, `src/vm.c` | Global VM state, operand stack, call frames, native functions, bytecode dispatch, and runtime errors. |
| `src/value.h`, `src/value.c` | Tagged `Value` representation and dynamic arrays of values. |
| `src/object.h`, `src/object.c` | Heap object model for strings, functions, closures, natives, and upvalues. |
| `src/table.h`, `src/table.c` | Open-addressed hash table used for globals and string interning. |
| `src/memory.h`, `src/memory.c` | Allocation helpers, resizing, and heap-object cleanup. |
| `src/debug.h`, `src/debug.c` | Bytecode disassembly helpers used by debug tracing/printing flags. |
| `src/common.h` | Common includes and compile-time debug flags. |

> Note: the bytecode files are named `chuck.*` in this repository, but they define the `Chunk` abstraction from the book.

## Front end

### Scanner

The scanner owns the source pointer state and emits one token at a time to the compiler. It recognizes Lox punctuation, operators, literals, identifiers, comments, keywords, and EOF. Scanner errors are represented as `TOKEN_ERROR` tokens so the compiler can report them through its normal diagnostic path.

### Pratt parser and compiler

`compiler.c` combines parsing and code generation. It uses Pratt parse rules for expressions, with each token type optionally having prefix and infix parse functions plus a precedence level. This directly emits bytecode for expression evaluation order without building an intermediate AST.

The compiler handles:

- declarations for functions and variables;
- statements for blocks, `if`, `while`, `for`, `print`, `return`, and expression statements;
- expressions for assignment, logical `and`/`or`, equality, comparison, arithmetic, unary operations, calls, literals, variables, and grouping;
- local scopes and local slot allocation;
- closure capture through upvalue resolution;
- jump patching for branches and loops;
- panic-mode synchronization after compile errors.

`class` is tokenized as a keyword and used for synchronization, but class declarations, instances, methods, fields, `this`, and `super` are not implemented in the VM object model in this repository.

## Bytecode representation

`Chunk` stores:

- a dynamically sized byte array of opcodes and operands;
- a constants array for literals and referenced heap objects;
- compressed source-line metadata stored as line/count pairs.

The opcode set includes constants, literals, arithmetic, comparisons, logical negation, printing, stack pops, global/local/upvalue get and set, jumps, loops, calls, closures, returns, and upvalue closing.

Constants are stored as `Value` entries. Bytecode operands use one-byte constant indices and local/upvalue indices, so individual functions are limited to 256 constants, locals, parameters, and captured variables where those operands are used.

## Runtime value and object model

### Values

`Value` is a tagged union supporting:

- booleans;
- `nil`;
- double-precision numbers;
- heap object references.

Object values are identified with `ObjType` and accessed through macros such as `IS_STRING`, `AS_CLOSURE`, and `AS_NATIVE`.

### Heap objects

The heap object hierarchy includes:

- `ObjString`: interned strings with cached FNV-1a hashes;
- `ObjFunction`: compiled function metadata, arity, upvalue count, name, and chunk;
- `ObjClosure`: runtime closure containing an `ObjFunction` plus captured upvalue references;
- `ObjUpvalue`: a captured variable that points either to an open stack slot or to a closed heap value;
- `ObjNative`: wrapper for C native functions.

All heap objects are linked through `vm.objects` so `freeObjects()` can walk and release them when the VM shuts down. This implementation frees objects at shutdown rather than implementing a tracing garbage collector.

### Tables and strings

`Table` is an open-addressed hash table with linear probing and tombstones. It is used for:

- `vm.globals`, mapping interned global variable names to values;
- `vm.strings`, interning strings so equal strings share one `ObjString` allocation.

String interning makes equality checks and global lookups cheaper because table keys can be compared by pointer once interned.

## Virtual machine

The VM is stack based. `VM` contains:

- a fixed-size value stack;
- an array of call frames;
- global variables;
- the interned string table;
- the linked list of open upvalues;
- the linked list of all heap objects.

Each `CallFrame` stores the closure being executed, an instruction pointer into that closure's bytecode, and a pointer to the first stack slot for that call. Function calls push a frame, verify arity, and reuse the value stack for parameters and locals.

The dispatch loop in `run()` repeatedly reads an opcode and performs the operation. Important execution patterns include:

- arithmetic opcodes pop operands and push results;
- `OP_ADD` concatenates two strings or adds two numbers;
- globals live in `vm.globals` and locals live in stack slots;
- `OP_JUMP_IF_FALSE`, `OP_JUMP`, and `OP_LOOP` implement conditionals and loops;
- `OP_CALL` dispatches to closures or native functions;
- `OP_CLOSURE` creates closures and wires up each captured upvalue;
- `OP_CLOSE_UPVALUE` moves captured locals from stack slots into heap storage;
- `OP_RETURN` pops a frame, restores the caller frame, and leaves the return value on the caller's stack.

## Closures and upvalues

The compiler resolves names in three tiers: locals in the current compiler, upvalues captured from enclosing compilers, and globals. When a nested function captures a local, the enclosing compiler marks that local as captured and the nested function records an upvalue descriptor.

At runtime, `captureUpvalue()` maintains `vm.openUpvalues` as an ordered linked list of variables still living on the stack. `closeUpvalues()` copies stack values into `ObjUpvalue.closed` when locals go out of scope or a function returns. Closures then keep those values alive independently of the stack frame that created them.

## Native functions

`initVM()` registers native functions in the global table. The current native surface includes:

- `clock()`: returns elapsed CPU time as a number;
- `len(value)`: returns the length of a string and reports a runtime error for unsupported argument types.

Native calls use the same call protocol as Lox functions: arguments are already on the VM stack, the native receives an argument count and pointer to the first argument, and the VM replaces the callee plus arguments with the result.

## Error handling and diagnostics

Compile-time errors are reported by the compiler with source line information. The compiler uses panic mode to avoid cascaded errors and synchronization points to resume at likely declaration boundaries.

Runtime errors are reported with formatted messages and a stack trace built from active call frames. The VM resets the operand stack after a runtime error so the next REPL input can start from a clean state.

Optional debug flags in `common.h` can enable bytecode disassembly after compilation or instruction-by-instruction execution tracing.

## Supported language features

This VM currently supports the procedural and functional subset of Lox:

- literals: numbers, strings, booleans, and `nil`;
- arithmetic, comparison, equality, logical, grouping, and unary expressions;
- global variables, local variables, and assignment;
- lexical blocks;
- `if`, `while`, and `for` control flow;
- `print` statements;
- functions, returns, calls, recursion, closures, and captured variables;
- native functions `clock()` and `len()`.

The object-oriented Lox features from later chapters are not present in this `clox` implementation: classes, instances, fields, methods, initializers, inheritance, `this`, and `super`.

## Build and run

From the `clox` directory:

```sh
cmake -S . -B build
cmake --build build
./build/CLox program.lox
```

Run `./build/CLox` without a script path to start the REPL.

## Design tradeoffs

- Compilation is single pass and bytecode-oriented, which makes the implementation compact but requires forward jumps to be patched after their target positions are known.
- The VM uses fixed maximums for call frames and stack slots, keeping memory management simple while imposing practical program-size limits.
- Values are explicit tagged unions and objects are manually allocated, giving C-level control over representation.
- Objects are freed at VM shutdown. There is no incremental or tracing garbage collection in this repository.
- The stack VM is more complex than `jlox`'s tree-walk interpreter but avoids repeatedly traversing AST nodes and is closer to production interpreter architecture.
