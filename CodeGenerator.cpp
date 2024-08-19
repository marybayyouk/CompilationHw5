#include "CodeGenerator.h"
#include "GeneralFunctions.h"
#include "ProgramTypes.h"
#include "cg.hpp"

extern StackTable scopes;

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

void CodeGenerator::generateReturn() {
    buffer.emit("ret i32 0");
}

void CodeGenerator::closeFunction() {
        generateReturn();
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

void CodeGenerator::generateJumpStatement(const string& label) {
    buffer.emit("br label %" + label);
}




//  void CodeGenerator::generateJumpStatement(const string& label) {
//     auto scope = scopes.getScope();
//     if ()
//     buffer.emit("br label %" + 
// }

// void handle_continue() {
//     auto scope = find_last_while();

//     string cmd = "br label %" + scope->sl;
//     buffer.emit(cmd);
// }

// cope *find_last_while() {
//     scope* s = symbol_table::get_instance()->current;
//     while (s != NULL){
//         if (s->scope_type == "while"){
//             return s;
//         }
//         s = s->parent;
//     }
//     return nullptr;
// }

// void handle_while(Node *exp) {
//     auto currScope = 
//     auto scope = find_last_while();
//     auto expNode = (ExpNode *) exp;

//     scope->el = expNode->false_l;
//     string cmd = expNode->true_l + ":";
//     buffer.emit(cmd);

// }





// void general::if_else_code(Exp* exp, Label* label)
// {
//     buffer.emit("br i1 " + exp->reg + ", label %" + label->true_label + ", label %"+ label->false_label);
// }