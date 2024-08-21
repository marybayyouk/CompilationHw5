#include "ProgramTypes.h"

extern int yylineno;
extern StackTable stackTable;
extern CodeGenerator codeGenerator;
using namespace std;


bool isBool(Expression* exp) {
    if(exp->getType() != "BOOL") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    return true;
}

///////////////////////////////////////BooleanExpression///////////////////////////////////////

// BooleanExpression -> Exp RELOP/AND/OR Exp
BooleanExpression::BooleanExpression(Node* leftExp, Node* rightExp, const string op) {
    BooleanExpression* left = dynamic_cast<BooleanExpression *> (leftExp);
    BooleanExpression* right = dynamic_cast<BooleanExpression *> (rightExp);
    string lType = left->getType();
    string rType = right->getType();

    if (op == "OR" || op == "AND") {
        if (lType != "BOOL" || rType != "BOOL") {
        output::errorMismatch(yylineno);
        exit(0);
        }
        setType("BOOL");
        setTrueLabel(left->getTrueLabel());
        setFalseLabel(right->getFalseLabel());
        codeGenerator.generateBinaryInst(this->getType(), left->getValue(), right->getValue(), op, "BINOP");
    } 
    else { //RELOP
        if (lType != "INT" || lType != "BYTE" || rType != "INT" || rType != "BYTE") {
        output::errorMismatch(yylineno);
        exit(0);
        }
        setType("BOOL");
        setTrueLabel(allocateLable("true")); ///////need to change this
        setFalseLabel(allocateLable("false"));  ///////need to change this
        codeGenerator.generateBinaryInst(this->getType(), left->getValue(), right->getValue(), op, "RELOP");
    }
}

// Exp -> NOT Exp
BooleanExpression::BooleanExpression(Node* exp) : Node(exp->getValue(), "") {
    if (!stackTable.isDefinedInProgram(exp->getValue())) {
        output::errorUndef(yylineno, exp->getValue());
    }
    if (exp->getType() != "BOOL") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    setType("BOOL");
}

// Exp -> LParen Exp RParen
BooleanExpression::BooleanExpression(Node *exp) {
    if(exp->getType() == "BOOL") {
        BooleanExpression* boolExp = dynamic_cast<BooleanExpression *> (exp);
        setType("BOOL");
        setValue(boolExp->getValue());
        setTrueLabel(boolExp->getTrueLabel());
        setFalseLabel(boolExp->getFalseLabel());
        setReg(boolExp->getReg());
    }
    else {
        // initialize the expression with the value of the expression - regular expression -
        setValue(exp->getValue());
        setType("");
        setReg(exp->getReg());
    }
}

// EXP -> Call --------GENERATION IS DONE NOTHING TO DO--------
BooleanExpression::BooleanExpression(Call* call) {
    setValue(call->getValue());
    setType(stackTable.findSymbol(call->getValue())->getType());
    setReg(call->getReg());
    setTrueLabel(call->getTrueLabel());
    setFalseLabel(call->getFalseLabel());
}


//////////////////////////////////////////Expression///////////////////////////////////////////

// EXP -> ID --------GENERATION IS DONE--------
Expression::Expression(Node* terminalExp) {
    if (!stackTable.isDefinedInProgram(terminalExp->getValue())){
        output::errorUndef(yylineno, terminalExp->getValue());
        exit(0);
    }
    setValue(terminalExp->getValue());
    setType(stackTable.findSymbol(terminalExp->getValue())->getType());

    int offset = stackTable.findSymbol(terminalExp->getValue())->getOffset();
    string ptr = freshReg();
    string reg = codeGenerator.generateLoad(offset, ptr, terminalExp->getType());
    setReg(reg); 
}

// Exp -> LPAREN Type RPAREN Exp --------GENERATION IS DONE--------
Expression::Expression(Type* type, Expression* exp) {
    if((exp->getType() != "INT" && exp->getType() != "BYTE") || (type->getType() != "INT" && type->getType() != "BYTE")){
        output::errorMismatch(yylineno);
        exit(0);
    }
    setType(type->getType());
    setValue(exp->getValue());

    string reg = emitTruncation(exp->getReg(), type->getType(), exp->getType(), true);
    setReg(reg);
}

// Exp->BYTE/INT/NUM/STRING --------GENERATION IS DONE--------
Expression::Expression(string value, string type, bool isFunc=false) : Node(value, type) {
    if((type == "BYTE") && (stoi(value) > 255)){
        output::errorByteTooLarge(yylineno, value);
    }
    codeGenerator.emitTypesLiteral(this, type);
}

// Exp -> Exp Binop Exp
Expression::Expression(Node* leftExp, Node* rightExp, const string op) {
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
}


////////////////////////////////////////NumB////////////////////////////////////////////////////

