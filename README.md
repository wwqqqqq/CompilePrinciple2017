# CompilePrinciple2017
Experiments in 'Compile Principle' course in 2017 fall.
----

## What is C1 language?

C1 Language in this course is a subset of C Programming Language. It doesn't include holistic type systems. 
Only `integer` and `array of integers` and are supported, and they can be described with `const`. 
Functions in C1 do not have any parameters or return value.

The grammar can be describe in EBNF (Extended Backus-Naur Form) as below:  
```
CompUnit    → [ CompUnit ] ( Decl | FuncDef ) 
Decl        → ConstDecl | VarDecl
ConstDecl   → const int ConstDef { , ConstDef } ';'
ConstDef    → ident '=' Exp | ident '[' [ Exp ] ']' '=' '{' Exp { ',' Exp } '}'
VarDecl     → int Var { , Var } ';'
Var         → ident | ident '[' Exp ']' | ident '=' Exp
            | ident '[' [ Exp ] ']' '=' '{' Exp { ',' Exp } '}'
FuncDef     → void ident '(' ')' Block
Block       →'{' { BlockItem } '}'
BlockItem   → Decl | Stmt
Stmt        → LVal '=' Exp ';' | ident '(' ')' ';' | Block 
            | if '( Cond ')' Stmt [ else Stmt ] | while '(' Cond ')' Stmt | ';'
LVal        → ident | ident '[' Exp ']'
Cond        → Exp RelOp Exp
RelOp       →'==' | '!=' | '<' | '>' | '<=' | '>='
Exp         → Exp BinOp Exp | UnaryOp Exp | '(' Exp ')' | LVal | number
BinOp       → '+' | '−' | '*' | '/' | '%'
UnaryOp     → '+' | '−'
```

## C1 Recognizer
Generate AST for each program written in C1 language. Prerequest and usage can be seen in 
CompilePrinciple2017/C1recognizer/README.md

The grammar is described using ANTLR.

[Reference](https://clarazhang.gitbooks.io/compiler-f2017/content/parser.html)

## C1 Interpreter:
Genrate a CLI tool, which is capable of compiling C1 code into LLVM IR, print it and execute it.

Using the AST generated with C1recognizer, and LLVM IRBuilder.

How to compile and install: 
``` shell
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DLLVM_DIR=/your/llvm/installation/lib/cmake/llvm -Dc1recognizer_DIR=/your/c1recgonizer/installation/cmake ..
cmake -DCMAKE_INSTALL_PREFIX=/your/install/prefix ..
make install
make -j
```
After these steps, an executable program `c1i` will be generated under the build directory. `c1i` can accept one input file, and generate
the LLVM IR code of it. If parameter `-emit-llvm` is used, the program will print the generated code; otherwise, it will execute the input program.

[Reference](https://clarazhang.gitbooks.io/compiler-f2017/content/llvmIRGen.html)
