#include "SymbolTable.h"
#include "GeneralFunctions.h"
//#include "cg.hpp"
#include <string>
#include <vector>

extern CodeBuffer buffer;
using namespace std;

class CodeGenerator{
//private:
public:
    CodeGenerator() = default;
    ~CodeGenerator() = default;
    void defineLable(const string& label); ///define label
    void emitBranchToLabel(const string& label); ///emit branch to label
    void emitProgramStart(); ///emit program start
    void emitTypesLiteral(Expression* exp, const string& type); ///emit types literal && getelementptr
    void checkDivZero(const string& reg); ///check if division by zero
    void generateGlobalVar(const string& name, const string& type); ///generate global variable
    string generateLoad(int offset, const string& ptr, string expType); ///generate load instruction
    void generateStore(int offset, const string& valueReg, const string& ptr); ///generate store instruction
    string generateIcmp(const string& op, const string& lhs, const string& rhs);  ///generate iCompare operation "RELOP"
    bool generateBoolOperator(const string& lhs, const string& rhs, string op); ///generate boolean operation "AND/OR"
    void generateBinaryInst(const string& expType, const string& lhs, const string& rhs, string op, string inst); ///generate Binop/Relop instruction
    void generateUnaryInst(const string& expType, const string& reg, string op); ///generate unary instruction
    void generateCondBranch(const string& condReg, const string& trueLabel, const string& falseLabel); ///generate conditional branch
    void generateUncondBranch(const string& label); ///generate unconditional branch
    void generateFunctionCall(Node* terminalID); ///generate function call
    void funcRet(); ///generate return instruction
    //void generateAssign(const string& lhs, const string& rhs); ///generate assign instruction
    void  mainRet(); ///close function
    string generateAlloca(); ///generate alloca instruction
    ///phi instruction
    void generatePhi(const string& resReg, const string& type, const vector<pair<string, string>>& labelsAndRegs);
    void generateJumpStatement(const string& label); ///generate break/continue statement
};