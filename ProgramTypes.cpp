#include "ProgramTypes.hpp"
#include "CodeGenerator.hpp"

extern int yylineno;
extern StackTable stackTable;
extern CodeBuffer buffer;
extern CodeGenerator codeGenerator;
using namespace std;


void generateElfStatements(BooleanExpression* exp, bool isElf) {
    if (isElf) 
        buffer.emit("br label %" + exp->getTrueLabel() + "\n" + exp->getTrueLabel() + ":");
    else
        buffer.emit("br label %" + exp->getFalseLabel() + "\n" + exp->getFalseLabel() + ":");           
}

void emitTypesLiteral(Expression* exp, const string& type) {
    if (type == "byte") ///NUMB
        buffer.emit(exp->getReg() + " = add i18 " + exp->getValue() + ", 0");
    else if (type == "bool") 
        buffer.emit(exp->getReg() + " = add i1 " + exp->getValue() + ", 0");
    else if (type == "int") ///NUM
        buffer.emit(exp->getReg() + " = add i32 " + exp->getValue() + ", 0"); 
    else if (type == "string") {
        string globalReg = freshGlobalReg();
        string strReg = freshReg();
        ///MAYBE NEXT LINE SHOULD BE -1 AND NOT +1
        buffer.emit(globalReg + " = constant [" + to_string(exp->getValue().size() + 1) + " x i8]" + " c" + exp->getValue() + "\\00\"");
        buffer.emit(strReg + " = getelementptr[" + to_string(exp->getValue().size() + 1) + " x i8]" 
                        + ", " + to_string(exp->getValue().size() + 1) + " x i8]*  " + globalReg + ", i32 0, i32 0");
        exp->setReg(strReg);
    }
}

void isBoolExp(Expression* exp) {
    if(exp->getType() != "bool") {
        output::errorMismatch(yylineno);
        exit(0);
    }
}

//WHILE ENDING LOOP MARKER
void endingLoopMarker() {
    stackTable.popScope();
}
void beginingLoopMarker(string beginLabel) {
    codeGenerator.generateUncondBranch(beginLabel);
    codeGenerator.defineLable(beginLabel);
}


///////////////////////////////////////BooleanExpression///////////////////////////////////////

// Expression -> NOT Expression //regFixed
BooleanExpression::BooleanExpression(Node* boolExp, const string op) { 
    setReg(boolExp->getReg());
    if (boolExp->getType() != "bool") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    if (!stackTable.isDefinedInProgram(boolExp->getValue())) {
        output::errorUndef(yylineno, boolExp->getValue());
        exit(0);
    }
    string temp = boolExp->getFalseLabel();
    boolExp->setFalseLabel(boolExp->getTrueLabel());
    boolExp->setTrueLabel(temp);
}

// BooleanExpression -> Exp RELOP/AND/OR Exp //regFixed
BooleanExpression::BooleanExpression(Node* leftExp, Node* rightExp, const string op) { 
    string lType = leftExp->getType();
    string rType = rightExp->getType();

    if (op == "OR" || op == "AND") {  
        if (lType != "bool" || rType != "bool") {
            output::errorMismatch(yylineno);
            exit(0);
        }
        setTrueLabel(rightExp->getTrueLabel());
        setFalseLabel(rightExp->getFalseLabel());
        setType("bool");
        // setTrueLabel(leftExp->getTrueLabel());

        if(op == "OR") {
            codeGenerator.defineLable(leftExp->getTrueLabel()); //before i defined Lexp->true
            codeGenerator.generateUncondBranch(rightExp->getTrueLabel()); //rightExp->true
        }
        else {
            codeGenerator.defineLable(leftExp->getFalseLabel());
            codeGenerator.generateUncondBranch(rightExp->getFalseLabel());
        }
    }
    else { //RELOP
        if ((lType != "int" && lType != "byte") || (rType != "int" && rType != "byte")) {
            output::errorMismatch(yylineno);
            exit(0);
        }
        setType("bool");
        string reg = freshReg();
        setReg(reg);    

        setTrueLabel(buffer.freshLabel());
        setFalseLabel(buffer.freshLabel()); 
        string oper = getRelopOp(op);
        string rightReg = emitTruncation(rightExp->getReg(), lType, rType, true);
        string leftReg = emitTruncation(leftExp->getReg(), lType, lType, true);    
        codeGenerator.generateCondBranch(reg, getTrueLabel(), getFalseLabel());
    }
}

