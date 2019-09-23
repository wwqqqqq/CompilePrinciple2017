# 实验2  C1语言的LLVM IR代码生成器

## 遇到的错误
1. visit函数怎样识别`global_def_syntax` `expr_syntax` `stmt_syntax`  
直接使用`visit()`函数，参数为`global_def_syntax`、`expr_syntax`、`stmt_syntax`时会报错。  
解决方法：考虑到`syntax_tree.h`中对各种结构体类型中定义的函数`virtual void accept(syntax_tree_visitor &visitor) override final;`。将`visit(expr)`改为`expr->accept(*this);`便可以正常运行。对`stmt_syntax`和`global_def_syntax`同理。

2. 声明非全局数组时，使用`AllocaInst *CreateAlloca(Type *Ty, Value *ArraySize)`，将数组大小直接传入CreateAlloca中，Ty设为`Type::getInt32Ty(context)`。但声明全局数组时，ty为`ArrayType::get(Type::getInt32Ty(context),length)`。这样会造成全局数组和非全局数组数据类型不同，在load时若使用一种方法解引用，会造成错误。  
解决方法：将它们的数据类型统一改成`ArrayType::get(Type::getInt32Ty(context),length)`，索引时解两次引用`getelementptr 0, index`。


## 分析与设计
1. 如何声明数组 
    * 对于全局变量，见4。
    * 对于非全局变量，与普通变量相似，同样使用`CreateAlloca`函数，不同的是全局变量的类型应该是`ArrayType`。由于C1语言只识别int32类型的一维数组，先通过`visit(array_length)`来获得数组长度length，则它的数组类型即为`ArrayType::get(Type::getInt32Ty(context),length)`。

2. 如何对数组元素进行索引  
调用`CreateGEP`函数对数组进行索引。因为声明的数组类型都为`ArrayType::get(Type::getInt32Ty(context),length)`，故虽然是一维数组，索引时却需要解两次引用，即`CreateGEP`的第二个参数`ArrayRef<Value *> IdxList`中应有两个元素。若索引为`(Value *)index`，则`IdxList`应为`{ConstantInt::get(Type::getInt32Ty(context),0,true), index}`。`(Value *)PTR`为数组起始地址（`lookup_variable(node.name)`返回值的第一个元素）。调用`CreateGEP(PTR,IdxList);`即可得到索引后的元素地址。

3. 如何对非全局数组赋初值  
对`int32`类型数据赋初值非常简单，直接使用`CreateStore`即可。这里对数组元素赋初值虽然也使用`CreateStore`函数，但需要先遍历Initializers，再使用`CreateGEP`函数获得数组中各元素的地址。值得注意的是，若Initializers的大小大于数组长度，应报错。     

4. 如何声明全局变量  
`value_result = new GlobalVariable(*(module.get()),ty,node.is_constant,GlobalValue::LinkageTypes::ExternalLinkage, init);`   
注：此处ty类型为`Type *`，init类型为`Constant *`。  
    * 如果是int32类型数据，`ty = Type::getInt32Ty(context)`；由于C1语言中，全局变量初始化必须使用常数，故将`constexpr_expected`设为true，调用`visit((node.initializers)[0])`，`init = ConstantInt::get(Type::getInt32Ty(context), const_result)`。
    * 若为大小为length的数组，`ty = ArrayType::get(Type::getInt32Ty(context),length)`；定义一个`std::vector<Constant *> vec`，遍历Initializers，并将得到的`Constant *`类型数据加入容器vec中，`init = ConstantArray::get(ty,vec)`。

5. 在遇到错误时，将`error_flag`置为true，调用`err.error(node.line, node.pos, "Error report");`即可。

6. 需要设置`constexpr_expected`的情况：  
    * 声明数组时`visit(array_length)`；
    * 全局变量获取initializers时，因为需要确保初始化的是常数。

7. 在访问全局定义的变量和函数时，调用`enter_scope()`，`visit(assembly &node)`的结尾`xit_scope()`；在进入`block_syntax`时调用`enter_scope()`，退出前调用`exit_scope()`。

