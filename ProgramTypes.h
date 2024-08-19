#include <string>
#include "SymbolTable.h"
#include "GeneralFunctions.h"
#include "cg.hpp"
#include "CodeGenerator.h"
#include "hw3_output.hpp"

using std::vector;
class Expression;

bool isBool(Expression* exp);
vector<string> convertVectorToUpperCase(vector<string> toUpper);
bool LegalType(string typeOne, string typeTwo);

class Node {
    std::string value;
    std::string type;
    string reg;
public:
    Node() : value(""), type("") , reg("") {};
    Node(string value, string type) : value(value), type(type), reg("") {};
    Node(string value, string type, string reg) : value(value), type(type), reg(reg) {};
    virtual ~Node() {};
    virtual void print() = 0;
    /// GETTERS
    std::string getValue() const { return value; }
    std::string getType() const { return type; }
    string getReg() const { return reg; }   
    /// SETTERS
    void setValue(std::string value) { this->value = value; }
    void setType(std::string type) { this->type = type; }
    void setReg(std::string reg) { this->reg = reg; }
};

class Type : public Node {
public:
    Type(std::string type) : Node("",type) {};
};

class Expression : public Node {
public:
    Expression();
    Expression(string reg) : Node("", "", reg) {};
    Expression(Node* exp, bool _); //𝐸𝑥𝑝 → Not Exp
    Expression(Node* exp); //𝐸𝑥𝑝 → 𝐿𝑃𝐴𝑅𝐸𝑁 𝐸𝑥𝑝 𝑅𝑃𝐴𝑅𝐸𝑁
    Expression(Call* call); //𝐸𝑥𝑝 → 𝐶𝑎𝑙𝑙
    Expression(Node* terminalExp); //𝐸𝑥𝑝 → 𝐼𝐷
    Expression(Node* exp, string type); //𝐸𝑥𝑝 → 𝐿𝑃𝐴𝑅𝐸𝑁 𝑇𝑦𝑝𝑒 𝑅𝑃𝐴𝑅𝐸𝑁 𝐸𝑥𝑝
    Expression(Node* terminalExp, string type); //Exp->BOOL/BYTE/INT/NUM/STRING
    Expression(Node* leftExp, Node* rightExp, string op); // Exp -> Exp And / Or Exp
    ~Expression() = default;
};

class Label : public Node {
    string trueLabel;
    string falseLabel;
    string nextLabel;   
public:
    Label() : Node("",""), trueLabel(buffer.freshLabel()), 
                    falseLabel(buffer.freshLabel()), nextLabel(buffer.freshLabel()) {};
    
    string getTrueLabel() const { return trueLabel; }
    string getFalseLabel() const { return falseLabel; }
    string getNextLabel() const { return nextLabel; }
    void setTrueLabel(std::string label) { trueLabel = label; }
    void setFalseLabel(std::string label) { falseLabel = label; }
    void setNextLabel(std::string label) { nextLabel = label; }
    ~Label() = default;
};

class Call : public Node {
public:
    Call(string type, Node* terminalID); 
    ~Call() = default;
};

class Statement : public Node {
public:
    Statement() {};
    Statement(std::string value); // Statement -> BREAK / CONTINUE
    Statement(Call * call); // Statement -> Call SC
    Statement(string type,Node * id); // Statement -> Type ID SC  
    Statement(string type, Node * id, Expression * exp); // Statement -> Type ID Assign Exp SC
    Statement(Node * id, Expression * exp); // Statement -> ID Assign Exp SC
    Statement(Statments* Statments); // Statement -> { Statements }
    Statement(Expression* exp); // Statement -> IF ( Exp ) Statement
};

class Statments : public Node {
    public:
    Statments(Statement* Statement) : Node() {}; // Statements -> Statement
    Statments (Statement* Statement, Statments* Statments) : Node() {}; // Statements -> Statements Statement
    ~Statments() = default;
};

class Program : public Node {
public:
    Program() {};
    ~Program() = default;
};



