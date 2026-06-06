# jlox Architecture

`jlox` is a tree-walk implementation of the Lox language written in Kotlin. It follows the front-end/runtime split from *Crafting Interpreters*: source text is scanned into tokens, parsed into an abstract syntax tree (AST), statically resolved for lexical scope information, and then evaluated directly by walking that AST.

## Execution pipeline

```text
source file or REPL line
        │
        ▼
Lexer.scanTokens()
        │  List<Token>
        ▼
Parser.parse()
        │  List<Statement?> AST
        ▼
Resolver.resolve()
        │  lexical distances recorded in Interpreter
        ▼
Interpreter.interpret()
        │
        ▼
printed output, side effects, or reported errors
```

`Main.kt` is the command-line entry point. With no arguments it starts an interactive prompt; with one argument it reads and executes that file; with more than one argument it reports usage and exits. `Lox.kt` owns the shared `Interpreter`, orchestrates the pipeline, and centralizes compile-time and runtime error reporting.

## Directory and package map

| Path | Responsibility |
| --- | --- |
| `src/main/kotlin/Main.kt` | CLI entry point and argument handling. |
| `src/main/kotlin/Lox.kt` | Top-level runner, REPL/file modes, pipeline coordination, and error flags. |
| `src/main/kotlin/lexer/` | Token definitions and lexical scanner. |
| `src/main/kotlin/parser/` | Recursive-descent parser and AST printer helper. |
| `src/main/kotlin/construct/` | AST node definitions and visitor interfaces. |
| `src/main/kotlin/resolver/` | Static lexical-scope resolver for variables, functions, classes, `this`, and `super`. |
| `src/main/kotlin/environment/` | Runtime environment chain used for lexical scopes. |
| `src/main/kotlin/interpreter/` | Tree-walk evaluator plus runtime representations for callables, functions, classes, instances, returns, and runtime errors. |
| `src/test/kotlin/` | Kotlin tests for scanner and parser behavior. |

## Front end

### Lexer

The lexer consumes raw source text and emits a flat list of `Token` values. `TokenType.kt` defines punctuation, operators, literals, and Lox keywords such as `class`, `fun`, `for`, `if`, `return`, `super`, `this`, `var`, and `while`.

The scanner is responsible for:

- recognizing single-character and multi-character operators;
- skipping whitespace and comments;
- tracking source line numbers for diagnostics;
- parsing string and numeric literals;
- distinguishing identifiers from reserved keywords;
- appending an EOF token at the end of input.

### AST model

The AST is split into expressions and statements:

- `Expr.kt` models assign, binary, logical, grouping, literal, unary, variable, call, property get/set, `this`, and `super` expressions.
- `Statement.kt` models blocks, expression statements, prints, variable declarations, loops, functions, returns, conditionals, and classes.

Both hierarchies use the visitor pattern. This keeps the tree data classes small while allowing the parser, resolver, interpreter, and utility tools to define behavior outside the node definitions.

### Parser

The parser consumes the token stream and produces a list of statement AST nodes. It is a recursive-descent parser organized around Lox's grammar and precedence levels:

- declarations: classes, functions, variables, and ordinary statements;
- statements: block, `if`, `while`, desugared `for`, `print`, `return`, and expression statements;
- expressions: assignment, logical `or`/`and`, equality, comparison, term, factor, unary, calls/property access, and primaries.

Parse errors are reported through `Lox.error(...)`. The parser uses synchronization after errors so a single bad declaration does not necessarily prevent later code from being parsed.

## Static resolution

`Resolver` performs a pre-interpretation pass over the AST. It does not execute code; instead, it tracks lexical scopes and records how many environment hops away each local variable lives. The interpreter stores those distances in a map keyed by expression node.

This pass catches semantic errors that are easier to detect statically, including:

- reading a local variable inside its own initializer;
- declaring the same local name twice in the same scope;
- returning from top-level code;
- returning a value from an initializer;
- using `this` outside a class;
- using `super` outside a subclass;
- inheriting a class from itself.

