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
%r2 = alloca i32, i32 50
call void @print(i8* %r3);
"hello"
Statement -> Type ID Assign Exp SC: start
%r5 = getelementptr i32, i32* %r1, i32 3
store i32 %r4, i32* %r5
Statement -> Type ID Assign Exp SC: end
Statement -> Type ID Assign Exp SC: start
%r7 = getelementptr i32, i32* %r1, i32 4
store i32 %r6, i32* %r7
Statement -> Type ID Assign Exp SC: end
-------------------------***************------------------------------
%r10 = getelementptr i32, i32* %r1, i32 4
%r9 = load i32, i32* %r10
-------------------------***************------------------------------
%r11 = icmp ne i32 1, %r8
br i1 %r11, label %label_5, label %label_6
label_5:
call void @print(i8* %r12);
"val is true"
label_7:
-------------------------------ELSE-----------------------------
br label %
:
:
-----------------------------------------------------------------
call void @print(i8* %r13);
"val is false"
:
call void @print(i8* %r14);
"true"
-------------------------------IF-------------------------------
br label %
:
-----------------------------------------------------------------
label_7:
label_8:
And started: 
:
br label %
And ended: 
OR started: 
:
br label %
OR ended: 
:
call void @print(i8* %r15);
"true"
-------------------------------IF-------------------------------
br label %
:
-----------------------------------------------------------------
ret i32 0
}
