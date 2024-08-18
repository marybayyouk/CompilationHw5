#include "ProgramTypes.h"
#include "SymbolTable.h"
#include "cg.hpp"
#include <string>
#include <vector>

extern CodeBuffer buffer;
using namespace std;

string getBinopOp(string op); ///get binary operation
string getRelopOp(string op); ///get relational operation
string freshReg(); ///allocate new register
string freshGlobalReg(); ///allocate new global register
string freshLabel(const string& prefix); ///allocate new label

class CodeGenerator{
//private:
public:
    CodeGenerator() = default;
    ~CodeGenerator() = default;
    string checkDivZero(const string& reg); ///check if division by zero
    string generateLoad(int offset, const string& ptr); ///generate load instruction
    void generateStore(int offset, const string& valueReg, const string& ptr); ///generate store instruction
    void generateBinaryInst(Expression* exp, string& lhs, const string& rhs, string op, string inst); ///generate Binop/Relop instruction
    string generateIcmp(const string& cond, const string& lhs, const string& rhs);  ///generate iCompare operation
    void generateCondBranch(const string& condReg, const string& trueLabel, const string& falseLabel); ///generate conditional branch
    void generateUncondBranch(const string& label); ///generate unconditional branch
    void generateFunctionCall(Node* node, CodeBuffer& buffe); ///generate function call
    void generateReturn(const string& valueReg); ///generate return instruction
    string generateAlloca(); ///generate alloca instruction
    string generateGEP(const string& ptr, const vector<string>& indices); ///generate getElementPtr instruction
    ///phi instruction
    void generatePhi(const string& resReg, const string& type, const vector<pair<string, string>>& labelsAndRegs);
};