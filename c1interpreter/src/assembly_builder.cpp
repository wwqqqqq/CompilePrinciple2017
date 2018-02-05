
#include "assembly_builder.h"
#include <cstdlib>
#include <cstring>

#include <vector>

using namespace llvm;
using namespace c1_recognizer::syntax_tree;



void assembly_builder::visit(assembly &node)
{
/*struct assembly : syntax_tree_node
{
    ptr_list<global_def_syntax> global_defs;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};*/
    enter_scope();
    auto def_list = node.global_defs;
    ptr_list<global_def_syntax>::iterator it;
    in_global=true;
    for(it=def_list.begin();it!=def_list.end();it++)
        (*it)->accept(*this); 
    exit_scope();
}

void assembly_builder::visit(func_def_syntax &node)
{
/*struct func_def_syntax : global_def_syntax
{
    std::string name;
    ptr<block_syntax> body;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};*/
//llvm::Function *current_function;
    if(functions[node.name]!=nullptr)
    {
        error_flag = true;
        err.error(node.line, node.pos, "Reduplicate function declaration");
        return;
    }
    auto func_body = node.body;
    in_global=false;
    current_function = Function::Create(FunctionType::get(Type::getVoidTy(context), std::vector<Type *>(), false),
                                 GlobalValue::LinkageTypes::ExternalLinkage,
                                 node.name, module.get());
    functions[node.name] = current_function;
    bb_count = 0;
    auto bb = BasicBlock::Create(context, "entry", current_function);
    builder.SetInsertPoint(bb);
    visit(*func_body);
    builder.CreateRetVoid();
    builder.ClearInsertionPoint();
    in_global=true;
}

void assembly_builder::visit(cond_syntax &node)
{
/*struct cond_syntax : syntax_tree_node
{
    relop op;
    ptr<expr_syntax> lhs, rhs;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};*/
    lval_as_rval = true;
    node.lhs->accept(*this);
    auto LHS = value_result;
    lval_as_rval = true;
    (node.rhs)->accept(*this);
    auto RHS = value_result;
    switch(node.op){
        case relop::equal:     value_result = builder.CreateICmpEQ(LHS, RHS); break; 
        case relop::non_equal: value_result = builder.CreateICmpNE(LHS, RHS); break;
        case relop::less:      value_result = builder.CreateICmpSLT(LHS, RHS); break;
        case relop::less_equal:value_result = builder.CreateICmpSLE(LHS, RHS); break;
        case relop::greater:   value_result = builder.CreateICmpSGT(LHS, RHS); break;
        case relop::greater_equal:
                               value_result = builder.CreateICmpSGE(LHS, RHS); break;
        default:               error_flag = true; err.error(node.line, node.pos, "Invalid relop"); break;
    }
}

void assembly_builder::visit(binop_expr_syntax &node)
{
/*struct binop_expr_syntax : expr_syntax
{
    binop op;
    ptr<expr_syntax> lhs, rhs;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};*/
    lval_as_rval = true;
    (node.lhs)->accept(*this);
    auto LHS = value_result;
    int lnum = const_result;
    lval_as_rval = true;
    (node.rhs)->accept(*this);
    auto RHS = value_result;
    int rnum = const_result;
    if(constexpr_expected)
    {
        switch(node.op){
        case binop::plus:      const_result = lnum+rnum; break;
        case binop::minus:     const_result = lnum-rnum; break;
        case binop::multiply:  const_result = lnum*rnum; break;
        case binop::divide:    const_result = lnum/rnum; break;
        case binop::modulo:    const_result = lnum%rnum; break;
        default:        error_flag = true; err.error(node.line, node.pos, "Invalid binop"); break;
        }
    }
    else
    {
        switch(node.op){
        case binop::plus:      value_result = builder.CreateAdd(LHS, RHS); const_result = lnum+rnum; break;
        case binop::minus:     value_result = builder.CreateSub(LHS, RHS); const_result = lnum-rnum; break;
        case binop::multiply:  value_result = builder.CreateMul(LHS, RHS); const_result = lnum*rnum; break;
        case binop::divide:    value_result = builder.CreateSDiv(LHS, RHS); const_result = lnum/rnum; break;
        case binop::modulo:    value_result = builder.CreateBinOp(Instruction::SRem, LHS, RHS); 
                                const_result = lnum%rnum; break;
        default:        error_flag = true; err.error(node.line, node.pos, "Invalid binop"); break;
        }
    }
    
}

