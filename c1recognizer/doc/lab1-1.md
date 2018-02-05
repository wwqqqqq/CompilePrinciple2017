#Lab1-1 Report
##Analysis
1. Format: Token: <definition\>;
2. The name of a token should begin with a capital letter.
3. Characters between a pair of '/\*' and '\*/' are considered as block comment, if there is no other '\*/' between them.
4. A line comment is begun with '//' and ended with the first '\n' without any '\' before it.
5. Comments and whitespace will be ignored when processing a program. In .g4 file, use '-> skip' to describe comments and whitespace, such that strings identified as comments and whitespace will be not be treated as a token.
6. An identidier is a string that begins with '\_' or an alphabetical letter, consists of '\_'s, letters or numbers, and is not any reserved words for c1 language such as 'int' and 'if'.
7. A hexadecimal number should begin with '0x' or '0X'. It does not matter whether using lowercase or highercase letter in a hexadecimal notation.
8. Bracket: []   Brace: {}   Paren: ()

##Design
1. Block Comment:  
    * **fragment** Other1: ~'/';
    * **fragment** Other2: ~('/'|'\*');
    * BlockComment: '/\*' '/'\* (Other1\* Other2 '/'+)\* Other1\* '\*/' -> skip;
2. Line Comment:  
    * LineComment: '//' ( (~'\n') | ('\\\n') )\* '\n' -> skip;
3. Identifier: 
    * Identifier: ('\_'|[a-zA-Z]) ('\_'|[0-9a-zA-Z])\*;
4. Number:  
    * Number: [0-9]+ | (('0x'|'0X') [0-9a-fA-F]+);

##Testing Report
1. All tokens are identified correctly in all test files under directory ../test/test_cases.
2. When meeting comments like this:  
    /\*  
    //line comment \*/  
    emmmm \*/  
    (This can be tested using test1.c1 under directory ../test/test_cases/lexer)  
    In this case, the program will consider '/\*\n\r//line comment \*/' as a block comment, 'emmmm' as an identifier, and '\*/' after 'emmmm' as Mutiply and Divide tokens.
3. In testing program test2.c1:  
    const int if1=1;  
    int ifx=0x235ab;  
    int else3=if1;  
    In this case, 'if1', 'ifx' and 'else3' will all be interpreted as identifiers although they contain strings 'if' and 'else'.
4. test3.c1 gives a more general test about block comment and line comment:  
    int /\*/\*\*\*\*/i=5;int/\*  
    //jfafajlj\*/i=3;  
    i=1//\\\\\  
    ;\  
    ;\  
    ;\  
    ;  
    \+  
    //  
    5;  
    The string '/\*/\*\*\*\*/', and string '/\*\n\r//jfafajlj\*/' are identidied as block comments. And string '//\\\\\\n\r;\\n\r;\\n\r;\\n\r;' after 'i=1' before '+' is identified as a line comment.  
    So the program is like this ignoring comments:  
    int i=5; int  
    i=3;  
    i=1  
    \+  
    5;  