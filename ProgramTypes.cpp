#include "ProgramTypes.h"

extern int yylineno;
extern StackTable scopes;
extern CodeGenerator codeGenerator;
using namespace std;


bool isBool(Expression* exp) {
    if(exp->getType() != "BOOL") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    return true;
}

vector<string> convertVectorToUpperCase(vector<string>& toUpper) {
    vector<string> toRet;
    for (string name : toUpper) {
        toRet.push_back(upperCase(name));
    }
    return toRet;
}

bool LegalType(string typeOne, string typeTwo) {
    if (typeOne == "INT" && typeTwo == "BYTE") {
        return true;
    } else if (typeOne == typeTwo) {
        return true;
    }
    // need to check 3rd legal assignment (byte) (int) with casting
    return false;
}


///////////////////////////////////////BooleanExpression///////////////////////////////////////

// BooleanExpression -> Exp RELOP/AND/OR Exp
BooleanExpression::BooleanExpression(Node* leftExp, Node* rightExp, string op) {
    BooleanExpression* left = dynamic_cast<BooleanExpression *> (leftExp);
    BooleanExpression* right = dynamic_cast<BooleanExpression *> (rightExp);

    if (left->getType() != "BOOL" || right->getType() != "BOOL") {
        output::errorMismatch(yylineno);
        exit(0);
    }

    if (op == "OR") {
        setType("BOOL");
        setTrueLabel(left->getTrueLabel());
        setFalseLabel(right->getFalseLabel());
        buffer.emit(getValue() + " = or i1 " + left->getValue() + ", " + right->getValue());
    } 
    else if(op == "AND") {
        setType("BOOL");
        setTrueLabel(right->getTrueLabel());
        setFalseLabel(left->getFalseLabel());
        buffer.emit(getValue() + " = and i1 " + left->getValue() + ", " + right->getValue());
    } 
    else if (op == "GT" || op == "GE" || op == "LT" || op == "LE" || op == "EQ" || op == "NE") {
        setType("BOOL");
        setTrueLabel(allocateLable("true"));
        setFalseLabel(allocateLable("false"));
        buffer.emit(getValue() + " = icmp " + getRelopOp(op) + " i32 " + left->getValue() + ", " + right->getValue());
        buffer.emit("br i1 " + getValue() + ", label %" + getTrueLabel() + ", label %" + getFalseLabel());
    }
    
    else {
        output::errorMismatch(yylineno);
        exit(0);
    }
}


//////////////////////////////////////////Expression///////////////////////////////////////////
Expression::Expression() : Node("","VOID") {};

// 𝐸𝑥𝑝 → 𝑁𝑜𝑡 𝐸𝑥𝑝
Expression::Expression(Node* exp, bool _) : Node(exp->getValue(), "") {
    Expression* expression = dynamic_cast<Expression *> (exp);
    if (expression->getType() != "BOOL") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    setType("BOOL");
}

// 𝐸𝑥𝑝 → 𝐿𝑃𝐴𝑅𝐸𝑁 𝐸𝑥𝑝 𝑅𝑃𝐴𝑅𝐸𝑁
Expression::Expression(Node *exp) : Node(exp->getValue(), "") {};

// EXP -> Call --------GENERATION IS DONE NOTHING TO DO--------
BooleanExpression::BooleanExpression(Call* call) {
    setValue(call->getValue());
    setType(scopes.findSymbol(call->getValue())->getType());
    setReg(call->getReg());
    setTrueLabel(call->getTrueLabel());
    setFalseLabel(call->getFalseLabel());
    setNextLabel(call->getNextLabel());
}

// EXP -> ID --------GENERATION IS DONE--------
Expression::Expression(Node* terminalExp) {
    if (!scopes.isDefinedInProgram(terminalExp->getValue())){
        output::errorUndef(yylineno, terminalExp->getValue());
        exit(0);
    }
    setValue(terminalExp->getValue());
    setType(scopes.findSymbol(terminalExp->getValue())->getType());

    int offset = scopes.findSymbol(terminalExp->getValue())->getOffset();
    string ptr = freshReg();
    string reg = codeGenerator.generateLoad(offset, ptr, terminalExp->getType());
    setReg(reg); 
}

// Exp -> LPAREN Exp RPAREN --------GENERATION IS DONE--------
Expression::Expression(Node* toExp, string type) {
    Expression* exp = dynamic_cast<Expression *> (toExp);
    if(!LegalType(exp->getType(), type)){
        output::errorMismatch(yylineno);
        exit(0);
    }
    setType(type);
    setValue(exp->getValue());

    string reg = emitTruncation(exp->getReg(), type, exp->getType(), true);
    setReg(reg);
}

// Exp->BOOL/BYTE/INT/NUM/STRING --------GENERATION IS DONE--------
Expression::Expression(Node* terminalExp, string type) : Node(terminalExp->getValue(), type) {
    if((type == "BYTE") && (stoi(terminalExp->getValue()) > 255)){
        output::errorByteTooLarge(yylineno, terminalExp->getValue());
        exit(0);
    }
    codeGenerator.emitTypesLiteral(this, type);
}