void assembly_builder::visit(unaryop_expr_syntax &node)
{
/*struct unaryop_expr_syntax : expr_syntax
{
    unaryop op;
    ptr<expr_syntax> rhs;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};*/
    lval_as_rval = true;
    (node.rhs)->accept(*this);
    auto RHS = value_result;
    auto Zero = ConstantInt::get(Type::getInt32Ty(context),0,true);
    if(constexpr_expected)
    {
        switch(node.op){
        case unaryop::plus:  break;
        case unaryop::minus: const_result = -const_result; break;
        default:    error_flag = true; err.error(node.line, node.pos, "Invalid unaryop"); break;
        }
    }
    else
    {
        switch(node.op){
        case unaryop::plus:  value_result = RHS; break;
        case unaryop::minus: value_result = builder.CreateSub(Zero, RHS); const_result = -const_result; break;
        default:    error_flag = true; err.error(node.line, node.pos, "Invalid unaryop"); break;
        }
    }
    
}

void assembly_builder::visit(lval_syntax &node)
{
/*struct lval_syntax : expr_syntax
{
    std::string name;
    ptr<expr_syntax> array_index; // nullptr if not indexed as array
    virtual void accept(syntax_tree_visitor &visitor) override final;
};*/
    auto ident = lookup_variable(node.name);
    auto PTR = std::get<0>(ident);
    if(constexpr_expected)
    {
        error_flag = true;
        err.error(node.line, node.pos, "Expecting a constant");
	return;
    }
    if(PTR == nullptr)
    {
        error_flag = true;
        err.error(node.line, node.pos, "Use of undeclared identifier");
	return;
    }
    if(std::get<2>(ident) && node.array_index !=nullptr)
    {
        auto Zero = ConstantInt::get(Type::getInt32Ty(context),0,true);
	bool temp = lval_as_rval;
	lval_as_rval = true;
        (node.array_index)->accept(*this);
	lval_as_rval = temp;
        auto index = value_result;
        std::vector<Value *> ind_list;
        ind_list.push_back(Zero);
        ind_list.push_back(index);
        PTR = builder.CreateGEP(PTR, ind_list);
    }
    else if(node.array_index!=nullptr)
    {
        error_flag = true;
        err.error(node.line, node.pos, "The identifier cannot be indexed");
        return;
    }
    if(std::get<2>(ident) && node.array_index==nullptr)//lval is the array ptr
    {
        error_flag = true;
        err.error(node.line, node.pos, "Array without being indexed cannot be used without indexed");
	return;
    }
    if(lval_as_rval)
    {
        //if(node.array_index != nullptr)
        value_result = builder.CreateLoad(PTR);
        //lval_as_rval = false;
    }
    else if(std::get<1>(ident)){
       error_flag = true;
        err.error(node.line, node.pos, "Const identifier cannot be used as left value");
    }
    else value_result = PTR; //No error occurs and it's a left value.
}

void assembly_builder::visit(literal_syntax &node)
{
/*struct literal_syntax : expr_syntax
{
    int number;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};*/
    const_result = node.number;
    value_result = ConstantInt::get(Type::getInt32Ty(context),node.number,true);
}

