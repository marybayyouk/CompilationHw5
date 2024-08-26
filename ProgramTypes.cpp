#include "ProgramTypes.hpp"
#include "CodeGenerator.hpp"

extern int yylineno;
extern StackTable stackTable;
extern CodeBuffer buffer;
extern CodeGenerator codeGenerator;
using namespace std;


void generateElfStatements(BooleanExpression* exp, bool isElf) {
    // buffer.emit("ElfStatement started: ");
    if (isElf) 
        buffer.emit("br label %" + exp->getTrueLabel() + "\n" + exp->getTrueLabel() + ":");
    else
        buffer.emit("br label %" + exp->getFalseLabel() + "\n" + exp->getFalseLabel() + ":");    
    // buffer.emit("ElfStatement ended: ");

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
        string strReg = buffer.freshReg();;
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
    if (boolExp->getValue() == "true") {
        setValue("false");
    } else {
        setValue("true");
    }
}

// BooleanExpression -> Exp RELOP/AND/OR Exp //regFixed
BooleanExpression::BooleanExpression(Node* leftExp, Node* rightExp, const string op) { 
    string lType = leftExp->getType();
    string rType = rightExp->getType();
    if (leftExp->getTrueLabel() == "") {
        //we reach here from Bool
        leftExp->setTrueLabel(buffer.freshLabel());
        leftExp->setFalseLabel(buffer.freshLabel());
    }
    if (rightExp->getTrueLabel() == "") {
        //we reach here from Bool
        rightExp->setTrueLabel(buffer.freshLabel());
        rightExp->setFalseLabel(buffer.freshLabel());
    }

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
            // buffer.emit("OR started: ");
            codeGenerator.defineLable(leftExp->getTrueLabel()); //before i defined Lexp->true
            codeGenerator.generateUncondBranch(rightExp->getTrueLabel()); //rightExp->true
            // buffer.emit("OR ended: ");
            if ((stackTable.findSymbol(leftExp->getValue())->truefalse == "true") || (stackTable.findSymbol(rightExp->getValue())->truefalse == "true"))
            {
                setValue("true");
            }
            else {
                setValue("false");
            }  
        } else {
            if ((stackTable.findSymbol(leftExp->getValue())->truefalse == "false") || (stackTable.findSymbol(rightExp->getValue())->truefalse == "false"))
                {
                    setValue("false");
                }
                else {setValue("true");
                }
            // buffer.emit("And started: ");
            codeGenerator.defineLable(leftExp->getFalseLabel());
            codeGenerator.generateUncondBranch(rightExp->getFalseLabel());
        //    buffer.emit("And ended: ");

        }
    } else { //RELOP
        //buffer.emit("Relop started: ");
        if ((lType != "int" && lType != "byte") || (rType != "int" && rType != "byte")) {
            output::errorMismatch(yylineno);
            exit(0);
        }
        setType("bool");
        setTrueLabel(buffer.freshLabel());
        setFalseLabel(buffer.freshLabel());
        setReg(buffer.freshReg());
        string oper = getRelopOp(op);

        if (lType == "byte" && rType == "byte") {  //BYTE op BYTE
            //setType("byte");
            buffer.emit(getReg() + " = " + oper + " i8 " + leftExp->getReg() + ", " + rightExp->getReg());        
        }
        else if (lType == "byte" && rType == "int") { //BYTE op INT
            //setType("int"); /// are we sure?!
            emitTruncation(leftExp->getReg(), "byte", true);       
        }
        else if (lType == "int" && rType == "byte") { //INT op BYTE
            //setType("int");
            emitTruncation(rightExp->getReg(), "int", true);
        } else { //INT op INT
            //setType("int");
            buffer.emit(getReg() + " = " + oper + " i32 " + leftExp->getReg() + ", " + rightExp->getReg());
        }
        codeGenerator.generateCondBranch(getReg(), getTrueLabel(), getFalseLabel());
        //buffer.emit("Relop ended: ");
    }
}

