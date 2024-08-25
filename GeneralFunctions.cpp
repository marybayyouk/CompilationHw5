#include "GeneralFunctions.hpp"

extern CodeBuffer buffer;

string upperCase(string str) {
    for (char& c : str) {
        c = toupper(c);
    }
    return str;
}

vector<string> convertVectorToUpperCase(vector<string>& toUpper) {
    vector<string> toRet;
    for (string name : toUpper) {
        toRet.push_back(upperCase(name));
    }
    return toRet;
}

bool LegalType(string typeOne, string typeTwo) {
    if ((typeOne == "int" && typeTwo == "byte") || (typeOne == typeTwo)) {
        return true;
    } 
    return false;
}

bool isLegalFunc(string func, bool functionType) {
    if (func == "print" || func == "printi" || func == "readi") {
        if (!functionType) {
            return false;
        }
    }
    return true;
}

string TypeToLLVM(string type) {
    if (type == "int") {
        return "i32";
    } else if (type == "byte") {
        return "i8";
    } else if (type == "bool") {
        return "i1";
    }
    return "i8*";
}

string getBinopOp(string op) {
    if (op == "ADD") {
        return "add";
    } else if (op == "SUB") {
        return "sub";
    } else if (op == "MUL") {
        return "mul";
    } 
    
    return "DIV";
}

string getRelopOp(string op) {
    if (op == "EQ") {
        return "eq";
    } else if (op == "NE") {
        return "ne";
    } else if (op == "LT") {
        return "slt";
    } else if (op == "LE") {
        return "sle";
    } else if (op == "GT") {
        return "sgt";
    } 
    return "sge";
}

string freshReg() { 
    int static currReg = 1;
    return "%" + to_string(currReg++);
}

string freshGlobalReg() {
    int static currReg = 1;
    return "@g" + to_string(currReg++);
}

string allocateLable(const string& prefix) {  
    int static currLabel = 1;
    return prefix + to_string(currLabel++);
}

string getCallEmitLine(string funcName, string reg) { //takeen
    string cmd;
    if (funcName == "print") {
        cmd = "call void @print(i8* " + reg + ");";
    } else if (funcName == "printi") {
        cmd = "call void @printi(i32 0);";
    } else if (funcName == "readi") {
        cmd = "int = call i32 @readi(i32 " + reg + ");";
    }
    return cmd;
}

string emitTruncation(const string& reg, const string& type1, const string& type2, bool is2Types) { //takeen
    string trunReg = freshReg();
    if(is2Types) {
        if (type1 == "byte" && type2 == "int") { 
            buffer.emit(trunReg + " = trunc i32 " + reg + " to i8");
        }
        else if (type1 == "int" && type2 == "byte") {
            buffer.emit(trunReg + " = zext i8 " + reg + " to i32");
        }   
    }
    else {
        buffer.emit(trunReg + " = load i32, i32* " + reg);
        if (type1 == "byte") {
            buffer.emit(trunReg + " = trunc i32 " + reg + " to i8");
        }
        else if (type1 == "BOOL") {
            buffer.emit(trunReg + " = zext i8 " + reg + " to i1");
        }
    }    
    
    return trunReg;
}
