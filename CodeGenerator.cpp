#include "CodeGenerator.h"

string getBinopOp(string op) {
    if (op == "+") {
        return "add";
    } else if (op == "-") {
        return "sub";
    } else if (op == "*") {
        return "mul";
    } 
    else {
        return "DIV";
    }
}

string getRelopOp(string op) {
    if (op == "==") {
        return "eq";
    } else if (op == "!=") {
        return "ne";
    } else if (op == "<") {
        return "slt";
    } else if (op == "<=") {
        return "sle";
    } else if (op == ">") {
        return "sgt";
    } else if (op == ">=") {
        return "sge";
    }
    return "";
}

string freshReg() {
    int static currReg = 0;
    return "%" + to_string(currReg++);
}

string freshGlobalReg() {
    int static currReg = 0;
    return "@g" + to_string(currReg++);
}

string freshLabel(const string& prefix) {  
    int static currLabel = 0;
    return prefix + to_string(currLabel++);
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