Classes and functions are tracked with `ClassType` and `FunctionType` enums so the resolver can validate context-sensitive constructs.

## Runtime architecture

### Interpreter

`Interpreter` implements both expression and statement visitors. It evaluates expressions to Kotlin values and executes statements for side effects.

Runtime values are represented with Kotlin types:

- `Double` for Lox numbers;
- `String` for Lox strings;
- `Boolean` for Lox booleans;
- `null` for Lox `nil`;
- `LoxFunction`, `LoxClass`, and `LoxInstance` for user-defined runtime objects;
- anonymous `LoxCallable` implementations for native functions.

The interpreter owns a global environment and a current environment. Blocks, functions, and methods create nested `Environment` instances. Variable lookup uses resolver-provided lexical distances when available and falls back to the global environment for globals.

### Environments and scope

`Environment` is a linked scope frame containing a mutable map from variable names to values and an optional enclosing environment. It supports:

- `define` for creating bindings;
- `assign` for updating an existing binding through the enclosing chain;
- `get` for dynamic lookup through the enclosing chain;
- `getAt` and `assignAt` for distance-based lookup after static resolution.

### Functions and closures

`LoxFunction` wraps a function declaration, the closure environment captured at declaration time, and an initializer flag. Calling a function creates a fresh environment whose parent is the closure, binds parameters to arguments, executes the body, and returns either an explicit `return` value or `nil`.

Returns are implemented by throwing a dedicated `Return` exception internally. This unwinds nested statement execution until the call boundary catches it. Initializers always return the receiver stored as `this`, even if the body uses `return` without a value.

### Classes, instances, methods, and inheritance

`LoxClass` implements `LoxCallable`, so class names can be called like constructor functions. A call allocates a `LoxInstance`, looks for an `init` method, binds it to the new instance, and runs it. The initializer arity determines the class arity.

`LoxInstance` stores fields in a mutable map. Property lookup checks fields first, then methods. Methods are `LoxFunction` values bound to the receiver by creating a closure environment containing `this`.

Inheritance is represented by an optional superclass reference in `LoxClass`. Method lookup walks up the superclass chain. When a subclass is declared, the interpreter creates a temporary scope containing `super`, allowing `super.method()` expressions to resolve to methods on the superclass and bind them to the current instance.

## Error handling

The implementation distinguishes:

- **scan/parse/resolution errors**, which set `hadError` and cause file execution to exit with code `65`;
- **runtime errors**, which throw `RuntimeError`, print the message and line number, set `hadRuntimeError`, and cause file execution to exit with code `70`.

The REPL clears compile-time error state after each input line so one failed line does not poison the rest of the session.

## Supported language features

This implementation supports the object-oriented Lox language from the tree-walk interpreter chapters:

- literals: numbers, strings, booleans, and `nil`;
- arithmetic, comparison, equality, logical, grouping, and unary expressions;
- variable declarations and assignment;
- lexical blocks and closures;
- `if`, `while`, and `for` control flow;
- `print` statements;
- functions with parameters, return values, and closures;
- classes, fields, methods, `this`, initializers, inheritance, and `super`;
- native `clock()` function.

## Build, test, and run

From the `jlox` directory:

```sh
bash ./gradlew test
bash ./gradlew build
```

The command-line entry class is `org.zakat.MainKt`. Launch it with one script path argument for file mode, or with no script path for REPL mode.

## Design tradeoffs

- The AST is interpreted directly, which keeps the implementation approachable but makes execution slower than bytecode.
- Lexical resolution is separated from interpretation, so most local variable access can use precomputed distances instead of repeated name-chain searches.
- Kotlin exceptions are used for Lox returns and runtime errors, simplifying unwinding at the cost of using exceptions for control flow inside the interpreter.
- Runtime values rely on Kotlin's type system rather than a custom tagged union.
