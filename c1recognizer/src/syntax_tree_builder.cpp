
#include "syntax_tree_builder.h"
#include <memory>

using namespace c1_recognizer;
using namespace c1_recognizer::syntax_tree;

syntax_tree_builder::syntax_tree_builder(error_reporter &_err) : err(_err) {}

antlrcpp::Any syntax_tree_builder::visitCompilationUnit(C1Parser::CompilationUnitContext *ctx)
{
    //compilationUnit:  (decl | funcdef)+ ;
    //assembly
    //ptr_list<global_def_syntax> global_defs;
    auto result = new assembly;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    int i,num;
    auto declaration = ctx->decl();
    num = declaration.size();
        for(i=0;i<num;i++)
        {
	    auto dec = visitDecl(declaration[i]).as<ptr_list<var_def_stmt_syntax> *>();
            //std::shared_ptr<ptr_list<var_def_stmt_syntax>> dec = visitDecl(declaration[i]);
	    int j,length;
	    length = dec->size();
	    for(j=0;j<length;j++)
            {/*
		var_def_stmt_syntax *temp= new var_def_stmt_syntax;
		*temp = *(*dec)[j];
		result->global_defs.emplace_back(static_cast<global_def_syntax *>(temp));*/
		ptr<global_def_syntax> temp = (*dec)[j];
	        result->global_defs.emplace_back(temp);
	    }
        }
    auto function = ctx->funcdef();
    num = function.size();
        for(i=0;i<num;i++)
        {
            auto func = visitFuncdef(function[i]).as<func_def_syntax *>();
            result->global_defs.emplace_back(static_cast<global_def_syntax *>(func));
    
    }
    
    return result;
}

antlrcpp::Any syntax_tree_builder::visitDecl(C1Parser::DeclContext *ctx)
{
    // decl -> constdecl | vardecl ;
    if(auto cd = ctx->constdecl())
        return visitConstdecl(cd);
    else
    {
        auto vd = ctx->vardecl();
	//auto temp = visitVardecl(vd);
        return visitVardecl(vd);
    }
}

antlrcpp::Any syntax_tree_builder::visitConstdecl(C1Parser::ConstdeclContext *ctx)
{
    // constdecl: Const Int constdef (Comma constdef)* SemiColon ;
    auto var = ctx->constdef();
    int var_num = var.size();
    int i;
    auto Const = ctx->Const();
    auto variables = new ptr_list<var_def_stmt_syntax>;
    if(!ctx->Int())
	err.warn(Const->getSymbol()->getLine(),Const->getSymbol()->getCharPositionInLine(), "const delaration without \"int\"");
    for(i=0;i<var_num;i++)
        variables->emplace_back(visitConstdef(var[i]).as<var_def_stmt_syntax *>());
    return variables;////
}

antlrcpp::Any syntax_tree_builder::visitConstdef(C1Parser::ConstdefContext *ctx)
{
    auto ident = ctx->Identifier();
    auto result = new var_def_stmt_syntax;
    int i,j;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    result->is_constant = true;
    result->name = ident->getSymbol()->getText();
    if(ctx->LeftBracket()) //array
    {
        auto expression = ctx->exp();
        int comma_num;
	auto commas = ctx->Comma();
        if(commas.empty()) comma_num = 0;
        else comma_num = (ctx->Comma()).size();
        if(expression.size()==comma_num+2) //ident[exp]={exp,exp,...}
        {
            result->array_length.reset(visitExp(expression[0]).as<expr_syntax *>());
            j = 1;
        }
        else{
            //ident[]={exp,exp,...}
            //size of array = comma_num + 1
            //array_length: type = literal_syntax
            //              pos = pos of ']'
            auto rbracket = ctx->RightBracket();
            auto num = new literal_syntax;
            num->line = rbracket->getSymbol()->getLine();
            num->pos = rbracket->getSymbol()->getCharPositionInLine();
            num->number = comma_num + 1;
            result->array_length.reset(static_cast<expr_syntax *>(num));
            j = 0;
        }
        for(i=0;i<comma_num+1;i++)
            result->initializers.emplace_back(visitExp(expression[i+j]).as<expr_syntax *>());
    }
    else
    {
        auto expression = ctx->exp();
        result->initializers.emplace_back(visitExp(expression[0]).as<expr_syntax *>());
    }
    return result;
}

antlrcpp::Any syntax_tree_builder::visitVardecl(C1Parser::VardeclContext *ctx)
{
    //vardecl: Int vardef (Comma vardef)* SemiColon ;
    auto var = ctx->vardef();
    int var_num = var.size();
    int i;
    auto variables = new ptr_list<var_def_stmt_syntax>;
    for(i=0;i<var_num;i++)
        variables->emplace_back(visitVardef(var[i]).as<var_def_stmt_syntax *>());
    return variables;////
}

