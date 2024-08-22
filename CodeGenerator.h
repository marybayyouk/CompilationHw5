#include "SymbolTable.h"
#include "GeneralFunctions.h"
//#include "cg.hpp"
#include <string>
#include <vector>

extern CodeBuffer buffer;
using namespace std;

using std::vector;
static vector<pair<string, string>> beginEndLabels;

class CodeGenerator{
//private:
public:
    CodeGenerator() = default;
    ~CodeGenerator() = default;
    void checkDivZero(const string& reg); ///check if division by zero
    void defineLable(const string& label); ///define label
    void emitFuncRet(); ///generate return instruction
    void emitProgramStart(); ///emit program start
    void emitProgramEnd();  /// close function
    void emitTypesLiteral(Expression* exp, const string& type); ///emit types literal && getelementptr
    string generateAlloca(); ///generate alloca instruction
    string generateIcmp(const string& op, const string& lhs, const string& rhs);  ///generate iCompare operation "RELOP"
    string generateLoad(int offset, const string& ptr, string expType); ///generate load instruction
    void generateJumpStatement(const string& label); ///generate break/continue statement
    void generateGlobalVar(const string& name, const string& type); ///generate global variable
    void generateStore(int offset, const string& valueReg, const string& ptr); ///generate store instruction
    void generateBinaryInst(const string& expType, const string& lhs, const string& rhs, string op, string inst); ///generate Binop/Relop instruction
    void generateUnaryInst(const string& expType, const string& reg, string op); ///generate unary instruction
    void generateCondBranch(const string& condReg, const string& trueLabel, const string& falseLabel); ///generate conditional branch
    void generateUncondBranch(const string& label); ///generate unconditional branch
    void generateFunctionCall(Node* terminalID); ///generate function call
    void generateElfStatements(BooleanExpression* boolExp, bool isElf); ///generate if/else/ statements

    //void generateAssign(const string& lhs, const string& rhs); ///generate assign instruction
    ///phi instruction
    void generatePhi(const string& resReg, const string& type, const vector<pair<string, string>>& labelsAndRegs);

};