//Bool Expression for CHECKBOOL
BooleanExpression::BooleanExpression(Node* exp) {
    string newReg;
    if(exp->getType() == "bool") {
        // determine if its true or false
        string boolValue = (stackTable.findSymbol(exp->getValue())->truefalse == "true") ? "1" : "0";
        if (exp->getReg() == "") {
            // we got here from bool
            if (boolValue == "0") {
                newReg = buffer.freshReg();
                buffer.emit(newReg + " = add i32 0, 0");
            } else {
                string freshReg = buffer.freshReg();
                buffer.emit(freshReg + " = add i32 0, 0");  // Set the fresh register to 0
                newReg = buffer.freshReg();
                buffer.emit(newReg + " = add i32 " + freshReg + ", 1");  
            }
            setReg(newReg);
        } else {
            setReg(exp->getReg());
        }
        setType("bool");
        setValue(exp->getValue());
        if (exp -> getTrueLabel() == "") {
            // we reach here only if we came from Bool constructor
            setTrueLabel(buffer.freshLabel());
        } else {
            // we already created true and false, probably from bool expression
            setTrueLabel(exp->getTrueLabel());
        }
        if (exp -> getFalseLabel() == "") {
            // we reach here only if we came from Bool constructor
            setFalseLabel(buffer.freshLabel());

        } else {
            setFalseLabel(exp->getFalseLabel());
        }
        // load the stroed value
        string newCmp = codeGenerator.generateIcmp("eq", "1", getReg());
        codeGenerator.generateCondBranch(newCmp, getTrueLabel(), getFalseLabel());
    } else {
        // initialize the expression with the value of the expression - regular expression -
        setValue(exp->getValue());
        setType(exp->getType());
        setReg(exp->getReg());
    }
}

// EXP -> Call
Expression::Expression(Call* call, bool flag) {
    setValue(call->getValue());
    setType(stackTable.findSymbol(call->getValue())->getType());
    string name = codeGenerator.generateLoad(stackTable.findSymbol(call->getValue())->getOffset(), stackTable.getScope()->getBaseReg(), call->getType());
    setReg(call->getReg());
    if (call->getType() == "byte") {
        string trunRet = emitTruncation(name, "byte", false);
        setReg(trunRet);
    }
    else if (call->getType() == "bool") {
    setTrueLabel(buffer.freshLabel());
    setFalseLabel(buffer.freshLabel());
    string newCmp = codeGenerator.generateIcmp("eq", "1", this->getReg());
    codeGenerator.generateCondBranch(newCmp, getTrueLabel(), getFalseLabel());
    }
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

    if (this->getType() == "bool") {
        string newReg2 = buffer.freshReg();
        string newReg = buffer.freshReg();
        string newTrueL = buffer.freshLabel();
        string newFalseL = buffer.freshLabel();
        string freshReg;
        string boolValue = (stackTable.findSymbol(terminalExp->getValue())->truefalse == "true") ? "1" : "0";

        if (boolValue == "1") {
            newReg = buffer.freshReg();
            buffer.emit(newReg + " = add i32 0, 0");  // Set the fresh register to 0
            freshReg = buffer.freshReg();
            buffer.emit(freshReg + " = add i32 " + newReg + ", 1");  
        } else {
            freshReg = buffer.freshReg();
            buffer.emit(freshReg + " = add i32 0, 0");  // Set the fresh register to 0
        }
        setReg(freshReg);
        buffer.emit(newReg2 + " = icmp eq i32 1, " + freshReg);
        codeGenerator.generateCondBranch(newReg2, newTrueL, newFalseL);
        setTrueLabel(newTrueL);
        setFalseLabel(newFalseL);
        //buffer.emit("Exp->ID ended: ");
    }
    else {
        int offset = stackTable.findSymbol(terminalExp->getValue())->getOffset();
        string ptr = stackTable.getScope()->getBaseReg();

        string reg = codeGenerator.generateLoad(offset, ptr, terminalExp->getType());    
        setReg(reg); 
    }
}

// Exp -> LPAREN Type RPAREN Exp //regFixed
Expression::Expression(Type* type, Node* exp) { 
    //buffer.emit( "Exp -> LPAREN Type RPAREN Exp: " );
    if((exp->getType() != "int" && exp->getType() != "byte") || (type->getType() != "int" && type->getType() != "byte")){
        output::errorMismatch(yylineno);
        exit(0);
    }
    setType(type->getType());
    setValue(exp->getValue());
    string reg = emitTruncation(exp->getReg(), type->getType(), true);
    setReg(reg);
    //buffer.emit( "Exp -> LPAREN Type RPAREN Exp: " );
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
    this->setReg(buffer.freshReg()); 
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
        setType("int"); /// are we sure?!
        emitTruncation(left->getReg(), "byte", true);       
    }
    else if (lType == "int" && rType == "byte") { //INT op BYTE
        setType("int");
        emitTruncation(right->getReg(), "int", true);
    } else { //INT op INT
        setType("int");
        buffer.emit(this->getReg() + " = " + operation + " i32 " + left->getReg() + ", " + right->getReg());
    }
}