antlrcpp::Any syntax_tree_builder::visitVardef(C1Parser::VardefContext *ctx)
{
/*  vardef: 
       Identifier
	   | Identifier LeftBracket exp RightBracket
	   | Identifier Assign exp
	   | Identifier LeftBracket (exp)? RightBracket Assign LeftBrace exp (Comma exp)* RightBrace*/
    auto ident = ctx->Identifier();
    auto result = new var_def_stmt_syntax;
    int i,j;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    result->is_constant = false;
    result->name = ident->getSymbol()->getText();
    if(ctx->LeftBracket())
    {
        auto expression = ctx->exp();
        if(ctx->Assign())
        {
            int comma_num;   
	    auto commas = ctx->Comma();
            if(commas.empty()) comma_num = 0;
            else comma_num = (ctx->Comma()).size();
            if(expression.size()==comma_num+2) //ident[exp]={exp,exp,...}
            {
                result->array_length.reset(visitExp(expression[0]).as<expr_syntax *>());
                j=1;
            }
            else{
                //ident[]={exp,exp,...}
                //size of array = comma_num + 1
                //array_length: type = literal_syntax
                //              pos = pos of ']'
                auto rbracket = ctx->RightBracket();
                auto num = new literal_syntax;
                num->line = rbracket->getSymbol()->getLine();
                num->pos = rbracket->getSymbol()->getCharPositionInLine();
                num->number = comma_num + 1;
                result->array_length.reset(static_cast<expr_syntax *>(num));
                j=0;
            }
            for(i=0;i<comma_num+1;i++)
            result->initializers.emplace_back(visitExp(expression[i+j]).as<expr_syntax *>());
        }
        else
            result->array_length.reset(visitExp(expression[0]).as<expr_syntax *>());
    }
    else
    {
        if(ctx->Assign())
        {
            auto expression = ctx->exp();
            result->initializers.emplace_back(visitExp(expression[0]).as<expr_syntax *>());
        }
    }
    return result;
}

antlrcpp::Any syntax_tree_builder::visitFuncdef(C1Parser::FuncdefContext *ctx)
{
    //funcdef: Void Identifier LeftParen RightParen block;
    auto ident = ctx->Identifier();
    auto func_body = ctx->block();
    auto result = new func_def_syntax;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    result->name = ident->getSymbol()->getText();
    result->body.reset(visitBlock(func_body).as<block_syntax *>());
    return result;
}

antlrcpp::Any syntax_tree_builder::visitBlock(C1Parser::BlockContext *ctx)
{
    //block: LeftBrace (decl | stmt)* RightBrace ;
    auto result = new block_syntax;
    int i,num;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    /*auto declaration =ctx->decl();
    num = declaration.size();   
        for(i=0;i<num;i++)
        {
            auto var_list = visitDecl(declaration[i]).as<ptr_list<var_def_stmt_syntax> *>();
            int lst_size = var_list->size();
            int j;
            for(j=0;j<lst_size;j++){
	 	ptr<stmt_syntax> temp = (*var_list)[j];
                result->body.emplace_back(temp);
	    }
        }
            
    
    auto statement = ctx->stmt() ;
    num = statement.size();
    for(i=0;i<num;i++)
        result->body.emplace_back(visitStmt(statement[i]).as<stmt_syntax *>());*/
    auto blockitems = ctx->blockitem();
    num = blockitems.size();
    for(i=0;i<num;i++){
	if(blockitems[i]->decl())
	{
	    auto var_list = visitDecl(blockitems[i]->decl()).as<ptr_list<var_def_stmt_syntax> *>();
            int lst_size = var_list->size();
            int j;
            for(j=0;j<lst_size;j++){
	 	ptr<stmt_syntax> temp = (*var_list)[j];
                result->body.emplace_back(temp);
	    }
	}
	else{
	    result->body.emplace_back(visitStmt(blockitems[i]->stmt()).as<stmt_syntax *>());
	}
    }
    return result;
}

