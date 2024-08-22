#include <string>
#include <vector>
#include <iostream>
#include "SymbolTable.h"
#include "GeneralFunctions.h"
#include "cg.hpp"
#include "CodeGenerator.h"
#include "hw3_output.hpp"

#define YYSTYPE Node*
extern CodeGenerator codeGenerator;
class Expression;


void isBoolExp(Expression* exp); //to use in parser.ypp
void endingLoopMarker(); //to use in parser.ypp to mark the end of a WHILE loop


class Node {
    std::string value;
    std::string type;
    string reg;
public:
    Node(string val = "", string type = "", string reg = "") : value(val), type(type), reg(reg) {};
    virtual ~Node() {};
    std::string getValue() const { return value; }
    std::string getType() const { return type; }
    string getReg() const { return reg; }   
    void setValue(std::string value) { this->value = value; }
    void setType(std::string type) { this->type = type; }
    void setReg(std::string reg) { this->reg = reg; }
};

class Type : public Node {
public:
    Type(string type) : Node("",type) {};
};

class iD : public Node {
public:
    iD(Node* id) : Node(id->getValue(), "ID") {
    }
};

class Call : public Node {
    string trueLebel;
    string falseLabel;
    string nextLabel;
public:
    Call(Node* terminalID, Expression* exp);
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
    //BooleanExpression(bool terminalBool); // Exp -> True/False
    //BooleanExpression(BooleanExpression* exp) : Node(exp->getValue(), exp->getType(), exp->getReg()) {};
    BooleanExpression(Call* call); // Exp -> Call
    BooleanExpression(Node* exp); // Exp -> LPAREN Exp RPAREN
    BooleanExpression(Node* leftExp, Node* rightExp, const string op); // Exp -> Exp RELOP/AND/OR Exp
    ~BooleanExpression() = default;
    BooleanExpression* notExpression(BooleanExpression* exp); // Exp->NOT Exp
    string getTrueLabel() const { return trueLabel; }
    string getFalseLabel() const { return falseLabel; }
    void setTrueLabel(std::string label) { trueLabel = label; } 
    void setFalseLabel(std::string label) { falseLabel = label; }
};

class Expression : public Node {    
    bool isFunction;
public:
    Expression(string reg) : Node("", "", reg) {};    
    Expression(Node* terminalExp); // Expression -> ID
    Expression(Type* type, Node* exp); // Expression -> LPAREN Type RPAREN Exp
    Expression(string value, string type, bool isFunc=false); //Expression->(SON'S C'TOR) BOOL/BYTE/INT/NUM/STRING    *****BOOL SHOULD BE REMOVED*****
    Expression(Node* leftExp, Node* rightExp, const string op); // Expression -> Expression Binop Expression
    ~Expression() = default;
    bool isFunc() const { return isFunction; }
    void setIsFunc(bool isFunction) { this->isFunction = isFunction; }
};

class Bool : public Expression {
public:
    Bool(string value) : Expression(value, "BOOL") {};
    Bool(Node* exp) : Expression(exp->getValue(), "BOOL") {};
};

class Num : public Expression {
public:
    Num(Node* exp) : Expression(exp->getValue(), "INT") {};
};

class NumB : public Expression {
public:
    NumB(Node* exp);
};

class String : public Expression {
public:
    String(Node* exp) : Expression(exp->getValue(), "STRING") {};
};


class Statement : public Node {
    string nextLabel; //the label of the next code to execute after Statement
public:
    //Statement() : Node(), nextLabel(buffer.freshLabel()) {};  
    Statement(Statement* Statment) {}; // Statement -> Statement
    Statement(Node* BCNode); // Statement -> BREAK / CONTINUE
    Statement(Call * call); // Statement -> Call SC
    Statement(const string cond, BooleanExpression* exp); // Statement -> IF | IF-ELSE | WHILE LP BooleanExpression RP Statment
    Statement(Node * id, Expression * exp); // Statement -> ID Assign Exp SC
    Statement(Type* type,Node * id); // Statement -> Type ID SC  
    Statement(Type* type, Node * id, Expression * exp); // Statement -> Type ID Assign Exp SC
    string getNextLabel() const { return nextLabel; }
    void setNextLabel(std::string label) { nextLabel = label; }
};

class Statements : public Node {
    string nextLabel; //the label of the next code to execute after Statments
    public:
    Statements(Statement* statement) : Node() { statement->setNextLabel(buffer.freshLabel()); }; // Statements -> Statement
    // Statements -> Statements Statement
    Statements (Statements* statements, Statement* statement) : Node() { 
        statement->setNextLabel(buffer.freshLabel());
        statements->setNextLabel(statement->getNextLabel());
    }; 
    ~Statements() = default;
    string getNextLabel() const { return nextLabel; }
    void setNextLabel(std::string label) { nextLabel = label; }
};

class Program : public Node {
public:
    Program() {
        codeGenerator.emitProgramStart();
    };
    ~Program() = default;
};

