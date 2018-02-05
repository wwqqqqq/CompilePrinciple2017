##1. source code
    //hello.c
    #include <stdio.h>
    
    int main(void)
    {
    	printf("Hello world!\n");
    	return 0;
    }

##2. gcc -S hello.c  (gcc -S -m64 hello.c)
		.file	"hello.c"
	 	.section	.rodata
	 .LC0:
	 	.string	"Hello world!"     //the original string is ended with '\n', so the compile delete last character '\n' and use function puts instead of printf
	 	.text
	 	.globl	main
	 	.type	main, @function
	 main:                          //the main function:
	 .LFB0:
	 	.cfi_startproc
	 	//cfi --- calling frame info
	 	pushq	%rbp               //push the quadword in register rbq into the stack
	 	.cfi_def_cfa_offset 16
	 	.cfi_offset 6, -16
	 	movq	%rsp, %rbp         //copy the quadword in register rsp into register rbp
	 	                           //rsp --- stack pointer (64-bit)
	 	                           //rbp --- base pointer (64-bit)
	 	.cfi_def_cfa_register 6
	 	movl	$.LC0, %edi        //register edi passes the first parameter of the function which will be called
	 	                           //.LC0 stores the location of the string "Hello world!"
	 	call	puts               //calling function puts (instead of printf)
	 	movl	$0, %eax           //set the return value of function main to 0
	 	popq	%rbp               //restore the quadword in rbp from the stack
	 	.cfi_def_cfa 7, 8
	 	ret                        //return
	 	.cfi_endproc
	 .LFE0:
	 	.size	main, .-main
	 	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.4) 5.4.0 20160609"
	 	.section	.note.GNU-stack,"",@progbits


##3. gcc -S -m32 hello.c 
	 	.file	"hello.c"
	 	.section	.rodata
	 .LC0:
	 	.string	"Hello world!"
	 	.text
	 	.globl	main
	 	.type	main, @function
	 main:
	 .LFB0:
	 	.cfi_startproc
	 	leal	4(%esp), %ecx     //ecx = esp + 4
	 	                          //esp -- stack pointer register (32-bit)
	 	                          //ecx -- general register or counter register (32-bit)
	 	.cfi_def_cfa 1, 0
	 	andl	$-16, %esp        //set the lowest 4bits of register esp to zero
	 	pushl	-4(%ecx)          //push the result of ecx minus 4 into stack (the data stored in esp before program executing)
	 	pushl	%ebp              //push the data in ebp into stack
	 	.cfi_escape 0x10,0x5,0x2,0x75,0
	 	movl	%esp, %ebp        //copy the 32-bit data from register esp into register ebp
	 	pushl	%ecx              //push the data stored in register ecx into stack
	 	.cfi_escape 0xf,0x3,0x75,0x7c,0x6
	 	subl	$4, %esp          //esp = esp - 4
	 	subl	$12, %esp         //esp = esp - 12
	 	pushl	$.LC0             //passing parameter of function "puts"
	 	call	puts              //calls function "puts"
	 	addl	$16, %esp         //esp = esp + 16
	 	movl	$0, %eax          //set register eax(the return value of fuction main) 0
	 	movl	-4(%ebp), %ecx    //ecx = ebp - 4
	 	.cfi_def_cfa 1, 0
	 	leave
	 	.cfi_restore 5
	 	leal	-4(%ecx), %esp
	 	.cfi_def_cfa 4, 4
	 	ret
	 	.cfi_endproc
	 .LFE0:
	 	.size	main, .-main
	 	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.4) 5.4.0 20160609"
	 	.section	.note.GNU-stack,"",@progbits
