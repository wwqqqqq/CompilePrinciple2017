# lab2  预热实验

## 遇到的问题及解决方法   
1. `lli fib.ll`指令报错：  
`error: instruction expected to be numbered "%0"`    
原因：fib函数中使用的临时变量编号从%1开始，而没有使用%0。  
解决方法： 将%1改为%0，%2改为%1，以此类推；或将fib参数中的%n删去，仅使用`define int32 @fib(int32)`，再将fib函数子基本块中所有%n换成%0即可。提交的版本中使用第一种方法。  
1. 编译`llvm-irgen-example.cpp`时遇到的问题：  
    1. `fatal error: llvm/IR/BasicBlock.h: 没有那个文件或目录`， 或提示`llvm-config`参数无法使用  
解决方法：将编译后的`llvm_install/bin`加入path中。  
    2. 修改参数，使用``c++ llvm-irgen-example.cpp `llvm-config --cxxflags --ldflags --system-libs --libs` -o llvm-irgen-example``
1. 编译fib_gen.cpp中出现的问题：  
    1. fib函数如何得到函数参数？  
   使用`auto n = func_fib->args();`，造成一系列类型匹配错误。  
   解决方法： 使用`auto n = &(*func_fib->arg_begin());`
    2. `BasicBlock::CreateCall`函数中`ArrayRef<Value *>`类型的参数，即传给函数的一串参数，若直接使用`{n}`（n为Value *类型的指针），会造成类型错误。
   解决方法：  
   `std::vector<Value *> arg_list;`  
   `arg_list.push_back(n);`  
   使用`arg_list`替换`{n}`即可。
    3. 将常数（如0，1，2，10）转换为Value *类型的指针  
   使用`Constant::get(Type::getInt32Ty(context),0,true)`会提示`llvm::Constant`中找不到get函数，将`Constant`改为`ConstantInt`即可解决。

## 分析与设计
1. 实验1：编写`fib.ll`
所使用的LLVM IR语法：
    * Conditional brach: `br i1 <cond>, label <iftrue>, label <iffalse>`
    * Unconditional branch: `br label <dest>`
    * Compare: `<result> = icmp <cond> <ty> <op1>, <op2>`   
    注: 
        1. result is of type i1, or <N\*i1> if op1, op2 are vectors.  
        2. `<cond>` = eq且op1, op2是数字时，若op1 = op2，`<result>` = true,否则`<result>` = false。 
    * Memory allocation: `<result> = alloca <type>`
    * Write memory: `store <ty> <value>, <ty>* <pointer>`
    * Read memory: `<result> = load <ty>, <ty>* <pointer>`
    * Sub: `<result> = sub <ty> <op1>, <op2> ;result = op1 - op2`
    * Add: `<result> = add <ty> <op1>, <op2> ;result = op1 + op2`
    * Return from function: `ret <ty> <value>`
1. 实验2：编写`fib_gen.cpp`  
    1. 函数结构：  
        * 首先声明`LLVMContext context;`和`IRBuilder<> builder(context);`。
        * 生成的ll程序由2个函数int32 fib(int32)和int32 main()组成。使用Function::Create函数分别生成两个对应Function *指针。
        * fib函数由5个基本块构成，label分别为entry, IfZero, IfNotZero, IfOne, Otherwise。main函数由4个基本块构成，label分别为entry, cmp, loop, exit。对每个基本块，使用BasicBlock::Create函数构造一个对应的基本块。
        * 每个基本块的开头调用`builder.SetInsertPoint()`函数，结尾调用`builder.ClearInsertionPoint()`函数。
    1. 对于基本块中的每种LLVM IR语句，对应的对builder进行操作的函数分别为：
    
         LLVM IR  | C++ Function
        ----------|-------------
        `br i1 <cond>, label <iftrue>, label <iffalse>`|`BranchInst *CreateCondBr(Value *Cond, BasicBlock *True, BasicBlock *False,MDNode *BranchWeights = nullptr,MDNode *Unpredictable = nullptr);`
        `br label <dest>`|`BranchInst *CreateBr(BasicBlock *Dest);`
        `<result> = icmp eq <ty> <op1>, <op2>`|`Value *CreateICmpEQ(Value *LHS, Value *RHS, const Twine &Name = "");`
        `<result> = alloca <type>` |`AllocaInst *CreateAlloca(Type *Ty, Value *ArraySize = nullptr,  const Twine &Name = "");`
        `store <ty> <value>, <ty>* <pointer>`|`StoreInst *CreateStore(Value *Val, Value *Ptr, bool isVolatile = false);`
        `<result> = load <ty>, <ty>* <pointer>`|`LoadInst *CreateLoad(Type *Ty, Value *Ptr, const Twine &Name = "");`
        `<result> = sub <ty> <op1>, <op2>`|`Value *CreateSub(Value *LHS, Value *RHS, const Twine &Name = "", bool HasNUW = false, bool HasNSW = false);`
        `<result> = add <ty> <op1>, <op2>`|`Value *CreateAdd(Value *LHS, Value *RHS, const Twine &Name = "", bool HasNUW = false, bool HasNSW = false)`
        `ret <ty> <value>`|`ReturnInst *CreateRet(Value *V)`
        
    1. 对于立即数作为操作数的语句，如`%cond1 = icmp eq i32 %n, 0`，0需要用`Value *`类型指针代替。这里使用`auto Zero = ConstantInt::get(Type::getInt32Ty(context),0,true);`，Zero即可作为`Value *`类型指针使用。
    1. fib函数中，需要取得传入的参数，这里使用`auto n = &(*func_fib->arg_begin());`，n即为传入的i32类型参数。当调用fib函数时，则用`std::vector<Value *>`类型的容器，装入待传输的参数，作为`CreateCall`函数的参数传给fib即可。
    1. 最后使用`module->print(outs(), nullptr);`来打印生成的LLVM IR程序。
