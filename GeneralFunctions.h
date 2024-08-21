#include <string>
#include "Cg.hpp"

using namespace std;
extern CodeBuffer buffer;
extern int yylineno;


string upperCase(string str);
vector<string> convertVectorToUpperCase(vector<string> toUpper);
bool isLegalFunc(string func, bool functionType);
bool LegalType(string typeOne, string typeTwo);
string getBinopOp(string op); ///get binary operation
string getRelopOp(string op); ///get relational operation
string freshReg(); ///allocate new register
string freshGlobalReg(); ///allocate new global register
string allocateLable(const string& prefix); ///allocate new label
string getCallEmitLine(string funcName, string reg); ///handle call
string emitTruncation(const string& reg, const string& type1, const string& type2, bool is2Types); ///handle truncation

