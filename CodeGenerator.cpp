#include "CodeGenerator.hpp"
#include "ProgramTypes.hpp"

extern CodeBuffer buffer;

void CodeGenerator::emitGlobals() {
    string rbp = buffer.freshReg();
    stackTable.getScope()->getBaseReg() = rbp;
    buffer.emit("define i32 @main(){");
    buffer.emit(rbp + " = alloca i32, i32 50");
    buffer.emitGlobal("@.int_specifier_scan = constant [3 x i8] c\"%d\\00\"");
    buffer.emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    buffer.emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
    buffer.emitGlobal("@.DIV_BY_ZERO_ERROR = constant [23 x i8] c\"Error division by zero\\00\"");
    buffer.emitGlobal("declare i32 @scanf(i8*, ...)");
    buffer.emitGlobal("declare i32 @printf(i8*, ...)");
    buffer.emitGlobal("declare void @exit(i32)");
    buffer.emitGlobal("");
    buffer.emitGlobal("");
    buffer.emitGlobal("define void @print(i8*) {");
    buffer.emitGlobal(" %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0");
    buffer.emitGlobal(" call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)");
    buffer.emitGlobal(" ret void");
    buffer.emitGlobal("}");
    buffer.emitGlobal("");
    buffer.emitGlobal("");
    buffer.emitGlobal("define i32 @readi(i32) {");
    buffer.emitGlobal(" %ret_val = alloca i32");
    buffer.emitGlobal(" %spec_ptr = getelementptr [3 x i8], [3 x i8]* @.int_specifier_scan, i32 0, i32 0");
    buffer.emitGlobal(" call i32 (i8*, ...) @scanf(i8* %spec_ptr, i32* %ret_val)");
    buffer.emitGlobal(" %val = load i32, i32* %ret_val");
    buffer.emitGlobal(" ret i32 %val");
    buffer.emitGlobal("}");
    buffer.emitGlobal("");
    buffer.emitGlobal("");
    buffer.emitGlobal("define void @printi(i32) {");
    buffer.emitGlobal(" %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0");
    buffer.emitGlobal(" call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)");
    buffer.emitGlobal(" ret void");
    buffer.emitGlobal("}");
    buffer.emitGlobal("");
    buffer.emitGlobal("");
    buffer.emitGlobal("define void @check_division_by_zero(i32) {");
    buffer.emitGlobal(" %valid = icmp eq i32 %0, 0");
    buffer.emitGlobal(" br i1 %valid, label %ILLEGAL, label %LEGAL");
    buffer.emitGlobal("ILLEGAL:");
    buffer.emitGlobal(" call void @print(i8* getelementptr([23 x i8], [23 x i8]* @.DIV_BY_ZERO_ERROR, i32 0, i32 0))");
    buffer.emitGlobal(" call void @exit(i32 0)");
    buffer.emitGlobal(" ret void");
    buffer.emitGlobal("LEGAL:");
    buffer.emitGlobal(" ret void");
    buffer.emitGlobal("}");
}

//Generate Return Instruction for all the functions EXCEPT main
void CodeGenerator::emitFuncRet() {
    buffer.emit("ret i32 0");
}

//Generate Return Instruction for main
void CodeGenerator::emitProgramEnd(){
    emitFuncRet();
    buffer.emit("}");
}

//Define Label - Start of a new block
void CodeGenerator::defineLable(const string& label) {
    buffer.emit(label + ":");
}

void CodeGenerator::checkDivZero(const string& reg) { 
    string zeroR = buffer.freshReg();;
    string compareReg = buffer.freshReg();;
    string IllegalDivLabel = allocateLable("divByZero");
    string LegalDivLable = allocateLable("nonDivByZero");

    buffer.emit(zeroR + " = alloca i32");
    buffer.emit("store i32 0, i32* " + zeroR);
    buffer.emit(compareReg + " = icmp eq i32 0, " + reg);
    buffer.emit("br i1 " + compareReg + ", label %" + IllegalDivLabel + ", label %" + LegalDivLable);
    buffer.emit(IllegalDivLabel + ":");
    buffer.emit("call void @printDivByZero()");
    buffer.emit("call void @exit(i32 0)");
    buffer.emit("br label %" + LegalDivLable);
    buffer.emit(LegalDivLable + ":");
}

void CodeGenerator::emitProgramStart() {
    buffer.emit("define i32 @main(){");
}

string CodeGenerator::generateAlloca() {
    string allocaReg = freshGlobalReg();
    buffer.emit(allocaReg + " = alloca i32 50"); ///allocate stack
    return allocaReg;
}

//exp->reg = generateLoad(offset, ptr, exp->getType());
string CodeGenerator::generateLoad(int offset, const string& ptr, string expType) {
    if (offset < 0)
        return "%" + to_string(offset);
    string reg = buffer.freshReg();
    string stackReg = buffer.freshReg();
    //get the address of the stack
    buffer.emit(stackReg + " = getelementptr i32, i32* " + ptr + ", i32 " + to_string(offset));
    buffer.emit(reg + " = load i32, i32* " + stackReg);
    return reg;
}

string CodeGenerator::generateIcmp(const string& op, const string& lhs, const string& rhs) {
    string resReg = buffer.freshReg();
    buffer.emit(resReg + " = icmp " + op + " i32 " + lhs + ", " + rhs);
    return resReg;
}

void CodeGenerator::generateGlobalVar(const string& name, const string& type) {
    buffer.emit("@g" + name + " = global i32 0");
}

void CodeGenerator::generateStore(int offset, const string& valueReg, const string& ptr) { 
    string stackReg = buffer.freshReg();
    //get the address of the stack
    buffer.emit(stackReg + " = getelementptr i32, i32* " + ptr + ", i32 " + to_string(offset));
    //store the value in the stack
    buffer.emit("store i32 " + valueReg + ", i32* " + stackReg);
}

//Condetional Jumo To New Label - End of the curr block
void CodeGenerator::generateCondBranch(const string& condReg, const string& trueLabel, const string& falseLabel) {
    buffer.emit("br i1 " + condReg + ", label %" + trueLabel + ", label %" + falseLabel);
}   

//UnCondetional Jump To New Label - End of the curr block
void CodeGenerator::generateUncondBranch(const string& label) { 
    buffer.emit("br label %" + label);
}

// Break and Continue
void CodeGenerator::generateJumpStatement(const string& label) { 
    if(label == "BREAK") 
        generateUncondBranch(stackTable.getScope()->getEntryLabel());
    else if(label == "CONTINUE") 
        generateUncondBranch(stackTable.getScope()->getNextLabel());
}
