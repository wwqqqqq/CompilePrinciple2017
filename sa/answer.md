# Clang Static Analyzer

## 3.1   
**简要说明 test.c 、 AST.svg 、 CFG.svg 和 ExplodedGraph.svg 之间的联系与区别**  
1. AST是由C语言程序test.c生成的抽象语法树(abstract syntax tree)。是analyzer-checker对程序进行的语法分析。
1. CFG, Control Flow Graph, 是程序test.c编译生成的控制流图。每一个结点代表一个基本块，代表了程序执行过程中，遍历各基本块的逻辑/顺序。CFG是编译器优化和静态分析的重要工具。
1. 在flow-sensitive analysis中，若把不同点(ProgramPoint, State)对看成不同的结点，即，即使程序点相同，状态不同的结点也看成不同的结点。这样以来，分析完后产生的图的结点比原先的CFG的结点要多，这个新的图即为Exploded Graph。在CFG图中，结点是基本块；在Exploded Graph中，各结点则是[ProgramPoint, State]这样的状态对。Explodede Graph是用来表示执行中所探测到的所有可能的路径。

## 3.2
1. **Checker 对于程序的分析主要在 AST 上还是在 CFG 上进行？**   
CFG  
2. **Checker 在分析程序时需要记录程序状态，这些状态一般保存在哪里？**   
每当分析引擎发现一个新的语句时，它指示所有的checker监听该语句，让它们可以报告错误或修改语句。Checkers按照预先定义的顺序被调用，在ExplodedGraph中加入调用checker的调用链。故checker分析程序时记录的程序状态，一般被保存在ExplodedGraph中。
3. **简要解释分析器在分析下面程序片段时的过程，在过程中产生了哪些symbolic values? 它们的关系是什么？**    
```C
int x = 3, y = 4;
int *p = &x;
int z = *(p + 1);
```
    1. 第一行，产生了4个Sval，分别是concrete number 2和3，以及x和y的memory location。x的memory location与concrete number 2绑定，y的memory location与concrete number 3绑定。
    1. 第二行，产生1个新的Sval，即指针p的memory location，x在内存中的地址被存在p的地址中。
    1. 第三行，产生了3个SVal，计算p+1，作为了一个新的symbolic value，对它所指向的内存地址进行访问，得到一个symbolic value，即`*(p+1)`，最后计算左值，即z的memory location，将`*(p+1)`的值存入。

## 3.3
1. **LLVM 大量使用了 C++11/14的智能指针，请简要描述几种智能指针的特点、使用场合，如有疑问也可以记录在报告中.**  
    - **`std::unique_ptr`** is a smart pointer that owns and manages another object through a pointer and disposes of that object when the `unique_ptr` goes out of scope.  
当`unique_ptr`被销毁，或由于`operator=`或`reset()`操作指向其他对象时，`unique_ptr`原来所指的对象被删除，即`unqie_ptr`独占所指向的对象；当`unique_ptr`不指向任何对象时，被称为*empty*。  
`std::unique_ptr`常被用于管理对象的生存时间，包括：  
        1. 通过不管是正常还是异常时退出程序时，皆将对象删除，来确保处理动态生命周期对象的类和函数出现异常时的安全；   
        1. 将`unique_ptr`所独有的具有动态寿命的对象所有权传递给函数；  
        1. 从函数获取`unique_ptr`所独有的具有动态寿命的对象的所有权；  
        1. 作为移动感知容器中的元素类型(如`std::vector`)，保存指向动态分配对象的指针。  
    - **`std::shared_ptr`** is a smart pointer that retains shared ownership of an object through a pointer.   
与`unique_ptr`不同，一个对象可以同时被多个`shared_ptr`所指向。当最后一个指向某对象的`shared_ptr`被销毁或因`operator=`或`reset()`操作而指向其他对象时，该对象被销毁。当`shared_ptr`不指向任何对象时，被称为*empty*。  
`std::shared_ptr`包括两个指针：它所保存的指针和一个指向控制块的指针。控制块是一个动态分配内存的对象，包括被`shared_ptr`所管理的对象或指向该对象的指针，deleter，allocator，拥有该对象所有权的`shared_ptr`的数目，及指向该对象的`weak_ptr`的数目。  
    - **`std::weak_ptr`** is a smart pointer that holds a non-owning ("weak") reference to an object that is managed by `std::shared_ptr`. `weak_ptr`必须被转化为`shared_ptr`才可以访问它所指向的对象。  
`weak_ptr`被用于追踪对象，当被转化为`shared_ptr`时承担临时所有权，当原始的`shared_ptr`被销毁时，临时`shared_ptr`的存在可以延长对象的寿命。此外，`weak_ptr`还被用于打破`shared_ptr`的循环引用。  