////////*********************neeed to delete this and use BooleanExpression*********************////////
// Exp -> Exp And / Or Exp   
Expression::Expression(Node* leftExp, Node* rightExp, string op) {
    Expression* left = dynamic_cast<Expression *> (leftExp);
    Expression* right = dynamic_cast<Expression *> (rightExp);
    if (left->getType() != "BOOL" || right->getType() != "BOOL") {
        output::errorMismatch(yylineno);
        exit(0);
    } 
    if (op == "AND" || op == "OR") {
        setType("BOOL");
        codeGenerator.generateBinaryInst(this->getType(), left->getValue(), right->getValue(), op, "BINOP");
    } else {
        output::errorMismatch(yylineno);
        exit(0);
    }
}

////////*********************neeed to delete the *RELOP* Part BooleanExpression*********************////////
// Exp -> Exp Relop/Binop Exp
Expression::Expression(Node* leftExp, Node* rightExp, string op) {
    Expression* left = dynamic_cast<Expression *> (leftExp);
    Expression* right = dynamic_cast<Expression *> (rightExp);
    string lType = left->getType();
    string lValue = left->getValue();
    string rType = right->getType();
    string rValue = right->getValue();

    if (lType != "INT" || lType != "BYTE" || rType != "INT" || rType != "BYTE") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    ///SET TYPE FOR BINOP 
    if (op == "ADD" || op == "SUB" || op == "MUL" || op == "DIV") {
        if (lType == "BYTE" && rType == "BYTE") {
            setType("BYTE");
        } else {
            setType("INT");
        }
        ///SOMETHING IS WRONG HERE - NEED TO REVIEW
        codeGenerator.generateBinaryInst(this->getType(), lValue, rValue, op, "BINOP");
    }
    ///SET TYPE FOR RELOP
    if (op == "GT" || op == "GE" || op == "LT" || op == "LE" || op == "EQ" || op == "NE") {
        setType("BOOL");
        codeGenerator.generateBinaryInst(this->getType(), lValue, rValue, op, "RELOP");
    }
}

//////////////////////////////////////////Call//////////////////////////////////////////
// Call -> ID LPAREN RPAREN ----GENERATION IS DONE----
Call::Call(string type, Node* terminalID) : Node(terminalID->getValue(), "") {
    if (!scopes.isDefinedInProgram(terminalID->getValue())) {
        output::errorUndefFunc(yylineno, terminalID->getValue());
        exit(0);
    }
    if (!scopes.findSymbol(terminalID->getValue())->getIsFunction()) {
        output::errorUndefFunc(yylineno, terminalID->getValue());
        exit(0);
    }
    if(scopes.findSymbol(terminalID->getValue())->getNameType().names.size() > 0) {
        output::errorPrototypeMismatch(yylineno, terminalID->getValue());
        exit(0);
    }
    //////I THINK WE SHOULF CHECK MORE CASES FOR EACH 3 POSSIBLE FUNCTION
    setType(scopes.findSymbol(terminalID->getValue())->getType());
    setValue(scopes.findSymbol(terminalID->getValue())->getName());
    codeGenerator.generateFunctionCall(terminalID);
    //**************************NEED TO ADD LABELS HERE**************************
}

//////////////////////////////////////////Statement//////////////////////////////////////////
// Statement -> BREAK / CONTINUE
Statement::Statement(std::string value) : Node(value,"") {
    if (value == "BREAK") {
        if (!scopes.getScope()->getIsLoop()) {
            output::errorUnexpectedBreak(yylineno);
            exit(0);
        }
        codeGenerator.generateJumpStatement("BREAK");
    } 
    else if (value == "CONTINUE") {
        if (!scopes.getScope()->getIsLoop()) {
            output::errorUnexpectedContinue(yylineno);
            exit(0);
        }
        codeGenerator.generateJumpStatement("CONTINUE");
    }
    //**************************NEED TO ADD LABELS HERE**************************
}

// Statement -> Call SC
Statement::Statement(Call * call) : Node() {
    if (!scopes.isDefinedInProgram(call->getValue()) || !scopes.findSymbol(call->getValue())->getIsFunction()){
        output::errorUndefFunc(yylineno, call->getValue());
        exit(0);
    } 
}

//Statement -> Type ID SC 
Statement::Statement(string type, Node * id) {
    if (scopes.isDefinedInProgram(id->getValue())) {
        output::errorDef(yylineno, id->getValue());
        exit(0);
    }
    scopes.addSymbolToProgram(id->getValue(), false, type, {});
    setValue(type);
}

// Statement -> Type ID Assign Exp SC
Statement::Statement(string type, Node * id, Expression * exp) {
    if (scopes.isDefinedInProgram(id->getValue())) {
        output::errorDef(yylineno, id->getValue());
    }
    if (!LegalType(type, exp->getValue())) {
        output::errorMismatch(yylineno);
        exit(0);
    }
    setValue(type);
    scopes.addSymbolToProgram(id->getValue(), false, type, {});
}

// Statement -> ID Assign Exp SC
Statement::Statement(Node * id, Expression * exp) {
    if (!scopes.isDefinedInProgram(id->getValue())) {
        output::errorUndef(yylineno, id->getValue());
        exit(0);
    }
    if (!LegalType(scopes.findSymbol(id->getValue())->getType(), exp->getType())) {
        output::errorMismatch(yylineno);
        exit(0);
    }
    setValue(exp->getType());
}

// Statement L Statement R 
Statement::Statement(Statments* Statments) {
    //open new scope
    scopes.pushScope(false, "");
}