void assembly_builder::visit(var_def_stmt_syntax &node)
{
/*struct var_def_stmt_syntax : stmt_syntax, global_def_syntax
{
    bool is_constant;
    std::string name;
    ptr<expr_syntax> array_length; // nullptr for non-array variables
    ptr_list<expr_syntax> initializers;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};*/
/* GlobalVariable(Type *Ty, bool isConstant, LinkageTypes Linkage,
                 Constant *Initializer = nullptr, const Twine &Name = "",
                 ThreadLocalMode = NotThreadLocal, unsigned AddressSpace = 0,
                 bool isExternallyInitialized = false);
  /// GlobalVariable ctor - This creates a global and inserts it before the
  /// specified other global.
  GlobalVariable(Module &M, Type *Ty, bool isConstant,
                 LinkageTypes Linkage, Constant *Initializer,
                 const Twine &Name = "", GlobalVariable *InsertBefore = nullptr,
                 ThreadLocalMode = NotThreadLocal, unsigned AddressSpace = 0,
                 bool isExternallyInitialized = false);*/
    bool is_array;
    int length=1;
    int i;
    if(node.array_length == nullptr) is_array = false;
    else is_array = true;
    Value *PTR;
    if(in_global)
    {
        if(!is_array){
            if(node.initializers.empty()){
                auto Zero = ConstantInt::get(Type::getInt32Ty(context),0,true);
                value_result = new GlobalVariable(*(module.get()),Type::getInt32Ty(context),node.is_constant,
                                          GlobalValue::LinkageTypes::ExternalLinkage,Zero);
            }
            else{
                auto num_expr = (node.initializers)[0];
                constexpr_expected = true;
                num_expr->accept(*this);
                constexpr_expected = false;
                auto init = ConstantInt::get(Type::getInt32Ty(context), const_result);
                value_result = new GlobalVariable(*(module.get()),Type::getInt32Ty(context),node.is_constant,
                                              GlobalValue::LinkageTypes::ExternalLinkage, init);
            }
        }
        else
        {
            constexpr_expected = true;
            (node.array_length)->accept(*this);
            constexpr_expected = false;
            length = const_result;
	    if(length<0)
	    {
        	error_flag = true;
        	err.error(node.line, node.pos, "Array length cannot be negative");
		return;
            }
            //static ArrayType *get(Type *ElementType, uint64_t NumElements);
            auto ty = ArrayType::get(Type::getInt32Ty(context),length);
            
            int i;
            std::vector<Constant *> vec;
            ptr_list<expr_syntax>::iterator it;
            auto initials = node.initializers;
            for(i=0,it=initials.begin();it!=initials.end()&&i<length;it++,i++)
            {
                constexpr_expected = true;
                (*it)->accept(*this);
                constexpr_expected = false;
                auto constant = ConstantInt::get(Type::getInt32Ty(context),const_result,true);
                vec.push_back(constant);
            }
	    if(it!=initials.end())
	    {
        	error_flag = true;
        	err.error(node.line, node.pos, "Too much initializer expressions");
            }
            for(;i<length;i++)
            {
                auto Zero = ConstantInt::get(Type::getInt32Ty(context),0,true);
                vec.push_back(Zero);
            }
            //std::initializer_list<T> &vec
            auto Initializer = ConstantArray::get(ty,vec);
            value_result = new GlobalVariable(*(module.get()),ty,node.is_constant,
                                              GlobalValue::LinkageTypes::ExternalLinkage, Initializer);
        }
        if(declare_variable(node.name, value_result, node.is_constant, is_array)==false)
        {
            error_flag = true;
            err.error(node.line, node.pos, "Global identifier declaration fails");
        }
        return;
    }


    if(is_array){
        constexpr_expected = true;
        (node.array_length)->accept(*this);
        constexpr_expected = false;
        length = const_result;
	    if(length<0)
	    {
        	error_flag = true;
        	err.error(node.line, node.pos, "Array length cannot be negative");
		return;
            }
        auto ty = ArrayType::get(Type::getInt32Ty(context),length);

        PTR = builder.CreateAlloca(ty);
        //PTR = builder.CreateAlloca(Type::getInt32Ty(context),value_result);
    //AllocaInst *CreateAlloca(Type *Ty, Value *ArraySize = nullptr,
    //                       const Twine &Name = "")
    }
    else PTR = builder.CreateAlloca(Type::getInt32Ty(context));
    if(declare_variable(node.name,PTR,node.is_constant,is_array)==false)
    {
        error_flag = true;
        err.error(node.line, node.pos, "Identifier declaration fails");
        return;
    }
    if(!node.initializers.empty()){
        ptr_list<expr_syntax>::iterator it;
        auto initials = node.initializers;
        if(is_array){
            for(i=0,it=initials.begin();it!=initials.end()&&i<length;it++,i++){
                std::vector<Value *> ind_list;
                auto Zero = ConstantInt::get(Type::getInt32Ty(context),0,true);
                auto index = ConstantInt::get(Type::getInt32Ty(context),i,true);
		lval_as_rval = true;
		(*it)->accept(*this);
		lval_as_rval = false;
		auto val = value_result;
                ind_list.push_back(Zero);
                ind_list.push_back(index);
                auto ptr = builder.CreateGEP(PTR, ind_list);
                builder.CreateStore(val,ptr);
            }
	    if(it!=initials.end())
	    {
        	error_flag = true;
        	err.error(node.line, node.pos, "Too much initializer expressions");
    	    }
        }
        else
        {
            auto expr = initials[0];
	    lval_as_rval = true;
            expr->accept(*this);
	    lval_as_rval =false;
            builder.CreateStore(value_result,PTR);
        }
    }
    value_result = PTR;
}

