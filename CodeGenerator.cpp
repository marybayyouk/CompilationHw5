#include "CodeGenerator.h"
#include "GeneralFunctions.h"
#include "ProgramTypes.h"
#include "cg.hpp"


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
    if (type == "BYTE") { ///NUMB
        buffer.emit(exp->getReg() + " = add i18 " + exp->getValue() + ", 0");
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
    buffer.emit("@g" + name + " = global i32 0");
}

//exp->reg = generateLoad(offset, ptr, exp->getType());
string CodeGenerator::generateLoad(int offset, const string& ptr, string expType) {
    if (offset < 0)
        return "%" + to_string(offset);
    string stackReg = freshReg();
    //get the address of the stack
    buffer.emit(stackReg + " = getelementptr i32, i32* " + ptr + ", i32 " + to_string(offset));
    
    return emitTruncation(stackReg, expType, " ", false);
}

void CodeGenerator::generateStore(int offset, const string& valueReg, const string& ptr) {
    if (offset < 0)
        return;
    string stackReg = freshReg();
    //get the address of the stack
    buffer.emit(stackReg + " = getelementptr i32, i32* " + ptr + ", i32 " + to_string(offset));
    //store the value in the stack
    buffer.emit("store i32 " + valueReg + ", i32* " + stackReg);
    ///IN PROGRAM TYPES NEED TO HANDLE NODE->TYPE == BOOL
}

string CodeGenerator::generateIcmp(const string& op, const string& lhs, const string& rhs) {
    string resReg = freshReg();
    buffer.emit(resReg + " = icmp " + op + " i32 " + lhs + ", " + rhs);
    return resReg;
}

void CodeGenerator::generateBinaryInst(const string& expType, const string& lhs,const string& rhs, string op, string inst) {
    if (inst == "BINOP") { //NEED TO REVIEW THIS PART - SOMETHING IS WRONG
        string resReg = freshReg();
        op = getBinopOp(op);
        if (op == "DIV") {   
            checkDivZero(rhs);  ///check if division by zero
            if(expType == "INT") {
                op = "sdiv";
            }
            else {
                op = "udiv";
            }
        }
        buffer.emit(resReg + " = " + op + " i32 " + lhs + ", " + rhs);
        if (expType == "BYTE") {
            string truncReg = freshReg();
            buffer.emit(truncReg + " = and i32 255, " + resReg);
        }
    }
    else if(inst == "RELOP") {
        op = getRelopOp(op);
        generateIcmp(op, lhs, rhs);
    }
}

void CodeGenerator::generateUnaryInst(const string& expType, const string& reg, string op) {
    string resReg = freshReg();
    if (op == "NOT") {
        buffer.emit(resReg + " = xor i1 1, " + reg);
    }
    else if (op == "NEG") {
        buffer.emit(resReg + " = sub i32 0, " + reg);
    }
}

void CodeGenerator::generateCondBranch(const string& condReg, const string& trueLabel, const string& falseLabel) {
    buffer.emit("br i1 " + condReg + ", label %" + trueLabel + ", label %" + falseLabel);
}   

void CodeGenerator::generateUncondBranch(const string& label) {
    buffer.emit("br label %" + label);
}

void CodeGenerator::generateFunctionCall(Node* terminalID) {
    string reg = freshReg();
    string cmd = getCallEmitLine(terminalID->getValue(), terminalID->getReg());
    buffer.emit(reg + " = " + cmd);
}

void CodeGenerator::checkDivZero(const string& reg) { ///NEED TO CHECK IT LATER NOT SURE IF IT WORKS
    string zeroR = freshReg();
    string compareReg = freshReg();
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

string CodeGenerator::generateAlloca() {
    string allocaReg = freshReg();
    buffer.emit(allocaReg + " = alloca i32 50"); ///allocate stack
    return allocaReg;
}

void CodeGenerator::funcRet() {
    buffer.emit("ret i32 0");
}

void CodeGenerator::mainRet() {
        funcRet();
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

// Break and Continue
void CodeGenerator::generateJumpStatement(const string& label) {
    if(label == "BREAK") {
        buffer.emit("br label %" + beginEndLabels.back().second);
    }
    else if(label == "CONTINUE") {
        buffer.emit("br label % " + beginEndLabels.back().first);
    }
}