//////////////////////////////////////////Bool//////////////////////////////////////////
//Bool -> TRUE | false //regFixed
Bool::Bool(Node* exp, string trueFalse) {  // Exp -> True / False
    //create new labels for the true and false branches
    string newTrueL = buffer.freshLabel();
    string newFalseL = buffer.freshLabel();
    setTrueLabel(newTrueL);
    setFalseLabel(newFalseL);
    setType("bool");
    setValue(trueFalse);
    // stackTable.findSymbol(exp->getValue())->truefalse = trueFalse;
    // cout<<stackTable.findSymbol(exp->getValue())->truefalse<<endl;
}

//////////////////////////////////////////Num////////////////////////////////////////////////////
//Exp -> NUM // regFixed
Num::Num(Node* exp) : Expression() { //Exp -> NUM
    setValue(exp->getValue());
    setType("int");
    setReg(buffer.freshReg());
    exp->setReg(getReg());
    buffer.emit(getReg() + " = add i32" + exp->getValue() + " , 0");
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
    setReg(buffer.freshReg());
    buffer.emit(getReg() + " = add i8" + expression->getValue() + " , 0");
    expression->setReg(getReg());   
}

//////////////////////////////////////////String//////////////////////////////////////////
//Exp -> STRING // regFixed
String::String(Node* exp) : Expression() { //Exp -> STRING
    // Out scanner produces extra "" so we strip them here
    std::string value = exp->getValue();
    if (!value.empty() && value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.size() - 2);
    }

    setValue(value);
    setType("string");
    string global = freshGlobalReg();
    string local = buffer.freshReg();

    buffer.emitGlobal(global + " = constant [" + to_string(value.size() + 1) + " x i8]" + " c\"" + value + "\\00\"");
    buffer.emit(local + ".ptr = getelementptr[" + to_string(value.size() + 1) + " x i8]" 
                    + ", [" + to_string(value.size() + 1) + " x i8]*  " + global + ", i32 0, i32 0");
    setReg(local);   
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

        buffer.emit(getCallEmitLine(terminalID->getValue(), exp->getReg()+".ptr"));
    }
    
    //PRINTI FUNCTION
    else if (terminalID->getValue() == "printi") {
        string args = buffer.freshReg();
        string newReg = buffer.freshReg();;
        if (exp->getType() != "byte" && exp->getType() != "int") {
            output::errorPrototypeMismatch(yylineno,terminalID->getValue(), "int");
            exit(0);
        }
        if( exp->getType() == "byte") {
            buffer.emit(newReg + " = zext i8 " + exp->getReg() + " to i32");
        }
        buffer.emit(args + " = add i32 " + newReg + ", 0");
        buffer.emit(getCallEmitLine(terminalID->getValue(), newReg));
    }
    //MUST BE READI FUNCTION
    else { 
        string args = buffer.freshReg();
        string newReg = buffer.freshReg();
        string result = buffer.freshReg();
        if(exp->getType() != "byte" && exp->getType() != "int") {
            output::errorPrototypeMismatch(yylineno,terminalID->getValue(), "int");
            exit(0);
        }
        if( exp->getType() == "byte") {
            buffer.emit(newReg + " = zext i8 " + exp->getReg() + " to i32");
        }
        buffer.emit(args + " = add i32 " + newReg + ", 0");
        setType(stackTable.findSymbol(terminalID->getValue())->getType());
        setValue(stackTable.findSymbol(terminalID->getValue())->getName());
        buffer.emit(result + "= call i32 @readi(i32 " + args + ");");
        setReg(result);
        codeGenerator.generateStore(stackTable.findSymbol(terminalID->getValue())->getOffset(), result, stackTable.getScope()->getBaseReg());
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
    id->setReg(buffer.freshReg());
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
   
    string ptr_reg = buffer.freshReg();;
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
 if (stackTable.isDefinedInProgram(id->getValue())) {
        output::errorDef(yylineno, id->getValue());
        exit(0); 
    }
    // check idType == expType
    if (!LegalType(type->getType(), exp->getType())) {
        output::errorMismatch(yylineno);
        exit(0);
    }
    // check if id == illegal byte value
    if (type->getType() == "byte" && (stoi(exp->getValue()) > 255 || stoi(exp->getValue()) < 0)) {
        output::errorByteTooLarge(yylineno, exp->getValue());
        exit(0);
    }
    //buffer.emit("exp->reg: " + exp->getReg());
    if (exp->getType() == "byte") {
        string byteReg = buffer.freshReg();;
        buffer.emit(byteReg + " = zext i8 " + exp->getReg() + " to i32");
        this->setReg(byteReg);
    }
    if (exp->getType() == "bool") {
        string boolReg = buffer.freshReg();
        setReg(boolReg);
        string boolTrueL = buffer.freshLabel();
        string boolFalseL = buffer.freshLabel();
        string boolEndL = buffer.freshLabel(); 
        if (exp->getValue() == "true") {
            codeGenerator.generateUncondBranch(boolTrueL);
        } else {
            codeGenerator.generateUncondBranch(boolFalseL);
        }
        //emit true label
        codeGenerator.defineLable(boolTrueL);  
        codeGenerator.generateUncondBranch(boolEndL);
        //emit false label
        codeGenerator.defineLable(boolFalseL); 
        codeGenerator.generateUncondBranch(boolEndL);
        //emit end label
        codeGenerator.defineLable(boolEndL); 
        // //now hanndle the fucking phi
        buffer.emit(boolReg + " = phi i32 [ 1, %" + boolTrueL + " ], [ 0, %" + boolFalseL + " ]");
        // buffer.emit(this->getReg() + " = add i32 " + exp->getReg() + ", 0");
    }
    if (exp->getType() == "int") { 

        this->setReg(exp->getReg());

    }

    stackTable.addSymbolToProgram(id->getValue(), false, exp->getType(), "");
    stackTable.findSymbol(id->getValue())->truefalse = exp->getValue();
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
        //cout << "id assign exp sc" << endl;
        output::errorMismatch(yylineno);
        exit(0);
    }
    // check if id == illegal byte value
    if (id->getType() == "byte" && (stoi(exp->getValue()) > 255 || stoi(exp->getValue()) < 0)) {
        output::errorByteTooLarge(yylineno, exp->getValue());
        exit(0);
    }

    if (exp->getType() == "byte") {
        string byteReg = buffer.freshReg();;
        buffer.emit(byteReg + " = zext i8 " + exp->getReg() + " to i32");
        this->setReg(byteReg);
    }
    if (exp->getType() == "bool") {
        string boolReg = buffer.freshReg();;
        setReg(boolReg);
        string boolTrueL = exp->getTrueLabel();
        string boolFalseL = exp->getFalseLabel();
        string boolEndL = buffer.freshLabel(); 
       if (exp->getValue() == "true") {
            codeGenerator.generateUncondBranch(boolTrueL);
        } else {
            codeGenerator.generateUncondBranch(boolFalseL);
        }
        //emit true label
        codeGenerator.defineLable(boolTrueL);  
        codeGenerator.generateUncondBranch(boolEndL);
        //emit false label
        codeGenerator.defineLable(boolFalseL); 
        codeGenerator.generateUncondBranch(boolEndL);
        //emit end label
        codeGenerator.defineLable(boolEndL); 
        // //now hanndle the fucking phi
        buffer.emit(boolReg + " = phi i32 [ 1, %" + boolTrueL + " ], [ 0, %" + boolFalseL + " ]");
        // buffer.emit(this->getReg() + " = add i32 " + exp->getReg() + ", 0");
        
    }
    if (exp->getType() == "int") { 

        this->setReg(exp->getReg());
    }

    stackTable.addSymbolToProgram(id->getValue(), false, exp->getType(), "");
    stackTable.findSymbol(id->getValue())->truefalse = exp->getValue();
    id->setType(exp->getType());
    codeGenerator.generateStore((stackTable.findSymbol(id->getValue()))->getOffset(), this->getReg(), (stackTable.getScope())->getBaseReg());
}

// Statement -> IF|IF-ELSE|WHILE LP EXP RP SS 
Statement::Statement(const string cond, BooleanExpression* boolexp, string nextLabel) {
    if (boolexp->getType() == "byte" && (stoi(boolexp->getValue()) > 255 || stoi(boolexp->getValue()) < 0)) {
        output::errorByteTooLarge(yylineno, boolexp->getValue());
        exit(0);
    }
    if (boolexp->getType() != "bool") {
        //cout << "line 556: " << boolexp->getType() << endl;
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
        codeGenerator.defineLable(boolexp->getFalseLabel());    
    }
    else { //IF-ELSE
        setNextLabel(nextLabel);
        codeGenerator.generateUncondBranch(nextLabel);
        codeGenerator.defineLable(nextLabel);
    }
}
 Program::Program() {
        codeGenerator.emitProgramEnd();
    };
    
// Statement L Statement R 
Statement::Statement(Statement* Statement) { }

string Statement::afterElse() {
    setNextLabel(buffer.freshLabel());
    codeGenerator.generateUncondBranch(getNextLabel());
    return this->getNextLabel();
}