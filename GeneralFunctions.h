#include <string>

using namespace std;

string getBinopOp(string op); ///get binary operation
string getRelopOp(string op); ///get relational operation
string freshReg(); ///allocate new register
string freshGlobalReg(); ///allocate new global register
string allocateLable(const string& prefix); ///allocate new label