antlrcpp::Any syntax_tree_builder::visitStmt(C1Parser::StmtContext *ctx)
{
/*   stmt:
     lval Assign exp SemiColon
	 | Identifier LeftParen RightParen SemiColon
	 | block
	 | If LeftParen cond RightParen stmt ( Else stmt )?
	 | While LeftParen cond RightParen stmt
	 | SemiColon*/
     if(ctx->Assign())
     {
         auto result = new assign_stmt_syntax;
         auto lvalue = ctx->lval();
         auto rvalue = ctx->exp();
         //ptr<lval_syntax> target;
         //ptr<expr_syntax> value;
         result->line = ctx->getStart()->getLine();
         result->pos = ctx->getStart()->getCharPositionInLine();
         result->target.reset(visitLval(lvalue).as<lval_syntax *>());
         result->value.reset(visitExp(rvalue).as<expr_syntax *>());
         return static_cast<stmt_syntax *>(result);
     }
     if(ctx->Identifier())
     {
         auto result = new func_call_stmt_syntax;
         //std::string name;
         auto ident = ctx->Identifier();
         result->line = ctx->getStart()->getLine();
         result->pos = ctx->getStart()->getCharPositionInLine();
         result->name = ident->getSymbol()->getText();
         return static_cast<stmt_syntax *>(result);
     }
     if(auto blk = ctx->block())
     {
         return static_cast<stmt_syntax *>(visitBlock(blk).as<block_syntax *>());
     }
     if(ctx->If())
	 {
         auto result = new if_stmt_syntax;
         //ptr<cond_syntax> pred;
         //ptr<stmt_syntax> then_body;
         //ptr<stmt_syntax> else_body;
         auto condition = ctx->cond();
         auto statement = ctx->stmt();
         result->line = ctx->getStart()->getLine();
         result->pos = ctx->getStart()->getCharPositionInLine();
         result->pred.reset(visitCond(condition).as<cond_syntax *>());
         result->then_body.reset(visitStmt(statement[0]).as<stmt_syntax *>());
         if(ctx->Else())
            result->else_body.reset(visitStmt(statement[1]).as<stmt_syntax *>());
         return static_cast<stmt_syntax *>(result);
     }
     if(ctx->While())
     {
         //ptr<cond_syntax> pred;
         //ptr<stmt_syntax> body;
         auto result = new while_stmt_syntax;
         auto condition = ctx->cond();
         auto statement = ctx->stmt();
         result->line = ctx->getStart()->getLine();
         result->pos = ctx->getStart()->getCharPositionInLine();
         result->pred.reset(visitCond(condition).as<cond_syntax *>());
         result->body.reset(visitStmt(statement[0]).as<stmt_syntax *>());
         return static_cast<stmt_syntax *>(result);
     }
     // empty statement
     auto result = new empty_stmt_syntax;
     result->line = ctx->getStart()->getLine();
     result->pos = ctx->getStart()->getCharPositionInLine();
     return static_cast<stmt_syntax *>(result);


}

antlrcpp::Any syntax_tree_builder::visitLval(C1Parser::LvalContext *ctx)
{
    auto ident = ctx->Identifier();
    auto result = new lval_syntax;
    if(ctx->LeftBracket())
    {
        // lval -> Identifier LeftBracket exp RightBracket
        auto expression = ctx->exp();
        auto result = new lval_syntax;
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        result->name = ident->getSymbol()->getText();
        result->array_index.reset(visitExp(expression).as<expr_syntax *>());
        return result;
    }
    // Otherwise: lval -> Identifier
    result->line = ident->getSymbol()->getLine();
    result->pos = ident->getSymbol()->getCharPositionInLine();
    result -> name = ident->getSymbol()->getText();
    return result;
}

antlrcpp::Any syntax_tree_builder::visitCond(C1Parser::CondContext *ctx)
{
    //cond: exp (Equal | NonEqual | Less | Greater | LessEqual | GreaterEqual) exp ;
    auto expression = ctx->exp();
    auto result = new cond_syntax;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    result->lhs.reset(visitExp(expression[0]).as<expr_syntax *>());
    if(ctx->Equal())
        result->op = relop::equal;
    else if(ctx->NonEqual())
        result->op = relop::non_equal;
    else if(ctx->Less())
        result->op = relop::less;
    else if(ctx->Greater())
        result->op = relop::greater;
    else if(ctx->LessEqual())
        result->op = relop::less_equal;
    else if(ctx->GreaterEqual())
        result->op = relop::greater_equal;
    result->rhs.reset(visitExp(expression[1]).as<expr_syntax *>());
    return result;
}

