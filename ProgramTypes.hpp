#include <string>
#include <vector>
#include <iostream>
#include "SymbolTable.hpp"
#include "GeneralFunctions.hpp"

#define YYSTYPE Node*
class Expression;


void isBoolExp(Expression* exp); //to use in parser.ypp
void endingLoopMarker(); //to use in parser.ypp to mark the end of a WHILE loop


class Node {
    string reg;
    string value;
    string type;
    string trueLabel;
    string falseLabel;
public:
    Node(string val = "", string type = "" , string reg = "" ) : value(val), type(type), reg(reg) {};
    virtual ~Node() {};
    std::string getValue() const { return value; }
    std::string getType() const { return type; }
    string getReg() const { return reg; }   
    string getTrueLabel() const { return trueLabel;}
    string getFalseLabel() const { return falseLabel; }
    void setTrueLabel(std::string label) { trueLabel = label; }
    void setFalseLabel(std::string label) { falseLabel = label; }
    void setValue(std::string value) { this->value = value; }
    void setType(std::string type) { this->type = type; }
    void setReg(std::string reg) { this->reg = reg; }
};

class Type : public Node {
public:
    Type(string type) : Node( "", type, "") {};
};

class iD : public Node {
public:
    iD(Node* id) : Node(id->getValue(), "ID", "") {
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

class Expression : public Node {    
    //bool isFunction;
public:
    // Expression() = default;
    Expression(string val = "", string type = "", string reg = "" ) : Node(val, type, reg) {};
    Expression(Call* call, bool flag = true); //Exp -> Call
    // Expression(string reg, string value, string type) : Node(reg, value, type) {};
    Expression(Node* terminalExp); // Expression -> ID
    Expression(Type* type, Node* exp); // Expression -> LPAREN Type RPAREN Exp
    Expression(Node* leftExp, Node* rightExp, const string op); // Expression -> Expression Binop Expression
    ~Expression() = default;
};

class BooleanExpression : public Expression {
    string trueLabel;// target label for a jump when condition B evaluates to true
    string falseLabel; //target label for a jump when condition B evaluates to false
public:
    BooleanExpression() = default;
    //BooleanExpression(Call* call); // Exp -> Call
    BooleanExpression(Node* exp); // Exp -> LPAREN Exp RPAREN
    BooleanExpression(Node* leftExp, Node* rightExp, const string op); // Exp -> Exp RELOP/AND/OR Exp
    BooleanExpression(Node* boolexp, const string op); // Exp -> NOT Exp
    ~BooleanExpression() = default;
    // string getTrueLabel() const { return trueLabel; }
    // string getFalseLabel() const { return falseLabel; }
    string getTrueLabel() const { return trueLabel; }
    string getFalseLabel() const { return falseLabel; }
    //string getNextLabel() const { string trL = "value: " + getValue() + " ; nextLabel: " + nextLabel; return trL; }

    void setTrueLabel(std::string label) { trueLabel = label;} 
    void setFalseLabel(std::string label) { falseLabel = label; }
};

class Bool : public BooleanExpression { 
public:
    Bool(Node* exp, string trueFalse);
};

class Num : public Expression { //takeen
public:
    Num(Node* exp); // Exp -> NUM
};

class NumB : public Expression { //takeen
public:
    NumB(Node* exp); // Exp -> NUMB
};

class String : public Expression { //takeen
public:
    String(Node* exp); // Exp -> STRING
};

class Statement : public Node {
    string nextLabelS; //the label of the next code to execute after Statement
public:
    Statement(Statement* Statment); // Statement -> Statement
    Statement(Node* BCNode); // Statement -> BREAK / CONTINUE
    Statement(Call * call); // Statement -> Call SC
    Statement(const string cond, BooleanExpression* exp, string nextLabel = ""); // Statement -> IF | IF-ELSE | WHILE LP BooleanExpression RP Statment
    Statement(Node * id, Expression * exp); // Statement -> ID Assign Exp SC
    Statement(Type* type,Node * id); // Statement -> Type ID SC  
    Statement(Type* type, Node * id, Expression * exp); // Statement -> Type ID Assign Exp SC
    string afterElse(); //to use as parser.ypp
    string getNextLabel() const { return nextLabelS; }
    void setNextLabel(std::string label) { nextLabelS = label; }
};

class Statements : public Node {
    public:
    Statements(Statement* statement) : Node() { }; // Statements -> Statement
    // Statements -> Statements Statement
    Statements (Statements* statements, Statement* statement) : Node() { 
    };
    ~Statements() = default;
};

class Program : public Node { // Program -> Statements
    public:
    Program();
    ~Program() = default;
};

void generateElfStatements(BooleanExpression* boolExp, bool isElf); ///generate if/else/ statements
void emitTypesLiteral(Expression* exp, const string& type); ///emit types literal && getelementptr
