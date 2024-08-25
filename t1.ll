@.int_specifier_scan = constant [3 x i8] c"%d\00"
@.int_specifier = constant [4 x i8] c"%d\0A\00"
@.str_specifier = constant [4 x i8] c"%s\0A\00"
@.DIV_BY_ZERO_ERROR = constant [23 x i8] c"Error division by zero\00"
declare i32 @scanf(i8*, ...)
declare i32 @printf(i8*, ...)
declare void @exit(i32)


define void @print(i8*) {
 %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0
 call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)
 ret void
}


define i32 @readi(i32) {
 %ret_val = alloca i32
 %spec_ptr = getelementptr [3 x i8], [3 x i8]* @.int_specifier_scan, i32 0, i32 0
 call i32 (i8*, ...) @scanf(i8* %spec_ptr, i32* %ret_val)
 %val = load i32, i32* %ret_val
 ret i32 %val
}


define void @printi(i32) {
 %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0
 call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)
 ret void
}


define void @check_division_by_zero(i32) {
 %valid = icmp eq i32 %0, 0
 br i1 %valid, label %ILLEGAL, label %LEGAL
ILLEGAL:
 call void @print(i8* getelementptr([23 x i8], [23 x i8]* @.DIV_BY_ZERO_ERROR, i32 0, i32 0))
 call void @exit(i32 0)
 ret void
LEGAL:
 ret void
}
define i32 @main(){
%reg1 = alloca i32, i32 50
:
br label %********label_2
:
br label %********label_2
********label_2:
%reg2 = phi i32 [ 1, % ], [ 0, % ]
%reg3 = getelementptr i32, i32* , i32 3
store i32 %reg2, i32* %reg3
%reg4 = getelementptr i32, i32* , i32 3
%reg5 = load i32, i32* %reg4
%reg6 = icmp ne i32 1, 
br i1 %reg6, label %********label_3, label %********label_4
********label_3:
********label_5:
:
br label %
:
:
:
br label %
:
********label_7:
********label_8:
:
br label %
:
br label %
:
br label %
:
ret i32 0
}
