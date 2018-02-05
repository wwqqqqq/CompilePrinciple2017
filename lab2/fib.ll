define i32 @fib(i32 %n){
entry:
	%cond1 = icmp eq i32 %n, 0
	br i1 %cond1, label %IfZero, label %IfNotZero
IfZero:							    ; preds = %entry
    ret i32 0
IfNotZero:						    ; preds = %entry
    %cond2 = icmp eq i32 %n, 1
	br i1 %cond2, label %IfOne, label %Otherwise
IfOne:							    ; preds = %IfNotZero
	ret i32 1
Otherwise:						    ; preds = %IfNotZero
    %0 = sub i32 %n, 1
	%1 = sub i32 %n, 2
    %temp1 = call i32 @fib(i32 %0)
	%temp2 = call i32 @fib(i32 %1)
	%res = add i32 %temp1, %temp2
    ret i32 %res
}

define i32 @main(){
entry:
	%ptrx = alloca i32
	store i32 0, i32* %ptrx
	%ptri = alloca i32
	store i32 0, i32* %ptri
	br label %cmp
cmp:							    ; preds = %entry, %loop
    %i1 = load i32, i32* %ptri
	%cond = icmp slt i32 %i1, 10
	br i1 %cond, label %loop, label %exit
loop:							    ; preds = %cmp
    %x1 = load i32, i32* %ptrx
	%temp = call i32 @fib(i32 %i1)
	%x2 = add i32 %x1, %temp
	store i32 %x2, i32* %ptrx
	%i2 = add i32 %i1, 1
	store i32 %i2, i32* %ptri
	br label %cmp
exit:							    ; preds = %cmp
    %x3 = load i32, i32* %ptrx
	ret i32 %x3
}

