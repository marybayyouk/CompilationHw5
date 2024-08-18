#include "ProgramTypes.h"
#include "SymbolTable.h"
#include "cg.hpp"
#include <string>
#include <vector>

using namespace std;

string freshReg();
string freshGlobalReg();
string freshLabel(const string& prefix);

class CodeGenerator{
//private:

public:
    CodeGenerator() = default;
    ~CodeGenerator() = default;
    string generateLoad(const string& ptr);
    void generateStore(const string& valueReg, const string& ptr);
    string generateArithmeticOperation(const string& lhs, const string& rhs);
    string generateIcmp(const string& cond, const string& lhs, const string& rhs);
    void generateCondBranch(const string& condReg, const string& trueLabel, const string& falseLabel);
    void generateUncondBranch(const string& label);
    void generateFunctionCall(Node* node, CodeBuffer& buffe);
    void generateReturn(const string& valueReg);
    string generateAlloca();
    string generateGEP(const string& ptr, const vector<string>& indices);

};