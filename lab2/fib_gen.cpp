    #include <llvm/IR/BasicBlock.h>

    #include <llvm/IR/Constants.h>

    #include <llvm/IR/DerivedTypes.h>

    #include <llvm/IR/Function.h>

    #include <llvm/IR/IRBuilder.h>

    #include <llvm/IR/LLVMContext.h>

    #include <llvm/IR/Module.h>

    #include <llvm/IR/Type.h>

    #include <llvm/IR/Verifier.h>


    #include <memory>


    using namespace llvm;


    int main()

    {

        LLVMContext context;

        IRBuilder<> builder(context);


        auto module = new Module("fib", context);
	std::vector<Type *> arr_list;
	arr_list.push_back(Type::getInt32Ty(context));

        auto func_fib = Function::Create(FunctionType::get(Type::getInt32Ty(context), arr_list, false),

                                     GlobalValue::LinkageTypes::ExternalLinkage,

                                     "fib", module);

        auto func_main = Function::Create(FunctionType::get(Type::getInt32Ty(context), std::vector<Type *>(), false),

                                     GlobalValue::LinkageTypes::ExternalLinkage,

                                     "main", module);

        auto fib_entry = BasicBlock::Create(context, "entry", func_fib);

        auto IfZero = BasicBlock::Create(context, "IfZero", func_fib);

        auto IfNotZero = BasicBlock::Create(context, "IfNotZero", func_fib);

        auto IfOne = BasicBlock::Create(context, "IfOne", func_fib);

        auto Otherwise = BasicBlock::Create(context, "Otherwise", func_fib);

        auto main_entry = BasicBlock::Create(context, "entry", func_main);

        auto cmp = BasicBlock::Create(context, "cmp", func_main);

        auto loop = BasicBlock::Create(context, "loop", func_main);

        auto exit_bb = BasicBlock::Create(context, "exit", func_main);


        //constands definition

        auto Zero = ConstantInt::get(Type::getInt32Ty(context),0,true);

        auto One = ConstantInt::get(Type::getInt32Ty(context),1,true);

        auto Two = ConstantInt::get(Type::getInt32Ty(context),2,true);

        auto Ten = ConstantInt::get(Type::getInt32Ty(context),10,true);


        //fib function

        //entry:

        builder.SetInsertPoint(fib_entry);

        auto n = &(*func_fib->arg_begin());

        //static Constant *get(Type *Ty, uint64_t V, bool isSigned = false);

        auto cond1 = builder.CreateICmpEQ(n, Zero);

        //Value *CreateICmpEQ(Value *LHS, Value *RHS, const Twine &Name = "")

        builder.CreateCondBr(cond1, IfZero, IfNotZero);

        /*BranchInst *CreateCondBr(Value *Cond, BasicBlock *True, BasicBlock *False,

                               MDNode *BranchWeights = nullptr,

                               MDNode *Unpredictable = nullptr)*/

        builder.ClearInsertionPoint();

        

        //IfZero:

        builder.SetInsertPoint(IfZero);

        builder.CreateRet(Zero);

        //ReturnInst *CreateRet(Value *V)

        builder.ClearInsertionPoint();


        //IfNotZero:

        builder.SetInsertPoint(IfNotZero);

        auto cond2 = builder.CreateICmpEQ(n, One);

        builder.CreateCondBr(cond2, IfOne, Otherwise);

        builder.ClearInsertionPoint();


        //IfOne

        builder.SetInsertPoint(IfOne);

        builder.CreateRet(One);

        builder.ClearInsertionPoint();


        //Otherwise

        builder.SetInsertPoint(Otherwise);

        /*Value *CreateAdd(Value *LHS, Value *RHS, const Twine &Name = "",

                       bool HasNUW = false, bool HasNSW = false)

          Value *CreateSub(Value *LHS, Value *RHS, const Twine &Name = "",

                       bool HasNUW = false, bool HasNSW = false)*/

        auto n1 = builder.CreateSub(n, One);

        auto n2 = builder.CreateSub(n, Two);
	std::vector<Value *> arg1, arg2;
	arg1.push_back(n1);
	arg2.push_back(n2);

        auto temp1 = builder.CreateCall(func_fib, arg1);

        auto temp2 = builder.CreateCall(func_fib, arg2);

        auto result = builder.CreateAdd(temp1, temp2);

        builder.CreateRet(result);

        builder.ClearInsertionPoint();

       


        //main function

        //entry:

        builder.SetInsertPoint(main_entry);

        auto ptrx = builder.CreateAlloca(Type::getInt32Ty(context));

        builder.CreateStore(Zero, ptrx);

        auto ptri = builder.CreateAlloca(Type::getInt32Ty(context));

        builder.CreateStore(Zero, ptri);

        //AllocaInst *CreateAlloca(Type *Ty, Value *ArraySize = nullptr,

        //                         const Twine &Name = "")

        //StoreInst *CreateStore(Value *Val, Value *Ptr, bool isVolatile = false)

        builder.CreateBr(cmp);

        //BranchInst *CreateBr(BasicBlock *Dest)

        builder.ClearInsertionPoint();


        //cmp:

        builder.SetInsertPoint(cmp);

        auto i1 = builder.CreateLoad(Type::getInt32Ty(context),ptri);

        auto cond = builder.CreateICmpEQ(i1, Ten);

        //LoadInst *CreateLoad(Type *Ty, Value *Ptr, const Twine &Name = "")

        builder.CreateCondBr(cond, loop, exit_bb);

        builder.ClearInsertionPoint();


        //loop:

        builder.SetInsertPoint(loop);

        auto x1 = builder.CreateLoad(Type::getInt32Ty(context),ptrx);
	std::vector<Value *> arg;
	arg.push_back(i1);

        auto temp = builder.CreateCall(func_fib,{i1});

        auto x2 = builder.CreateAdd(x1, temp);

        builder.CreateStore(x2, ptrx);

        auto i2 = builder.CreateAdd(i1, One);

        builder.CreateStore(i2, ptri);

        builder.CreateBr(cmp);

        builder.ClearInsertionPoint();


        //exit:

        builder.SetInsertPoint(exit_bb);

        auto x3 = builder.CreateLoad(Type::getInt32Ty(context),ptrx);

        builder.CreateRet(x3);

        builder.ClearInsertionPoint();


        module->print(outs(), nullptr);

        delete module;

        return 0;

    }