// Exp -> LParen Exp RParen //regFixed
BooleanExpression::BooleanExpression(Node* exp) {  
    if(exp->getType() == "bool") {
        setReg(exp->getReg());
        setType("bool");
        setValue(exp->getValue());
        setTrueLabel(exp->getTrueLabel());
        setFalseLabel(exp->getFalseLabel());
    } else {
        // initialize the expression with the value of the expression - regular expression -
        setValue(exp->getValue());
        setType(exp->getType());
        setReg(exp->getReg());
    }
}

// EXP -> Call //not sure
BooleanExpression::BooleanExpression(Call* call) {
    setValue(call->getValue());
    setType(stackTable.findSymbol(call->getValue())->getType());
    setReg(call->getReg());
    setTrueLabel(call->getTrueLabel());
    setFalseLabel(call->getFalseLabel());
}

//////////////////////////////////////////Expression///////////////////////////////////////////

// EXP -> ID //regFixed
Expression::Expression(Node* terminalExp) {  
    if (!stackTable.isDefinedInProgram(terminalExp->getValue())){
        output::errorUndef(yylineno, terminalExp->getValue());
        exit(0);
    }
    setValue(terminalExp->getValue());
    setType(stackTable.findSymbol(terminalExp->getValue())->getType());

    int offset = stackTable.findSymbol(terminalExp->getValue())->getOffset();
    string ptr = stackTable.getScope()->getBaseReg();
    string reg = codeGenerator.generateLoad(offset, ptr, terminalExp->getType());
    setReg(reg); 

    if (this->getType() == "bool") {
        string newReg = freshReg();
        string newTrueL = buffer.freshLabel();
        string newFalseL = buffer.freshLabel();
        buffer.emit(newReg + " = icmp ne i32 1, " + terminalExp->getReg());
        codeGenerator.generateCondBranch(newReg, newTrueL, newFalseL);
        setTrueLabel(newTrueL);
        setFalseLabel(newFalseL);
    }
}

// Exp -> LPAREN Type RPAREN Exp //regFixed
Expression::Expression(Type* type, Node* exp) { 
    if((exp->getType() != "int" && exp->getType() != "byte") || (type->getType() != "int" && type->getType() != "byte")){
        output::errorMismatch(yylineno);
        exit(0);
    }
    setType(type->getType());
    setValue(exp->getValue());

    string reg = emitTruncation(exp->getReg(), type->getType(), exp->getType(), true);
    setReg(reg);
}

// Exp -> Exp Binop Exp //regFixed
Expression::Expression(Node* leftExp, Node* rightExp, const string op) { 
    Expression* left = dynamic_cast<Expression *> (leftExp);
    Expression* right = dynamic_cast<Expression *> (rightExp);
    string lType = left->getType();
    string rType = right->getType();

    if ((lType != "int" && lType != "byte") || (rType != "int" && rType != "byte")) {
            output::errorMismatch(yylineno);
            exit(0);
        }
    //get Binop operation
    this->setReg(freshReg()); 
    string operation = getBinopOp(op);
    if (operation == "DIV") { 
        buffer.emit("call void @check_division_by_zero(i32 " + right->getReg() + ")");
        if (this->getType() == "int") {
            this->setType("sdiv");
            operation = "sdiv";
        } else {
            this->setType("udiv"); //for byte division
            operation = "udiv";
        }
    }

    if (lType == "byte" && rType == "byte") {  //BYTE op BYTE
        setType("byte");
        buffer.emit(this->getReg() + " = " + operation + " i8 " + left->getReg() + ", " + right->getReg());        
    }
    else if (lType == "byte" && rType == "int") { //BYTE op INT
        setType("int");
        buffer.emit(this->getReg() + " = " + operation + " i32 " + left->getReg() + ", " + right->getReg());      
    }
    else if (lType == "int" && rType == "byte") { //INT op BYTE
        setType("int");
        buffer.emit(this->getReg() + " = " + operation + " i32 " + left->getReg() + ", " + right->getReg());
    }
    else { //INT op INT
        setType("int");
        buffer.emit(this->getReg() + " = " + operation + " i32 " + left->getReg() + ", " + right->getReg());
    }
}