// Returns antlrcpp::Any, which is constructable from any type.
// However, you should be sure you use the same type for packing and depacking the `Any` object.
// Or a std::bad_cast exception will rise.
// This function always returns an `Any` object containing a `expr_syntax *`.
antlrcpp::Any syntax_tree_builder::visitExp(C1Parser::ExpContext *ctx)
{
    // Get all sub-contexts of type `exp`.
    auto expressions = ctx->exp();
    // Two sub-expressions presented: this indicates it's a expression of binary operator, aka `binop`.
    if (expressions.size() == 2)
    {
        auto result = new binop_expr_syntax;
        // Set line and pos.
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        // visit(some context) is equivalent to calling corresponding visit method; dispatching is done automatically
        // by ANTLR4 runtime. For this case, it's equivalent to visitExp(expressions[0]).
        // Use reset to set a new pointer to a std::shared_ptr object. DO NOT use assignment; it won't work.
        // Use `.as<Type>()' to get value from antlrcpp::Any object; notice that this Type must match the type used in
        // constructing the Any object, which is constructed from (usually pointer to some derived class of
        // syntax_node, in this case) returning value of the visit call.
        result->lhs.reset(visit(expressions[0]).as<expr_syntax *>());
        // Check if each token exists.
        // Returnd value of the calling will be nullptr (aka NULL in C) if it isn't there; otherwise non-null pointer.
        if (ctx->Plus())
            result->op = binop::plus;
        if (ctx->Minus())
            result->op = binop::minus;
        if (ctx->Multiply())
            result->op = binop::multiply;
        if (ctx->Divide())
            result->op = binop::divide;
        if (ctx->Modulo())
            result->op = binop::modulo;
        result->rhs.reset(visit(expressions[1]).as<expr_syntax *>());
        return static_cast<expr_syntax *>(result);
    }
    // Otherwise, if `+` or `-` presented, it'll be a `unaryop_expr_syntax`.
    if (ctx->Plus() || ctx->Minus())
    {
        auto result = new unaryop_expr_syntax;
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        if (ctx->Plus())
            result->op = unaryop::plus;
        if (ctx->Minus())
            result->op = unaryop::minus;
        result->rhs.reset(visit(expressions[0]).as<expr_syntax *>());
        return static_cast<expr_syntax *>(result);
    }
    // In the case that `(` exists as a child, this is an expression like `'(' expressions[0] ')'`.
    if (ctx->LeftParen())
        return visit(expressions[0]); // Any already holds expr_syntax* here, no need for dispatch and re-patch with casting.
    // If `Number` exists as a child, we can say it's a literal integer expression.
    if (auto number = ctx->Number())
    {
        auto result = new literal_syntax;
        result->line = number->getSymbol()->getLine();
        result->pos = number->getSymbol()->getCharPositionInLine();
        auto text = number->getSymbol()->getText();
        if (text[0] == '0' && text[1] == 'x')              // Hexadecimal
            result->number = std::stoi(text, nullptr, 16); // std::stoi will eat '0x'
        else                                               // Decimal
            result->number = std::stoi(text, nullptr, 10);
        return static_cast<expr_syntax *>(result);
    }
    // Otherwise: expr -> lval
    auto lvalue = ctx->lval();
    return static_cast<expr_syntax *>(visitLval(lvalue).as<lval_syntax *>());
}

ptr<syntax_tree_node> syntax_tree_builder::operator()(antlr4::tree::ParseTree *ctx)
{
    auto result = visit(ctx);
    if (result.is<syntax_tree_node *>())
        return ptr<syntax_tree_node>(result.as<syntax_tree_node *>());
    if (result.is<assembly *>())
        return ptr<syntax_tree_node>(result.as<assembly *>());
    if (result.is<global_def_syntax *>())
        return ptr<syntax_tree_node>(result.as<global_def_syntax *>());
    if (result.is<func_def_syntax *>())
        return ptr<syntax_tree_node>(result.as<func_def_syntax *>());
    if (result.is<cond_syntax *>())
        return ptr<syntax_tree_node>(result.as<cond_syntax *>());
    if (result.is<expr_syntax *>())
        return ptr<syntax_tree_node>(result.as<expr_syntax *>());
    if (result.is<binop_expr_syntax *>())
        return ptr<syntax_tree_node>(result.as<binop_expr_syntax *>());
    if (result.is<unaryop_expr_syntax *>())
        return ptr<syntax_tree_node>(result.as<unaryop_expr_syntax *>());
    if (result.is<lval_syntax *>())
        return ptr<syntax_tree_node>(result.as<lval_syntax *>());
    if (result.is<literal_syntax *>())
        return ptr<syntax_tree_node>(result.as<literal_syntax *>());
    if (result.is<stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<stmt_syntax *>());
    if (result.is<var_def_stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<var_def_stmt_syntax *>());
    if (result.is<assign_stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<assign_stmt_syntax *>());
    if (result.is<func_call_stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<func_call_stmt_syntax *>());
    if (result.is<block_syntax *>())
        return ptr<syntax_tree_node>(result.as<block_syntax *>());
    if (result.is<if_stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<if_stmt_syntax *>());
    if (result.is<while_stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<while_stmt_syntax *>());
    return nullptr;
}
