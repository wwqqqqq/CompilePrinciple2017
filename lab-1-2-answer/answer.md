* ANTLR容许哪些类型的左递归?  
产生式中可以出现左递归，但出现左递归的标识符必须存在非左递归的匹配规则。  
* ANTLR对所支持的左递归如何处理？例如，对下面两种情况分别会怎样解析？  
	* e : e '\*' e # Mult | e '+' e # Add | INT # int ;  
	* e : e '+' e # Add | e '\*' e # Mult | INT # int ;  
在出现左递归的情形下，运算符优先级按从上到下依次降低。默认的结合性为左结合。  
故MultFirst文法中，'\*'的优先级高于'+'，PlusFirst则相反。  
测试程序中，对同样的表达式分别按照MultFirst文法和PlusFirst文法进行计算。  
Expression: 1+2\*3  
MultFirst value: 7[=1+(2\*3)]  
PlusFirst value: 9[=(1+2)\*3]  
Expression: 2\*3+4  
MultFirst value: 10[=(2\*3)+4]  
PlusFirst value: 14[=2\*(3+4)]  
Expression: 1+2\*3\*4+5\*6  
MultFirst value: 55[=1+((2\*3)\*4)+(5\*6)]  
PlusFirst value: 486[=(1+2)\*3\*(4+5)\*6]  
* 给出ANTLR不支持的左递归文法的例子并尝试分析原因  
参见UnsupportedLeftRecursive.g4:  
`grammar UnsupportedLeftRecursive;

Identifier: [a-zA-Z_] [a-zA-Z_0-9]*;
Number: [1-9][0-9]*;
Plus: '+';
Multiply: '*';
WhiteSpace: [ \t\n\r]+ -> skip;

expr:
	expr Multiply expr  # Mult
	| expr Plus expr  # Plus
	;`  
这种情况下，由于expr的所有产生式均为左递归文法，此时antlr4 UnsupportedLeftRecursive.g4会产生如下错误提示：  
error(147): UnsupportedLeftRecursive.g4:9:0: left recursive rule expr must contain an alternative which is not left recursive  
