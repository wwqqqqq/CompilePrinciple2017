#H10 answer
环境：  
处理器 Intel Core i7-4600U CPU @2.10GHz  
操作系统 Ubuntu 16.04 LTS  
编译器 gcc (Ubuntu 5.4.0-6ubuntu1~16.04.5) 5.4.0 20160609  

##6.18  
(a) sizeof(a)值为0。因为a的类型为`array[0] of array[4] of long`，`T.width = 0*4*long.width = 0`。  
(b) 实际使用gcc编译时，`a[0][0]`值为一不确定的数。gcc版本为 `gcc (Ubuntu 5.4.0-6ubuntu1~16.04.5) 5.4.0 20160609`  
分析产生的汇编代码(-m32)得，`a[0][0]`的起始地址为`ebp-16`， i的起始地址为`ebp-24`，j的起始地址为`ebp-20`。  
若按照第七章中语法制导翻译的方法进行编译，由于a的类型T,T.width = 0，故a和j的offset相同，故`a[0][0]`的值为8。

##补充题：  
1. 有如下C语言程序  
`int main(void){`  
`    int arr[2048][2048];`  
`    printf("%d\n",sizeof(arr));`  
`    return 0;`  
`}`  
Q: 输出结果是什么，为什么？  
A: 段错误（核心已转储）  
通过`ulimit -a`命令，可知所使用的系统(Ubuntu 16.04)中stack size为8192Kbytes；  
而在所用gcc编译器中(5.4.0 20160609)，`sizeof(int)`为4bytes。`sizeof(arr)`应为`4*2048*2048 = 16384 Kbytes`.  
故二维数组arr的大小超过了Linux系统中所规定的数据栈的大小，故运行时为arr分配存储空间失败，产生段错误。  
若将`int arr[2048][2048]`改为全局变量，则可以正常运行，输出结果为16777216.  

2. 有以下C语言程序：  
`#include <stdio.h>`  
`#include <string.h>`  
`int main(void){`  
    `char str1[]="012348";`  
    `char str2[]="abcx";`  
    `char str3[]="567";`  
    `char str4[]="defghij";`  
    `strcpy(str3,str4);`  
    `strcpy(str2,str1);`  
    `printf("%s\n%s\n%s\n%s\n",str1,str2,str3,str4);`  
    `return 0;`  
`}`  
Q: 输出结果是什么？为什么？  
A: 编译时加上参数`-m32`后的输出结果:  
8  
012348  
defg012348  
defghij  
原因： `<string.h>`库中的`strcpy`函数不检查字符串是否越界。所以当`strcpy(str1,str2);`中str2的strlen大于str1的strlen时，可能会超出分配给str1的内存范围。  
开始对四个char数组的声明后，给它们分配内存后，这一整块内存的内容为`"defghij\0567\0abcx\0012348\0"`，str1/str2/str3/str4分别指向'0'/'a'/'5'/'d'的地址。  
执行完`strcpy(str3,str4);`后，内存内容为`"defghij\0defghij\0\0012348\0"`，str1/str2/str3/str4分别指向'0'/第二个'h'/第二个'd'/第一个'd'的地址。次数若输出str1, str2, str3, str4，结果应为：  
012348  
hij  
defghij   
defghij  
执行完`strcpy(str2,str1);`后，内存内容为`"defghij\0defg01348\08\0"`，由于两次strcpy函数并未改变str1/str2/str3/str4所指向的内存地址，所以输出后得到结果:  
8  
012348  
defg012348  
defghij  

##test目录说明  
`618.c`: 课本习题6.18的程序（稍加修改）  
`618.s`: `gcc -S -m32 618.c -o 618.s`命令得到的汇编代码  
`ex1.c`: 补充题1的C语言程序  
`ex1_mortify.c`: 将ex1.c修改后可以正常运行的版本  
`ex2.c`: 补充题2的C语言程序  
`ex2.s`: `gcc -S -m32 ex2.c -o ex2.s`命令得到的汇编代码


