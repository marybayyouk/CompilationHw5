#include "GeneralFunctions.h"

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

string allocateLable(const string& prefix) {  
    int static currLabel = 0;
    return prefix + to_string(currLabel++);
}

string getCallEmitLine(string funcName, string reg) {
    string cmd;
    if (funcName == "print") {
        cmd = "call void @print(i8* getelementptr ([ " + reg + " x i8], [" + reg +
                      " x i8]* " + "0" + ", i32 0, i32 0))";
    } else if (funcName == "printi") {
        cmd = "call void @printi(i32 0);";
    } else if (funcName == "readi") {
        cmd = "call i32 @readi(i32 0);";
    }
    return cmd;
}