2. **LLVM 不使用 C++ 的运行时类型推断（RTTI） ，理由是什么？LLVM 提供了怎样的机制来代替它？**  
理由：为了减少代码量和可执行程序的大小，LLVM不使用RTTI。RTTI(如`dynamic_cast<>;`)违反了C++中“只为使用的东西花费时间或空间”的一般性原则，如即使在所用的类中并没有使用RTTI，也会导致相关的检查/处理，导致代码量和可执行程序大小增大。  
作为代替，LLVM支持使用类似`isa<>`, `cast<>`, `dyn_cast<>`模板的hand-rolled形式的RTTI。这种RTTI是可选的，并可以被加入到任何类中，它比例如`dynamic_cast<>`的RTTI效率更高。  
    -  `isa<>`: It returns true or false depending on whether a reference or pointer points to an instance of the specified class.
    -  `cast<>`: It converts a pointer or reference from a base class to a derived class, causing an assertion failure if it is not really an instance of the right type. 
    -  `dyn_cast<>`: It checks to see if the operand is of the specified type, and if so, returns a pointer to it (this operator does not work with references). If the operand is not of the correct type, a null pointer is returned.
    -  `cast_or_null<>`: It works just like the cast<> operator, except that it allows for a null pointer as an argument (which it then propagates). 
    -  `dyn_cast_or_null<>`: It works just like the dyn_cast<> operator, except that it allows for a null pointer as an argument (which it then propagates).
3. **如果你想写一个函数，它的参数既可以是数组，也可以是`std::vector`，那么你可以声明该参数为什么类型？如果你希望同时接受 C 风格字符串和`std::string`呢？**  
    1. 声明该参数为`std::vector`。
    1. 声明该参数为`std::string`。
4. **你有时会在cpp文件中看到匿名命名空间的使用，这是出于什么考虑？**    
C++中的匿名命名空间类似于C语言中的`static`关键字，却拥有更加强大的功能。它指示编译器该命名空间仅在当前的翻译单元中可见，允许了更加积极的优化，并消除了发生符号名冲突的可能。

## 3.4  
SimpleStreamChecker: a checker for proper use of fopen/fclose APIs.  
- If a file has been closed with fclose, it should not be accessed again. Accessing a closed file results in undefined behavior.  
- If a file was opened with fopen, it must be closed with fclose before the execution ends. Failing to do so results in a resource leak.  

1. **这个 checker 对于什么对象保存了哪些状态？保存在哪里？**  
StreamState：文件是否打开/关闭的状态
将从Stream symbols到它们状态的映射，存储在ProgramState中。
2. **状态在哪些时候会发生变化？**  
文件使用fopen/fclose被打开/关闭的时候。
3. **在哪些地方有对状态的检查？**  
isLeaked函数
checkPreCall函数
checkDeadSymbols函数
checkPostCall函数
4. **函数`SimpleStreamChecker::checkPointerEscape`的逻辑是怎样的？实现了什么功能？用在什么地方？**  
`SimpleStreamChecker::checkPointerEscape(ProgramStateRef State, const InvalidatedSymbols &Escaped, const CallEvent *Call, PointerEscapeKind Kind)`  
逻辑：若Call调用确保无法关闭文件，则直接返回State；否则，遍历Escape，并从StreamMap中清楚Escape中指向的所有无效指针。  
功能：检查是否出现指针逃逸，并删除逃逸指针在StreamMap中的对应条目，以停止通过escape pointer来追踪stream。 
应用：fopen函数前，fclose函数后。
5. **根据以上认识，你认为这个简单的checker能够识别出怎样的bug？又有哪些局限性？请给出测试程序及相关的说明。**  
重复关闭文件；在打开文件前尚未关闭该文件。  
局限性：无法在其他对文件进行操作时识别，只能识别fopen, fclose是否出现错误。
对如下程序进行编译：  
```C
#include <stdio.h>
FILE *open(char *file) {
    return fopen(file, "r");
}
void f1(FILE *f){
    fclose(f);
}
void f2(FILE *f) {
    fclose(f);
}
int main() {
    FILE *f = open("foo");
    f1(f);
    fputs(f,"abcd");
    f2(f);
    return 0;
}
```
编译选项为：`clang --analyze -Xanalyzer -analyzer-checker=alpha.unix.SimpleStream dblclose.c`  
此时会报`warning: Closing a previously closed file stream`  
即checker对重复关闭文件进行报错warning，却不会对f1函数已经关闭后，再次对它进行fputs操作的错误操作，进行报错或warning。

## 3.5
1. **增加一个checker需要增加哪些文件？需要对哪些文件进行修改？**  
    1. 增加checker implementation file: `lib/StaticAnalyzer/Checker/CheckerName.cpp`。
    1. 修改`include/clang/StaticAnalyzer/Checkers/Checkers.td`。
    1. 修改`lib/StaticAnalyzer/Checkers/CMakeLists.txt`。
2. **阅读`clang/include/clang/StaticAnalyzer/Checkers/CMakeLists.txt`，解释其中的`clang_tablegen`函数的作用。**  
把Checkers.td中的checkers加入ClangSACheckers中。
3. **.td 文件在clang中出现多次，比如这里的`clang/include/clang/StaticAnalyzer/Checkers/Checkers.td`。这类文件的作用是什么？它是怎样生成C++头文件或源文件的？这个机制有什么好处？**  
    1. 定义C++头文件，Checkers等，如：
Alpha Packages/Core Checkers/Evaluate "builtin" functions/Uninitialized Values Checkers/C++ Checkers/Valist Checkers/Deadcode Checkers/Performance Checkers/Security Checkers/Taint Checkers/Unix API Checkers/Core Foundation Checkers/Checkers for LLVM development/Debugging Checkers/Clone Detection/
    2. `def Cplusplus : Package<"cplusplus">;`  
    3. 方便修改。
