#include "ProgramTypes.h"

extern int yylineno;
extern StackTable stackTable;
using namespace std;


void isBoolExp(Expression* exp) {
    if(exp->getType() != "bool") {
        output::errorMismatch(yylineno);
        exit(0);
    }
}

//WHILE ENDING LOOP MARKER
void endingLoopMarker() {
    buffer.emit("br label %" + beginEndLabels.back().first);
    buffer.emit(beginEndLabels.back().second + ":");
    beginEndLabels.pop_back();
}

///////////////////////////////////////BooleanExpression///////////////////////////////////////

// Expression -> NOT Expression
BooleanExpression* BooleanExpression::notExpression(BooleanExpression* exp) { //takeen
    if (exp->getType() != "bool") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    if (!stackTable.isDefinedInProgram(exp->getValue())) {
        output::errorUndef(yylineno, exp->getValue());
        exit(0);
    }
    BooleanExpression* boolExp = new BooleanExpression(exp);
    if (exp->getValue() == "true") {
        boolExp->setValue("false");
    } else {
        boolExp->setValue("true");
    }
    boolExp->setType("bool");
    string tempTrueL = exp->getTrueLabel();
    boolExp->setTrueLabel(exp->getFalseLabel());
    boolExp->setFalseLabel(tempTrueL);
    return boolExp;
}

// BooleanExpression -> Exp RELOP/AND/OR Exp
BooleanExpression::BooleanExpression(Node* leftExp, Node* rightExp, const string op) {  //takeen
    BooleanExpression* left = dynamic_cast<BooleanExpression *> (leftExp);
    BooleanExpression* right = dynamic_cast<BooleanExpression *> (rightExp);
    string lType = left->getType();
    string rType = right->getType();

    if (op == "OR" || op == "AND") { //takeen
        if (lType != "bool" || rType != "bool") {
            output::errorMismatch(yylineno);
            exit(0);
        }
        setTrueLabel(right->getTrueLabel());
        setFalseLabel(right->getFalseLabel());
        setType("bool");
        if(op == "OR") {
            buffer.emit(left->getTrueLabel() + ":");
            buffer.emit("br label %" + right->getTrueLabel());
        }
        else {
            buffer.emit(left->getFalseLabel() + ":");
            buffer.emit("br label %" + right->getFalseLabel());
        }
    }
    else { //RELOP
        if (lType != "int" || lType != "byte" || rType != "int" || rType != "byte") {
            output::errorMismatch(yylineno);
            exit(0);
        }
        setType("bool");
        string reg = freshReg();
        setReg(reg);    

        setTrueLabel(buffer.freshLabel());
        setFalseLabel(buffer.freshLabel()); 
        string op = getRelopOp(op);
        string rightReg = emitTruncation(right->getReg(), lType, rType, true);
        string leftReg = emitTruncation(left->getReg(), lType, lType, true);    
        buffer.emit("br i1 " + reg + ", label %" + getTrueLabel() + ", label %" + getFalseLabel());
    }
}