//////////////////////////////////////////Bool//////////////////////////////////////////
//Bool -> TRUE //regFixed
Bool::Bool(Node* exp, string trueFalse) {  // Exp -> True / False
    //create new labels for the true and false branches
    string newTrueL = CodeBuffer::instance().freshLabel();
    string newFalseL = CodeBuffer::instance().freshLabel();
    exp->setTrueLabel(newTrueL);
    exp->setFalseLabel(newFalseL);

    //emit the branch instruction
    if (trueFalse == "true") { 
        CodeBuffer::instance().emit("br label %" + newTrueL);
    } else { 
        CodeBuffer::instance().emit("br label %" + newFalseL);
    }
}

//////////////////////////////////////////Num////////////////////////////////////////////////////
//Exp -> NUM // regFixed
Num::Num(Node* exp) : Expression() { //Exp -> NUM
    setValue(exp->getValue());
    setType("int");
    setReg(freshReg());
    exp->setReg(getReg());
    CodeBuffer::instance().emit(this->getReg() + " = add i32" + exp->getValue() + " , 0");
};


////////////////////////////////////////NumB////////////////////////////////////////////////////
//Exp -> NUMB // regFixed
NumB::NumB(Node* expression) : Expression() { 
    setValue(expression->getValue());
    setType("byte");
    if (stoi(expression->getValue()) >= 256) {
        output::errorByteTooLarge(yylineno, expression->getValue());
        exit(0);
    }
    setReg(freshReg());
    buffer.emit(getReg() + " = add i8" + expression->getValue() + " , 0");
    expression->setReg(getReg());   
}

//////////////////////////////////////////String//////////////////////////////////////////
//Exp -> STRING // regFixed
String::String(Node* exp) : Expression() { //Exp -> STRING
    setValue(exp->getValue());
    setType("string");
    string global = freshGlobalReg();
    string local = freshReg();
    CodeBuffer::instance().emit(global + " = constant [" + to_string(exp->getValue().size() + 1) + " x i8]" + " c" + exp->getValue() + "\\00\"");
    CodeBuffer::instance().emit(local + ".ptr = getelementptr[" + to_string(exp->getValue().size() + 1) + " x i8]" 
                    + ", " + to_string(exp->getValue().size() + 1) + " x i8]*  " + global + ", i32 0, i32 0");
    setReg(local);   
    exp->setReg(local);
}

//////////////////////////////////////////Call//////////////////////////////////////////

