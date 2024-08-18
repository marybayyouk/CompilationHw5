#include "CodeGenerator.h"

void CodeGenerator::defineLable(const string& label) {
    buffer.emit(label + ":");
}

void CodeGenerator::emitBranchToLabel(const string& label) {
    buffer.emit("br label %" + label);
}

void emitProgramStart() {
    buffer.printGlobalBuffer();
    buffer.printCodeBuffer();
}

void CodeGenerator::generateGlobalVar(const string& name, const string& type) {
    buffer.emit("@" + name + " = global i32 0");
}

string CodeGenerator::generateLoad(int offset, const string& ptr) {
    if (offset < 0)
        return "";
    string stackReg = freshReg();
    string loadReg = freshReg();
    //get the address of the stack
    buffer.emit(stackReg + " = getelementptr i32, i32* " + ptr + ", i32 " + to_string(offset));
    //load the value from the stack
    buffer.emit(loadReg + " = load i32, i32* " + stackReg);
    return loadReg; 
}

void CodeGenerator::generateStore(int offset, const string& valueReg, const string& ptr) {
    if (offset < 0)
        return;
    string stackReg = freshReg();
    //get the address of the stack
    buffer.emit(stackReg + " = getelementptr i32, i32* " + ptr + ", i32 " + to_string(offset));
    //store the value in the stack
    buffer.emit("store i32 " + valueReg + ", i32* " + stackReg);
}

void CodeGenerator::generateBinaryInst(Expression* exp, string& lhs, const string& rhs, string op, string inst) {
    if (inst == "BINOP") { 
        string resReg = freshReg();
        string binopOp = getBinopOp(op);  ///get the operation
        if (binopOp == "/") {   
            checkDivZero(rhs);  ///check if division by zero
            if(exp->getType() == "INT") {
                binopOp = "sdiv";
            }
            else {
                binopOp = "udiv";
            }
        }
        buffer.emit(resReg + " = " + binopOp + " i32 " + lhs + ", " + rhs);
        if (exp->getType() == "BYTE") {
            string truncReg = freshReg();
            buffer.emit(truncReg + " = and i32 255, " + resReg);
        }
    }
    else if(inst == "RELOP") {
        string resReg = freshReg();
        string relopOp = getRelopOp(op);  ///get the operation
        buffer.emit(resReg + " = icmp " + relopOp + " i32 " + lhs + ", " + rhs);
    }
}

string CodeGenerator::generateIcmp(const string& cond, const string& lhs, const string& rhs) {
    string resReg = freshReg();
    buffer.emit(resReg + " = icmp " + cond + " i32 " + lhs + ", " + rhs);
    return resReg;
}

void CodeGenerator::generateCondBranch(const string& condReg, const string& trueLabel, const string& falseLabel) {
    buffer.emit("br i1 " + condReg + ", label %" + trueLabel + ", label %" + falseLabel);
}   

void CodeGenerator::generateUncondBranch(const string& label) {
    buffer.emit("br label %" + label);
}

void CodeGenerator::generateFunctionCall(Node* node, CodeBuffer& buffer) {
    string funcName = node->getValue();
    buffer.emit("call void @" + funcName + "()");
}

void checkDivZero(const string& reg) {
    string zeroReg = freshReg();
    buffer.emit(zeroReg + " = alloca i32");
    buffer.emit("store i32 0, i32* " + zeroReg);
    string cmpReg = freshReg();
    buffer.emit(cmpReg + " = icmp eq i32 0, " + reg);
    string zeroLabel = allocateLable("divByZero");
    string nonZeroLabel = allocateLable("nonDivByZero");
    buffer.emit("br i1 " + cmpReg + ", label %" + zeroLabel + ", label %" + nonZeroLabel);
    buffer.emit(zeroLabel + ":");
    buffer.emit("call void @printDivByZero()");
    buffer.emit("call void @exit(i32 0)");
    buffer.emit("br label %" + nonZeroLabel);
    buffer.emit(nonZeroLabel + ":");
}