void assembly_builder::visit(assign_stmt_syntax &node)
{
/*struct assign_stmt_syntax : stmt_syntax
{
    ptr<lval_syntax> target;
    ptr<expr_syntax> value;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};*/
    lval_as_rval = false;
    visit(*(node.target));
    auto PTR = value_result;
    lval_as_rval = true;
    (node.value)->accept(*this);
    lval_as_rval = false;
    auto val = value_result;
    builder.CreateStore(val,PTR);
}

void assembly_builder::visit(func_call_stmt_syntax &node)
{
/*struct func_call_stmt_syntax : stmt_syntax
{
    std::string name;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};*/
    auto func = functions[node.name];
    if(func==nullptr)
    {
        error_flag = true;
        err.error(node.line, node.pos, "Invalid function call");
        return;
    }
    builder.CreateCall(func, {});
}

void assembly_builder::visit(block_syntax &node)
{
/*struct block_syntax : stmt_syntax
{
    ptr_list<stmt_syntax> body;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};*/
    auto stmt_list = node.body;
    ptr_list<stmt_syntax>::iterator it;
    enter_scope();
    for(it=stmt_list.begin();it!=stmt_list.end();it++)
        (*it)->accept(*this);
    exit_scope();
}

void assembly_builder::visit(if_stmt_syntax &node)
{
/*struct if_stmt_syntax : stmt_syntax
{
    ptr<cond_syntax> pred;
    ptr<stmt_syntax> then_body;
    ptr<stmt_syntax> else_body;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};*/
    visit(*(node.pred));
    auto cond = value_result;
    auto bb_true = BasicBlock::Create(context, "BB" + std::to_string(bb_count++), current_function);
    BasicBlock *bb_false;
    if(node.else_body!=nullptr)
        bb_false = BasicBlock::Create(context, "BB" + std::to_string(bb_count++), current_function);
    auto bb_next = BasicBlock::Create(context, "BB" + std::to_string(bb_count++), current_function);

    if(node.else_body!=nullptr)
    {
        builder.CreateCondBr(cond,bb_true,bb_false);
    }
    else builder.CreateCondBr(cond,bb_true,bb_next);
    builder.ClearInsertionPoint();

    builder.SetInsertPoint(bb_true);
    (node.then_body)->accept(*this);
    builder.CreateBr(bb_next);
    builder.ClearInsertionPoint();

    if(node.else_body!=nullptr)
    {
        builder.SetInsertPoint(bb_false);
        (node.else_body)->accept(*this);
        builder.CreateBr(bb_next);
        builder.ClearInsertionPoint();
    }

    builder.SetInsertPoint(bb_next);
}

void assembly_builder::visit(while_stmt_syntax &node)
{
/*struct while_stmt_syntax : stmt_syntax
{
    ptr<cond_syntax> pred;
    ptr<stmt_syntax> body;
    virtual void accept(syntax_tree_visitor &visitor) override final;
};*/
    auto bb_begin = BasicBlock::Create(context, "BB" + std::to_string(bb_count++), current_function);
    auto bb_loop = BasicBlock::Create(context, "BB" + std::to_string(bb_count++), current_function);
    auto bb_next = BasicBlock::Create(context, "BB" + std::to_string(bb_count++), current_function);
    builder.CreateBr(bb_begin);
    builder.ClearInsertionPoint();

    builder.SetInsertPoint(bb_begin);
    visit(*(node.pred));
    auto cond = value_result;
    builder.CreateCondBr(cond,bb_loop,bb_next);
    builder.ClearInsertionPoint();

    builder.SetInsertPoint(bb_loop);
    (node.body)->accept(*this);
    builder.CreateBr(bb_begin);
    builder.ClearInsertionPoint();

    builder.SetInsertPoint(bb_next);
    
}

void assembly_builder::visit(empty_stmt_syntax &node)
{
}


