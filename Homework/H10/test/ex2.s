	.file	"ex2.c"
	.section	.rodata
.LC0:
	.string	"%s\n%s\n%s\n%s\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	leal	4(%esp), %ecx
	.cfi_def_cfa 1, 0
	andl	$-16, %esp
	pushl	-4(%ecx)
	pushl	%ebp
	.cfi_escape 0x10,0x5,0x2,0x75,0
	movl	%esp, %ebp
	pushl	%ecx
	.cfi_escape 0xf,0x3,0x75,0x7c,0x6
	subl	$36, %esp
	movl	%gs:20, %eax
	movl	%eax, -12(%ebp)
	xorl	%eax, %eax
	movl	$858927408, -27(%ebp)
	movw	$14388, -23(%ebp)
	movb	$0, -21(%ebp)
	movl	$2019779169, -32(%ebp)
	movb	$0, -28(%ebp)
	movl	$3618357, -36(%ebp)
	movl	$1734763876, -20(%ebp)
	movl	$6973800, -16(%ebp)
	subl	$8, %esp
	leal	-20(%ebp), %eax
	pushl	%eax
	leal	-36(%ebp), %eax
	pushl	%eax
	call	strcpy
	addl	$16, %esp
	subl	$8, %esp
	leal	-27(%ebp), %eax
	pushl	%eax
	leal	-32(%ebp), %eax
	pushl	%eax
	call	strcpy
	addl	$16, %esp
	subl	$12, %esp
	leal	-20(%ebp), %eax
	pushl	%eax
	leal	-36(%ebp), %eax
	pushl	%eax
	leal	-32(%ebp), %eax
	pushl	%eax
	leal	-27(%ebp), %eax
	pushl	%eax
	pushl	$.LC0
	call	printf
	addl	$32, %esp
	movl	$0, %eax
	movl	-12(%ebp), %edx
	xorl	%gs:20, %edx
	je	.L3
	call	__stack_chk_fail
.L3:
	movl	-4(%ebp), %ecx
	.cfi_def_cfa 1, 0
	leave
	.cfi_restore 5
	leal	-4(%ecx), %esp
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.5) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
