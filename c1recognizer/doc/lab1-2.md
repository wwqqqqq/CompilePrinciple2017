#Problems encountered
1. The EBNF expression **CompUnit -> [ CompUnit ] ( Decl | FuncDef )** is left recursive grammar. Using it directly will cause an antlr4 error:  
**error(119): C1Parser.g4::: The following sets of rules are mutually left-recursive [compilationUnit]**  
Through analyzing, we know that compilation unit is a string of one or more declaration or function definition. So we can change the definition of compilation unit to **compilationUnit: (decl | funcdef)+;**
#Design
The C1Parser.g4 program is written according to EBNF and g4 grammar, except the compilationUnit because of the problem I refered to in the last section.
#Test cases and corresponding analysis
1. All provided test cases run correctly using my C1 lexer and parser.
2. mytest1.c1  
`int main(void)  
{
    i=3;
}`  
The result of grun C1 compilationUnit is  
**line 1:8 mismatched input '(' expecting {',', ';'}  
line 1:13 mismatched input ')' expecting Identifier**  
The message and the parser tree the program generates indicates that it marks the main function as a var declaraction because of the 'int' token, and when the program encounters the 'void' token, it marks the following as a function definition. The EBNF grammar only allows function in the form **void func_name()**, so the expression **int main(void)** can only lead to a wrong parser tree.
3. mytest2.c1  
`int i = ++ 5;  
int j = +(3 -+ 5);  
int k = 3 +- 5;  
int l = -3 *- 5;  
int m = 1 + 2 * 3;  
int n = 1 + 2 / 3 * (4 - 5);`  
The parser tree shows that the unary operation '+' and '-' have higher priority than binops. And they are right associative operations. However, the expression `-3*-5` is identified as `-(3*(-5))`, instead of `(-3)*(-5)`. The priority of '\*' '/', and '%' is higher than '+' and '-'(as binop).
4. mytest3.c1  
`void main()
{
    int i=5,j;
    while(i>0){
        if(i>4) j=1;
        else if(i>2) j=2;
        else j=3;
    }
}`  
The parser tree shows that the grammar only match the 'if' and the first 'else' following it, whichi means the 'if' token in 'else if' will be considered as a part of the second stmt in **If LeftParen cond RightParen stmt Else stmt**.
5. mytest4.c1  
`void main()
{
    int i=1,j;
    int a[]={};
    while(i);
    j=1+2
}}`  
The messages:  
**line 4:10 mismatched input '}' expecting {'(', '+', '-', Identifier, Number}  
line 5:8 mismatched input ')' expecting {'==', '!=', '<', '>', '<=', '>='}  
line 7:0 missing ';' at '}'**  
From the messages and the parser tree, we can learn that the bugs such as missing ';' and empty array in the assignment expression will be detected. However, the last '}' in the test program, which is redundant, will be ignored, instead of raising an error. And in the expression `while(i);`, which is corresponded to the grammar **while '(' Cond ')' Stmt**, cond is directly parsed as **exp** instead of **exp relop exp** which it is supposed to be. As the definition, the single exp 'i' cannot be parsed as cond, but no error message is delivered here.