lexer grammar C1Lexer;

tokens {
    Comma,
    SemiColon,
    Assign,
    LeftBracket,
    RightBracket,
    LeftBrace,
    RightBrace,
    LeftParen,
    RightParen,
    If,
    Else,
    While,
    Const,
    Equal,
    NonEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    Plus,
    Minus,
    Multiply,
    Divide,
    Modulo,

    Int,
    Void,

    Identifier,
    Number
}
fragment Other1: ~'/';
fragment Other2: ~('/'|'*');
Comment1: '/*' '/'* (Other1* Other2 '/'+)* Other1* '*/' -> skip; 
fragment Other3: ~'\n';
Comment2: '//' (Other3|('\\''\n'))* '\n' -> skip;

Comma: ',';
SemiColon: ';';
Assign: '=';

LeftBracket: '[';
RightBracket: ']';
LeftBrace: '{';
RightBrace: '}';
LeftParen: '(';
RightParen: ')';

If: 'if';
Else: 'else';
While: 'while';
Const: 'const';

Equal: '==';
NonEqual: '!=';
Less: '<';
Greater: '>';
LessEqual: '<=';
GreaterEqual: '>=';

Plus: '+' ;
Minus: '-' ;
Multiply: '*' ;
Divide: '/' ;
Modulo: '%' ;

Int: 'int';
Void: 'void';

Identifier: ('_'|[a-zA-Z]) ('_'|[0-9a-zA-Z])*;
Number: [0-9]+ | (('0x'|'0X') [0-9a-fA-F]+) ;

WhiteSpace: [ \t\r\n]+ -> skip;
