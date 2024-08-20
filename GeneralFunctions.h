#include <string>
#include "Cg.hpp"

using namespace std;
extern CodeBuffer buffer;
extern int yylineno;

string getBinopOp(string op); ///get binary operation
string getRelopOp(string op); ///get relational operation
string freshReg(); ///allocate new register
string freshGlobalReg(); ///allocate new global register
string allocateLable(const string& prefix); ///allocate new label
string getCallEmitLine(string funcName, string reg); ///handle call
string emitTruncation(const string& reg, const string& type1, const string& type2, bool is2Types); ///handle truncation