NumB::NumB(Node* expression) : Expression(expression->getValue(), "BYTE") {
    if (stoi(expression->getValue()) >= 256) {
        output::errorByteTooLarge(yylineno, expression->getValue());
        exit(0);
    }
}


//////////////////////////////////////////Call//////////////////////////////////////////

// Call -> ID LPAREN RPAREN ----GENERATION IS DONE----
Call::Call(Node* terminalID, Expression* exp) : Node(terminalID->getValue(), "") {
    if (!(terminalID->getValue() == "print") && !(terminalID->getValue() == "printi") && !(terminalID->getValue() == "readi")) {
        output::errorUndefFunc(yylineno, terminalID->getValue());
        exit(0);
    }
    if ((terminalID->getValue() == "print")) {
        if (exp->getType() != "STRING") {
            output::errorPrototypeMismatch(yylineno,terminalID->getValue());
            exit(0);
        }
    }
    else if (terminalID->getValue() == "printi") {
        if (exp->getType() != "BYTE" && exp->getType() != "INT") {
            output::errorPrototypeMismatch(yylineno,terminalID->getValue());
            exit(0);
        }
    }
    else { //MUST BE READI FUNCTION
        if(exp->getType() != "BYTE" && exp->getType() != "INT") {
            output::errorPrototypeMismatch(yylineno,terminalID->getValue());
            exit(0);
        }
    }
    //////I THINK WE SHOULF CHECK MORE CASES FOR EACH 3 POSSIBLE FUNCTION
    setType(stackTable.findSymbol(terminalID->getValue())->getType());
    setValue(stackTable.findSymbol(terminalID->getValue())->getName());
    codeGenerator.generateFunctionCall(terminalID);
    //**************************NEED TO ADD LABELS HERE**************************
}


//////////////////////////////////////////Statement//////////////////////////////////////////

// Statement -> BREAK / CONTINUE
Statement::Statement(Node* BCNode) : Node(BCNode->getValue(),"") {
    bool loop=false;
    for(SymbolTable* sym: stackTable.scopes){ 
        if (sym->getIsLoop()){
            loop = true;
        }
    }
    if (BCNode->getValue() == "break") {
        if (!loop) {
            output::errorUnexpectedBreak(yylineno);
            exit(0);
        }
        codeGenerator.generateJumpStatement("BREAK");
    } 
    else if (BCNode->getValue() == "continue") {
        if (!loop) {
            output::errorUnexpectedContinue(yylineno);
            exit(0);
        }
        codeGenerator.generateJumpStatement("CONTINUE");
    }
    //**************************NEED TO ADD LABELS HERE**************************
}

// Statement -> Call SC
Statement::Statement(Call * call) {};

//Statement -> Type ID SC 
Statement::Statement(Type* type, Node * id) {
    if (stackTable.isDefinedInProgram(id->getValue())) {
        output::errorDef(yylineno, id->getValue());
        exit(0);
    }
    id->setType(type->getType());
    stackTable.addSymbolToProgram(id->getValue(), false, type->getType(), {});
}

// Statement -> Type ID Assign Exp SC
Statement::Statement(Type* type, Node * id, Expression * exp) {
    if (stackTable.isDefinedInProgram(id->getValue())) {
        output::errorDef(yylineno, id->getValue());
        exit(0);
    }
    if (!isLegalFunc(exp->getValue(), exp->isFunc())) {
        output::errorUndef(yylineno, exp->getValue());
        exit(0);
    }   
    if (!LegalType(type->getType(), exp->getType())) {
        output::errorMismatch(yylineno);
        exit(0);
    }
    stackTable.addSymbolToProgram(id->getValue(), false, type->getType(), {});
    id->setType(type->getType());
}

// Statement -> ID Assign Exp SC
Statement::Statement(Node * id, Expression * exp) {
 if (!stackTable.isDefinedInProgram(id->getValue())) {
        output::errorUndef(yylineno, id->getValue());
        exit(0);
    }
    if (!LegalType((stackTable.findSymbol(id->getValue()))->getType(), exp->getType())) {
        output::errorMismatch(yylineno);
        exit(0);
    }
    string expV = exp->getValue();
    if (!isLegalFunc(exp->getValue(), exp->isFunc())) {
        output::errorUndef(yylineno, exp->getValue());
        exit(0);
    }
}

// Statement -> IF|IF-ELSE|WHILE LP EXP RP SS -----Tested-----
Statement::Statement(const string cond,BooleanExpression* boolexp) {
    if (boolexp->getType() == "BYTE" && (stoi(boolexp->getValue()) > 255)) {
        output::errorByteTooLarge(yylineno, boolexp->getValue());
        exit(0);
    }
    if (boolexp->getType() != "BOOL") {
            //cout << "here" << endl;
            output::errorMismatch(yylineno);
            exit(0);
        }
}

// Statement L Statement R 
Statement::Statement(Statement* Statement) {
    //open new scope
    stackTable.pushScope(false, "");
}