// Exp -> LParen Exp RParen
BooleanExpression::BooleanExpression(Node *exp) { //takeen
    if(exp->getType() == "bool") {
        BooleanExpression* boolExp = dynamic_cast<BooleanExpression *> (exp);
        setType("bool");
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

// EXP -> Call 
BooleanExpression::BooleanExpression(Call* call) {
    setValue(call->getValue());
    setType(stackTable.findSymbol(call->getValue())->getType());
    setReg(call->getReg());
    setTrueLabel(call->getTrueLabel());
    setFalseLabel(call->getFalseLabel());
}

//////////////////////////////////////////Expression///////////////////////////////////////////

// EXP -> ID 
Expression::Expression(Node* terminalExp) {  //takeen
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
    if (this->getType() == "bool") {
        BooleanExpression* boolExp = new BooleanExpression(this);
        string newReg = freshReg();
        string newTrueL = buffer.freshLabel();
        string newFalseL = buffer.freshLabel();
        buffer.emit(newReg + " = icmp ne i32 1, " + boolExp->getReg());
        buffer.emit("br i1 " + reg + ", label %" + newTrueL + ", label %" + newFalseL);
        boolExp->setTrueLabel(newTrueL);
        boolExp->setFalseLabel(newFalseL);
    }
    if (this->getType() == "byte") {
        string newReg = emitTruncation(reg, "byte", "int", false);
        setReg(newReg);
    }
}

// Exp -> LPAREN Type RPAREN Exp 
Expression::Expression(Type* type, Node* exp) { // takeen
    Expression* e = dynamic_cast<Expression *> (exp);
    if((exp->getType() != "int" && exp->getType() != "byte") || (type->getType() != "int" && type->getType() != "byte")){
        output::errorMismatch(yylineno);
        exit(0);
    }
    setType(type->getType());
    setValue(exp->getValue());

    string reg = emitTruncation(exp->getReg(), type->getType(), exp->getType(), true);
    setReg(reg);
}

// Exp -> Exp Binop Exp
Expression::Expression(Node* leftExp, Node* rightExp, const string op) { ///takeen
    Expression* left = dynamic_cast<Expression *> (leftExp);
    Expression* right = dynamic_cast<Expression *> (rightExp);
    string lType = left->getType();
    string rType = right->getType();

    if (lType != "int" || lType != "byte" || rType != "int" || rType != "byte") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    ///SET TYPE FOR BINOP 
    this->setReg(freshReg()); 
    if(op == "DIV") { 
        buffer.emit("call void @check_division_by_zero(i32 " + right->getReg() + ")");
        if(this->getType() == "int")
            this->setType("sdiv");
        else
            this->setType("udiv");
    }
    if (lType == "byte" && rType == "byte") {  //BYTE op BYTE
        setType("byte");
        buffer.emit(this->getReg() + " = " + op + " i8 " + left->getReg() + ", " + right->getReg());        
    }
    else if (lType == "byte" && rType == "int") { //BYTE op INT
        setType("int");
        buffer.emit(this->getReg() + " = " + op + " i32 " + left->getReg() + ", " + right->getReg());      
    }
    else if (lType == "int" && rType == "byte") { //INT op BYTE
        setType("int");
        buffer.emit(this->getReg() + " = " + op + " i32 " + left->getReg() + ", " + right->getReg());
    }
    else { //INT op INT
        setType("int");
        buffer.emit(this->getReg() + " = " + op + " i32 " + left->getReg() + ", " + right->getReg());
    }
}

////////////////////////////////////////NumB////////////////////////////////////////////////////

NumB::NumB(Node* expression) : Expression() { //takeen
    setValue(expression->getValue());
    setType("byte");
    if (stoi(expression->getValue()) >= 256 || stoi(expression->getValue()) < 0) {
        output::errorByteTooLarge(yylineno, expression->getValue());
        exit(0);
    }
    buffer.emit(expression->getReg() + " = add i8" + expression->getValue() + " , 0");
}

//////////////////////////////////////////Call//////////////////////////////////////////

// Call -> ID LPAREN RPAREN 
Call::Call(Node* terminalID, Expression* exp) : Node(terminalID->getValue(), "") { //takeen
    if (!(terminalID->getValue() == "print") && !(terminalID->getValue() == "printi") && !(terminalID->getValue() == "readi")) {
        output::errorUndefFunc(yylineno, terminalID->getValue());
        exit(0);
    }
    //PRINT FUNCTION
    if ((terminalID->getValue() == "print")) {
        if (exp->getType() != "string") {
            output::errorPrototypeMismatch(yylineno,terminalID->getValue(), "string");
            exit(0);
        }
        getCallEmitLine(terminalID->getValue(), exp->getReg());
    }

    string newReg = freshReg();
    //PRINTI FUNCTION
    if (terminalID->getValue() == "printi") {
        if (exp->getType() != "byte" && exp->getType() != "int") {
            output::errorPrototypeMismatch(yylineno,terminalID->getValue(), "int");
            exit(0);
        }
        if( exp->getType() == "byte") {
            buffer.emit(newReg + " = zext i8 " + exp->getReg() + " to i32");
        }
        buffer.emit(newReg + " = add i32 " + newReg + ", 0");
        getCallEmitLine(terminalID->getValue(), newReg);
    }
    //MUST BE READI FUNCTION
    else { 
        if(exp->getType() != "byte" && exp->getType() != "int") {
            output::errorPrototypeMismatch(yylineno,terminalID->getValue(), "int");
            exit(0);
        }
        if(exp->getType() == "byte") {
            buffer.emit(newReg + " = zext i8 " + exp->getReg() + " to i32");
            buffer.emit(newReg + " = add i32 " + newReg + ", 0");
        }
        else
            buffer.emit(newReg + " = add i32 " + exp->getReg() + ", 0");

        string reg = freshReg();
        setType(stackTable.findSymbol(terminalID->getValue())->getType());
        setValue(stackTable.findSymbol(terminalID->getValue())->getName());
        getCallEmitLine(reg, newReg);
        setReg(reg);
        codeGenerator.generateStore(stackTable.findSymbol(terminalID->getValue())->getOffset(), reg, stackTable.getScope()->getBaseReg());
    }
}

//////////////////////////////////////////Statement//////////////////////////////////////////

// Statement -> BREAK / CONTINUE
Statement::Statement(Node* BCNode) : Node(BCNode->getValue(),"") { //takeen
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
}

// Statement -> Call SC
Statement::Statement(Call * call) {};

//Statement -> Type ID SC 
Statement::Statement(Type* type, Node * id) { //takeen 
    if (stackTable.isDefinedInProgram(id->getValue())) {
        output::errorDef(yylineno, id->getValue());
        exit(0);
    }
    id->setType(type->getType());
    id->setReg(freshReg());
    stackTable.addSymbolToProgram(id->getValue(), false, type->getType(), {});

    Expression* exp = new Expression(freshReg(), "0", "int");
    buffer.emit(exp->getReg() + " = add i32" + exp->getValue() + ", 0");

    string ptr_reg = freshReg();
    string rbp = stackTable.getScope()->getBaseReg();
    int offset = stackTable.getScope()->getOffset();

    //store the value in the stack
    buffer.emit(ptr_reg + " = getelementptr i32, i32* " + rbp + ", i32 " + to_string(offset));
    buffer.emit("store i32 " + exp->getReg() + ", i32* %" + exp->getReg());
}

// Statement -> Type ID Assign Exp SC
Statement::Statement(Type* type, Node * id, Expression * exp) { //maybe i need to check if the valueExp is legal func but not considered as function
    if (stackTable.isDefinedInProgram(id->getValue())) {
        output::errorDef(yylineno, id->getValue());
        exit(0);
    }
    if (!LegalType(type->getType(), exp->getType())) {
        output::errorMismatch(yylineno);
        exit(0);
    }
    if (type->getType() == "byte" && (stoi(exp->getValue()) > 255 || stoi(exp->getValue()) < 0)) {
        output::errorByteTooLarge(yylineno, exp->getValue());
        exit(0);
    }
    id->setReg(freshReg());
    Expression* useless = new Expression();
    useless->setType(type->getType());
    useless->setReg(freshReg());

    if (type->getType() == "byte") {
        string byteReg = freshReg();
        buffer.emit(byteReg + " = zext i8 " + exp->getReg() + " to i32");
        useless->setReg(byteReg);
    }
    if (type->getType() == "bool") {
        BooleanExpression* boolExp = new BooleanExpression(exp);
        string boolTrueL = boolExp->getTrueLabel();
        string boolFalseL = boolExp->getFalseLabel();
        string boolEndL = buffer.freshLabel(); 
        codeGenerator.defineLable(boolTrueL);  //emit true label
        codeGenerator.generateUncondBranch(boolEndL);

        codeGenerator.defineLable(boolFalseL); //emit false label
        codeGenerator.generateUncondBranch(boolEndL);

        codeGenerator.defineLable(boolEndL); //emit end label
        //now hanndle the fucking phi
        buffer.emit(useless->getReg() + " = phi i32 [ 1, %" + boolTrueL + " ], [ 0, %" + boolFalseL + " ]");
    }
    if ((type->getType() == "int") && (exp->getType() == "byte")) {
        string intByte = freshReg();
        buffer.emit(intByte + " = zext i8 " + exp->getReg() + " to i32");
        useless->setReg(intByte);
    }
    else { //type=int && expType = byte
        useless->setReg(exp->getReg());
    }

    stackTable.addSymbolToProgram(id->getValue(), false, type->getType(), {});
    id->setType(type->getType());
    codeGenerator.generateStore(stackTable.findSymbol(id->getValue())->getOffset(), exp->getReg(), type->getType());
    delete useless;
}

// Statement -> ID Assign Exp SC
Statement::Statement(Node * id, Expression * exp) { //maybe i need to check if the valueExp is legal func but not considered as function
 if (!stackTable.isDefinedInProgram(id->getValue())) {
        output::errorUndef(yylineno, id->getValue());
        exit(0);
    }
    if (!LegalType((stackTable.findSymbol(id->getValue()))->getType(), exp->getType())) {
        output::errorMismatch(yylineno);
        exit(0);
    }
    if (id->getType() == "byte" && (stoi(exp->getValue()) > 255 || stoi(exp->getValue()) < 0)) {
        output::errorByteTooLarge(yylineno, exp->getValue());
        exit(0);
    }
    Type* type = new Type(exp->getType());

    id->setReg(freshReg());
    Expression* useless = new Expression();
    useless->setType(type->getType());
    useless->setReg(freshReg());

    if (type->getType() == "byte") {
        string byteReg = freshReg();
        buffer.emit(byteReg + " = zext i8 " + exp->getReg() + " to i32");
        useless->setReg(byteReg);
    }
    if (type->getType() == "bool") {
        BooleanExpression* boolExp = new BooleanExpression(exp);
        string boolTrueL = boolExp->getTrueLabel();
        string boolFalseL = boolExp->getFalseLabel();
        string boolEndL = buffer.freshLabel(); 
        codeGenerator.defineLable(boolTrueL);  //emit true label
        codeGenerator.generateUncondBranch(boolEndL);

        codeGenerator.defineLable(boolFalseL); //emit false label
        codeGenerator.generateUncondBranch(boolEndL);

        codeGenerator.defineLable(boolEndL); //emit end label
        //now hanndle the fucking phi
        buffer.emit(useless->getReg() + " = phi i32 [ 1, %" + boolTrueL + " ], [ 0, %" + boolFalseL + " ]");
    }
    if ((type->getType() == "int") && (exp->getType() == "byte")) {
        string intByte = freshReg();
        buffer.emit(intByte + " = zext i8 " + exp->getReg() + " to i32");
        useless->setReg(intByte);
    }
    else { //type=int && expType = byte
        useless->setReg(exp->getReg());
    }

    stackTable.addSymbolToProgram(id->getValue(), false, type->getType(), {});
    id->setType(type->getType());
    codeGenerator.generateStore(stackTable.findSymbol(id->getValue())->getOffset(), exp->getReg(), type->getType());
    delete useless;
}


// Statement -> IF|IF-ELSE|WHILE LP EXP RP SS 
Statement::Statement(const string cond, BooleanExpression* boolexp) {
    // if (boolexp->getType() == "byte" && (stoi(boolexp->getValue()) > 255 || stoi(boolexp->getValue()) < 0)) {
    //     output::errorByteTooLarge(yylineno, boolexp->getValue());
    //     exit(0);
    // }
    if (boolexp->getType() != "bool") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    if (cond == "WHILE") {
        string loopLabel = buffer.freshLabel();
        buffer.emit("br label %" + loopLabel);
        buffer.emit(loopLabel + ":");
        beginEndLabels.push_back(pair<string, string>(loopLabel, ""));
    }
    else if (cond == "IF") {
        codeGenerator.generateUncondBranch(boolexp->getFalseLabel());
        buffer.emit(boolexp->getFalseLabel() + ":");
    }
    else { //IF-ELSE
        codeGenerator.generateUncondBranch(this->getNextLabel());
        buffer.emit(this->getNextLabel() + ":");    
    }
}

// Statement L Statement R 
Statement::Statement(Statement* Statement) {
    //open new scope
    stackTable.popScope();
    codeGenerator.emitProgramEnd();
}
