#lab1-3 report

##遇到的问题
1. `error: call of overloaded 'shared_ptc(antlrcpp::Any)' is ambiguous`   
错误原因为visit()后没有.as<>()，修改后该错误解决。
1. 段错误（核心已转储）  
错误原因：对指针的不当使用。排查后发现是某个visit函数没有返回result，导致调用它时得到空指针，进而导致segment fault。
1. bad cast错误  
错误原因：shared_ptr和普通指针之间的直接转换。

##分析与设计
参照示例程序中`antlrcpp::Any syntax_tree_builder::visitExp(C1Parser::ExpContext *ctx)`函数的部分实现。
1. 首先判断规约到父节点时，使用了哪一种产生式。用if语句即可简单实现。  
例如，所有左部为var(variable)的产生式有：    
vardef:   
       Identifier  
	   | Identifier LeftBracket exp RightBracket  
	   | Identifier Assign exp  
	   | Identifier LeftBracket (exp)? RightBracket Assign LeftBrace exp (Comma exp)* RightBrace  
为了判断变量的类型（整型或整型数组，是否赋值），首先需判断规约为var时使用了哪种产生式。若if(ctx->Assign())成立，则该语句在声明变量的同时，给该变量赋值；若if(ctx->LeftBracket())成立，则该变量为整型数组，否则为整型数。
1. 对于产生式`Identifier LeftBracket (exp)? RightBracket Assign LeftBrace exp (Comma exp)* RightBrace`，如何判断数组大小(array_length)是否显式声明？若没有，怎么计算数组大小？    
对于arr[]={exp,exp,...,exp}，非终结符exp的个数=终结符Comma的个数+1， 对于arr[exp]={exp,exp,...,exp}，非终结符exp的个数=终结符Comma的个数+2。  
num_exp = (ctx->exp()).size();  
num_Comma = (ctx->Comma()).size();  
这样便能解决问题，判断数组大小了。
1. 文件`c1recognizer/include/c1recognizer/syntax_tree.h`中，存放有各种syntax\_tree\_nod的结构体，根据产生式和syntax\_tree\_node的数据结构，利用
`result->line = ctx->getStart()->getLine();`  
`result->pos = ctx->getStart()->getCharPositionInLine();`  
得到行数和列数。对于非终结符T，使用`T->getSymbol()->getCharPositionInLine()`得到它的列数，使用`T->getSymbol()->getText()`得到它的文本。再对这些数据结构进行合理的处理即可。完成后返回一个指定类型的antlrcpp::Any(shared_ptr)类型的返回值。
1. 对于visitVardecl(ctx)和visitConstdecl(ctx)函数，它们需要返回的并不是syntax_tree.h中定义的任何一种结构体的指针，而是一个指向容器的指针。  
`auto result = new ptr_list<var_def_tmt_syntax>`  
然后遍历`vardecl -> int vardef, vardef, ... , vardef;`中的所有vardef：对每一个vardef，调用visit函数，返回一个`var_def_tmt_syntax *`类型的指针，然后将返回值存入result所指向的vector中。  
visitDecl()函数对visitVardecl()和visitConstdecl()函数的返回值不做任何处理，直接返回。故只有visitcompilationUnit()函数和visitBlock()函数对visitVardecl()和visitConstdecl()的返回值进行处理。  
调用visitDecl时需注意.as<>()中的类型：
`auto var_list = visitDecl(declaration).as<ptr_list<var_def_stmt_syntax> *>();`  
然后对\*var\_list进行遍历。以visitcompilationUnit()函数中的操作为例，将var\_list的每个元素转换为ptr\<global_def_syntax\>，再存入result->global_defs容器中。

##问题回答
1. ANTLR分析树的编程接口与在lab1-2中写的文法之间的关系；  
对于结点非终结符s，C1Parser::SContext类型的指针ctx，为指向一个非终结符s的结点。对非终结符x或终结符T，调用ctx->x()和ctx->T()即可得到ctx的孩子中所有x或T对应的结点，若不存在，则返回nullptr。
1. ATN的英文全称是什么，它代表什么？  
Augmented Transition Network  
扩充转移网络文法，即扩充转移网络(Augmented Transition Network)，简称ATN，是美国哈佛大学人工智能专家伍兹(W. Woods)于1969年提出的。ATN是自然语言语法的一种多功能表示及语言自动分析的一种崭新的方法，ANT对自然语言处理领域产生了重要影响，并曾成功地应用于有限领域的问题应答系统中，如LUNAR程序。  
ATN就是用上下文无关文法的扩充来实现上下文相关文法，它用一组寄存器存放语法分析信息，每走一步都要测试一下当前情况，并根据测试结果决定做什么动作，最后把寄存器中的信息综合起来，即得到被分析句子的语法结构。
1. SLL的英文全称是什么，它与LL、以及ALL(*)的区别是什么？它们分别怎么对待文法二义、试探回溯？  
Simple LL
(LL: **L**eft to Right, **L**eftmost Derivation)
SLL是化简版的LL，是LL文法的子集。它不需要进行回溯，但可使用的范围要比LL小很多。LL(1)与SLL(1)等价。  
1. 了解并总结enterRecursionRule、unrollRecursionContexts、adaptivePredict函数的作用、接口和主要处理流程。  
public void enterRecursionRule(ParserRuleContext localctx,
                      int state, 
                      int ruleIndex
                      int precedence)  
public void unrollRecursionContexts(ParserRuleContext _parentctx)  
public int adaptivePredict(TokenStream input,
                  int decision,
                  ParserRuleContext outerContext)
1. 错误处理机制  
首先修改C1Parser.g4，使之可以识别形如const identifier;的语句而不报错。虽然实际上是错误的，但将报错放在syntax\_tree\_builder.cpp中处理。  
再syntax\_tree\_builder.cpp中，visitConstdecl()函数中，若`if(!ctx->Int())`成立，则进行报错，但解析照常进行。
`err.warn(Const->getSymbol()->getLine(),Const->getSymbol()->getCharPositionInLine(),"const declaration with \"int\"");`  
这样以来，对于有这样错误的源程序，解析器会在开头进行错误提示，再显示解析结果。