// Call -> ID LPAREN Exp RPAREN  // regFixed
Call::Call(Node* terminalID, Expression* exp) : Node(terminalID->getValue(), "", "") { 
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

    //PRINTI FUNCTION
    else if (terminalID->getValue() == "printi") {
        string newReg = freshReg();
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
        string newReg = freshReg();
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

// Statement -> BREAK / CONTINUE  //regFixed
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

//Statement -> Type ID SC //regFixed
Statement::Statement(Type* type, Node * id) { //takeen 
    if (stackTable.isDefinedInProgram(id->getValue())) {
        output::errorDef(yylineno, id->getValue());
        exit(0);
    }
    id->setType(type->getType());
    id->setReg(freshReg());
    stackTable.addSymbolToProgram(id->getValue(), false, type->getType(), "");
    if(type->getType() == "bool") { //name =
        buffer.emit(id->getReg() + " = add i1" + id->getValue() + ", false");
        setTrueLabel(id->getTrueLabel());
        setFalseLabel(id->getFalseLabel());
    }
    else if (type->getType() == "byte") {
        buffer.emit(id->getReg() + " = add i8" + id->getValue() + ", 0");
    } else {
        buffer.emit(id->getReg() + " = add i32" + id->getValue() + ", 0");
    }
   
    string ptr_reg = freshReg();
    string rbp = stackTable.getScope()->getBaseReg();
    int offset = stackTable.getScope()->getOffset();
    
    //store the value in the stack
    buffer.emit(ptr_reg + " = getelementptr i32, i32* " + rbp + ", i32 " + to_string(offset));
    buffer.emit("store i32 " + id->getReg() + ", i32* %" + id->getReg());
    if (type->getType() == "bool") {
        string endLabel = buffer.freshLabel();
        string trueLabel = buffer.freshLabel();
        string falseLabel = buffer.freshLabel();

        //emit the branch instruction
        codeGenerator.generateUncondBranch(trueLabel); 
        codeGenerator.generateUncondBranch(falseLabel);
        codeGenerator.defineLable(trueLabel);
        codeGenerator.generateUncondBranch(endLabel);

        codeGenerator.defineLable(falseLabel);
        codeGenerator.generateUncondBranch(endLabel);

        codeGenerator.defineLable(endLabel);
    }
}

// Statement -> Type ID Assign Exp SC //regFixed
Statement::Statement(Type* type, Node * id, Expression * exp) { //maybe i need to check if the valueExp is legal func but not considered as function
     if (!stackTable.isDefinedInProgram(id->getValue())) {
        output::errorUndef(yylineno, id->getValue());
        exit(0); 
    }
    // check idType == expType
    if (!LegalType((stackTable.findSymbol(id->getValue()))->getType(), exp->getType())) {
        output::errorMismatch(yylineno);
        exit(0);
    }
    // check if id == illegal byte value
    if (id->getType() == "byte" && (stoi(exp->getValue()) > 255 || stoi(exp->getValue()) < 0)) {
        output::errorByteTooLarge(yylineno, exp->getValue());
        exit(0);
    }
    //function without args case: print; | printi; | readi;
    // if( (exp->getType() == "printi" || "readi" || "readi") && ) { /// i need to hanle this case later 
    //     output::errorPrototypeMismatch(yylineno, exp->getValue(), "int");
    //     exit(0);
    // }

    if (exp->getType() == "byte") {
        string byteReg = freshReg();
        buffer.emit(byteReg + " = zext i8 " + exp->getReg() + " to i32");
        this->setReg(byteReg);
    }
    if (exp->getType() == "bool") {
        string boolReg = freshReg();
        string boolTrueL = exp->getTrueLabel();
        string boolFalseL = exp->getFalseLabel();
        string boolEndL = buffer.freshLabel(); 
        //emit true label
        codeGenerator.defineLable(boolTrueL);  
        codeGenerator.generateUncondBranch(boolEndL);
        //emit false label
        codeGenerator.defineLable(boolFalseL); 
        codeGenerator.generateUncondBranch(boolEndL);
        //emit end label
        codeGenerator.defineLable(boolEndL); 
        //now hanndle the fucking phi
        buffer.emit(boolReg + " = phi i32 [ 1, %" + boolTrueL + " ], [ 0, %" + boolFalseL + " ]");
        this->setReg(boolReg);
    }
    if (exp->getType() == "int") { 

        this->setReg(exp->getReg());
    }

    stackTable.addSymbolToProgram(id->getValue(), false, exp->getType(), "");
    id->setType(exp->getType());
    codeGenerator.generateStore((stackTable.findSymbol(id->getValue()))->getOffset(), this->getReg(), (stackTable.getScope())->getBaseReg());
}

// Statement -> ID Assign Exp SC //regFixed
Statement::Statement(Node * id, Expression * exp) { //maybe i need to check if the valueExp is legal func but not considered as function
    if (!stackTable.isDefinedInProgram(id->getValue())) {
        output::errorUndef(yylineno, id->getValue());
        exit(0); 
    }
    // check idType == expType
    if (!LegalType((stackTable.findSymbol(id->getValue()))->getType(), exp->getType())) {
        output::errorMismatch(yylineno);
        exit(0);
    }
    // check if id == illegal byte value
    if (id->getType() == "byte" && (stoi(exp->getValue()) > 255 || stoi(exp->getValue()) < 0)) {
        output::errorByteTooLarge(yylineno, exp->getValue());
        exit(0);
    }
    //function without args case: print; | printi; | readi;
    // if( (exp->getType() == "printi" || "readi" || "readi") && ) { /// i need to hanle this case later 
    //     output::errorPrototypeMismatch(yylineno, exp->getValue(), "int");
    //     exit(0);
    // }

    if (exp->getType() == "byte") {
        string byteReg = freshReg();
        buffer.emit(byteReg + " = zext i8 " + exp->getReg() + " to i32");
        this->setReg(byteReg);
    }
    if (exp->getType() == "bool") {
        string boolReg = freshReg();
        string boolTrueL = exp->getTrueLabel();
        string boolFalseL = exp->getFalseLabel();
        string boolEndL = buffer.freshLabel(); 
        //emit true label
        codeGenerator.defineLable(boolTrueL);  
        codeGenerator.generateUncondBranch(boolEndL);
        //emit false label
        codeGenerator.defineLable(boolFalseL); 
        codeGenerator.generateUncondBranch(boolEndL);
        //emit end label
        codeGenerator.defineLable(boolEndL); 
        //now hanndle the fucking phi
        buffer.emit(boolReg + " = phi i32 [ 1, %" + boolTrueL + " ], [ 0, %" + boolFalseL + " ]");
        this->setReg(boolReg);
    }
    if (exp->getType() == "int") { 

        this->setReg(exp->getReg());
    }

    stackTable.addSymbolToProgram(id->getValue(), false, exp->getType(), "");
    id->setType(exp->getType());
    codeGenerator.generateStore((stackTable.findSymbol(id->getValue()))->getOffset(), this->getReg(), (stackTable.getScope())->getBaseReg());
}

// Statement -> IF|IF-ELSE|WHILE LP EXP RP SS 
Statement::Statement(const string cond, BooleanExpression* boolexp) {
    if (boolexp->getType() == "byte" && (stoi(boolexp->getValue()) > 255 || stoi(boolexp->getValue()) < 0)) {
        output::errorByteTooLarge(yylineno, boolexp->getValue());
        exit(0);
    }
    if (boolexp->getType() != "bool") {
        output::errorMismatch(yylineno);
        exit(0);
    }

    if (cond == "WHILE") {
        codeGenerator.generateUncondBranch(stackTable.getScope()->getEntryLabel());
        codeGenerator.defineLable(stackTable.getScope()->getEntryLabel());
        codeGenerator.defineLable(boolexp->getFalseLabel());    
        codeGenerator.generateUncondBranch(stackTable.findInnermostLoopScope()->getEntryLabel());
    }
    else if (cond == "IF") {
        codeGenerator.generateUncondBranch(boolexp->getFalseLabel());
        codeGenerator.defineLable(boolexp->getTrueLabel());
    }
    else { //IF-ELSE
        codeGenerator.generateUncondBranch(stackTable.getScope()->getEntryLabel());
        codeGenerator.defineLable(boolexp->getTrueLabel());
        codeGenerator.defineLable(getNextLabel()); //Statement Next Label
    }
}
 Program::Program() {
        codeGenerator.emitProgramEnd();
    };
    
// Statement L Statement R 
Statement::Statement(Statement* Statement) { }

void Statement::afterElse() {
    setNextLabel(buffer.freshLabel());
    codeGenerator.defineLable(getNextLabel()); //Statement Next Label
}