#include "CodeGenerator.h"

void CodeGenerator::defineLable(const string& label) {
    buffer.emit(label + ":");
}

void CodeGenerator::emitBranchToLabel(const string& label) {
    buffer.emit("br label %" + label);
}

void CodeGenerator::emitProgramStart() {
    buffer.printGlobalBuffer();
    buffer.printCodeBuffer();
}

void CodeGenerator::emitTypesLiteral(Expression* exp, const string& type) {
    if (type == "BYTE") {
        buffer.emit(exp->getReg() + " = add i32 " + exp->getValue() + ", 0");
    }
    else if (type == "BOOL") {
        buffer.emit("; Alloc&Emit BOOL, expName: " + exp->getValue());
        buffer.emit("br lable @");
    }
    else if (type == "INT") { ///NUM
        buffer.emit(exp->getReg() + " = add i32 " + exp->getValue() + ", 0");
    }
    else if (type == "STRING") {
        string globalReg = freshGlobalReg();
        string strReg = freshReg();
        ///MAYBE NEXT LINE SHOULD BE -1 AND NOT +1
        buffer.emit(globalReg + " = constant [" + to_string(exp->getValue().size() + 1) + " x i8]" + " c" + exp->getValue() + "\\00\"");
        buffer.emit(strReg + " = getelementptr[" + to_string(exp->getValue().size() + 1) + " x i8]" 
                        + ", " + to_string(exp->getValue().size() + 1) + " x i8]*  " + globalReg + ", i32 0, i32 0");
        exp->setReg(strReg);
    }
    
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

void CodeGenerator::checkDivZero(const string& reg) { ///NEED TO CHECK IT LATER NOT SURE IF IT WORKS
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


    buffer.emit("@.DIV_BY_ZERO_ERROR = internal constant [23 x i8] c\"Error division by zero\\00\"");
    // Define the function that checks for division by zero
    buffer.emit("define void @check_division(i32 %value) {");
    // Compare the input value to zero
    buffer.emit("  %isZero = icmp eq i32 %value, 0");
    // Conditional branch: if %isZero is true, go to ILLEGAL, else go to LEGAL
    buffer.emit("  br i1 %isZero, label %ILLEGAL, label %LEGAL");
    // ILLEGAL block - handles division by zero
    buffer.emit("ILLEGAL:");
    buffer.emit("  call void @print(i8* getelementptr([23 x i8], [23 x i8]* @.DIV_BY_ZERO_ERROR, i32 0, i32 0))");
    buffer.emit("  call void @exit(i32 0)");
    buffer.emit("  ret void");
    // LEGAL block - continue if no division by zero
    buffer.emit("LEGAL:");
    buffer.emit("  ret void");

    // End of function
    buffer.emit("}");
}

string CodeGenerator::generateAlloca() {
    string allocaReg = freshReg();
    buffer.emit(allocaReg + " = alloca i32 50"); ///allocate stack
    return allocaReg;
}

void CodeGenerator::generateReturn() {
    buffer.emit("ret i32 0");
    buffer.emit("}");
}

void CodeGenerator::generatePhi(const string& resReg, const string& type, const vector<pair<string, string>>& labelsAndRegs) {
    string phiReg = freshReg();
    string phiStr = "phi i32 ";
    for (auto& labelAndReg : labelsAndRegs) {
        phiStr += "[ " + labelAndReg.first + ", %" + labelAndReg.second + " ], ";
    }
    phiStr.pop_back();
    phiStr.pop_back();
    buffer.emit(phiReg + " = " + phiStr);
    buffer.emit(resReg + " = " + phiReg);
}

