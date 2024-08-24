#include "SymbolTable.hpp"
#include "GeneralFunctions.hpp"
#include "cg.hpp"
#include <string>
#include <vector>

extern StackTable stackTable;
using namespace std;

//static vector<pair<string, string>> beginEndLabels;

class CodeGenerator{
//private:
public:
    CodeGenerator() = default;
    ~CodeGenerator() = default;
    void emitGlobals(); ///initialize global code
    void checkDivZero(const string& reg); ///check if division by zero
    void defineLable(const string& label); ///define label   ++++
    void emitFuncRet(); ///generate return instruction   ++++
    void emitProgramStart(); ///emit program start   
    void emitProgramEnd();  /// close function   ++++
    string generateAlloca(); ///generate alloca instruction
    string generateIcmp(const string& op, const string& lhs, const string& rhs);  ///generate iCompare operation "RELOP"
    string generateLoad(int offset, const string& ptr, string expType); ///generate load instruction
    void generateJumpStatement(const string& label); ///generate break/continue statement
    void generateGlobalVar(const string& name, const string& type); ///generate global variable
    void generateStore(int offset, const string& valueReg, const string& ptr); ///generate store instruction
    void generateCondBranch(const string& condReg, const string& trueLabel, const string& falseLabel); ///generate conditional branch
    void generateUncondBranch(const string& label); ///generate unconditional branch
    //string generateBinaryInst(const string& expType, const string& lhs, const string& rhs, string op, string inst); ///generate Binop/Relop instruction
    //void generateFunctionCall(Node* terminalID); ///generate function call
    //void generateAssign(const string& lhs, const string& rhs); ///generate assign instruction
    ///phi instruction
    //void generatePhi(const string& resReg, const string& type, const vector<pair<string, string>>& labelsAndRegs);
};