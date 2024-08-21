#include <string>
#include "SymbolTable.h"
#include "GeneralFunctions.h"
#include "cg.hpp"
#include "CodeGenerator.h"
#include "hw3_output.hpp"

using std::vector;
class Expression;

bool isBool(Expression* exp);

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

class Call : public Node {
    string trueLebel;
    string falseLabel;
    string nextLabel;
public:
    Call(string type, Node* terminalID); 
    ~Call() = default;
    string getTrueLabel() const { return trueLebel; }
    string getFalseLabel() const { return falseLabel; } 
    string getNextLabel() const { return nextLabel; }
    void setTrueLabel(std::string label) { trueLebel = label; }
    void setFalseLabel(std::string label) { falseLabel = label; }
    void setNextLabel(std::string label) { nextLabel = label; }
};

class BooleanExpression : public Node {
    string trueLabel;// target label for a jump when condition B evaluates to true
    string falseLabel; //target label for a jump when condition B evaluates to false
public:
    BooleanExpression(bool terminalBool); // BooleanExpression -> BOOL
    BooleanExpression(Node* exp); // BooleanExpression -> Exp
    BooleanExpression(Call* call); // BooleanExpression -> Call
    BooleanExpression(Node* exp, bool _); // BooleanExpression -> NOT BooleanExpression
    BooleanExpression(Node* leftExp, Node* rightExp, string op); // BooleanExpression -> Exp RELOP/AND/OR Exp
    // BooleanExpression -> ( BooleanExpression )
    BooleanExpression(BooleanExpression* exp) : Node(exp->getValue(), exp->getType(), exp->getReg()) {};
    ~BooleanExpression() = default;
    string getTrueLabel() const { return trueLabel; }
    string getFalseLabel() const { return falseLabel; }
    void setTrueLabel(std::string label) { trueLabel = label; } 
    void setFalseLabel(std::string label) { falseLabel = label; }
};

class Expression : public Node {    
public:
    Expression();
    Expression(string reg) : Node("", "", reg) {};    
    Expression(Node* terminalExp); //𝐸𝑥𝑝 → 𝐼𝐷
    Expression(Node* exp, string type); //𝐸𝑥𝑝 → 𝐿𝑃𝐴𝑅𝐸𝑁 𝑇𝑦𝑝𝑒 𝑅𝑃𝐴𝑅𝐸𝑁 𝐸𝑥𝑝
    Expression(Node* terminalExp, string type); //Exp->BOOL/BYTE/INT/NUM/STRING    *****BOOL SHOULD BE REMOVED*****

    ////////THEESE C'TORS WILL BE DELETED
    //Expression(Node* exp); //𝐸𝑥𝑝 → 𝐿𝑃𝐴𝑅𝐸𝑁 𝐸𝑥𝑝 𝑅𝑃𝐴𝑅𝐸𝑁
    Expression(Node* exp, bool _); //𝐸𝑥𝑝 → Not Exp
    Expression(Node* leftExp, Node* rightExp, string op); // Exp -> Exp And / Or Exp
    ~Expression() = default;
};


class Statement : public Node {
    string nextLabel; //the label of the next code to execute after Statement
public:
    Statement() : Node(), nextLabel(buffer.freshLabel()) {};   
    Statement(std::string value); // Statement -> BREAK / CONTINUE
    Statement(Call * call); // Statement -> Call SC
    Statement(string type,Node * id); // Statement -> Type ID SC  
    Statement(string type, Node * id, Expression * exp); // Statement -> Type ID Assign Exp SC
    Statement(Node * id, Expression * exp); // Statement -> ID Assign Exp SC
    Statement(Statments* Statments); // Statement -> { Statements }
    //Statement(Expression* exp); // Statement -> IF ( Exp ) Statement

    string getNextLabel() const { return nextLabel; }
    void setNextLabel(std::string label) { nextLabel = label; }
};

class Statments : public Node {
    public:
    Statments(Statement* statement) : Node() { /*statement->setNextLabel(buffer.freshLabel());*/ }; // Statements -> Statement
    // Statements -> Statements Statement
    Statments (Statement* statement, Statments* statments) : Node() { 
        //statement->setNextLabel(buffer.freshLabel());
    }; 
    ~Statments() = default;
};

class Program : public Node {
public:
    Program() {};
    ~Program() = default;
};

