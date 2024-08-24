#include "SymbolTable.hpp"
#include "CodeGenerator.hpp"
#include "cg.hpp"

extern CodeGenerator codeGenerator;
CodeBuffer buffer;


/////////////////////////////////////////////////SymbolTable//////////////////////////////////////////////////////////
SymbolTable::SymbolTable(int maxOff,bool isloop) : symbols() { 
    currentOffset = maxOff;
    isLoop = isloop;
    this->setBaseReg("");
    this->setNextLabel("");
}

SymbolTable::~SymbolTable() {
    for (Symbol* symbol : symbols) 
         delete symbol; 
}

bool SymbolTable::isDefinedInTable(const string& name) {
    for (const Symbol* symbol : symbols) {
        if (symbol->getName() == name) {
            return true;
        }
    }
    return false;
}

Symbol* SymbolTable::findSymbol(const string& name){   
    if(!isDefinedInTable(name)) {
        return nullptr;
    }
    for (Symbol* symbol : symbols) {
        if (symbol->getName() == name) {
            return symbol;
        }
    }
    return nullptr;
}



void SymbolTable::addSymbol(Symbol* symbol) {
    if(isDefinedInTable(symbol->getName())) {
        return;
    }
    symbols.push_back(symbol);
    currentOffset = symbol->getOffset() ;
}


/////////////////////////////////////////////////StackTable//////////////////////////////////////////////////////////
StackTable::StackTable() : scopes(), scopesOffset() {
    pushScope(false);
    scopesOffset.push_back(0);
    (scopes.back())->addSymbol(new Symbol("print", 0, true, "string", {"string"}));
    (scopes.back())->addSymbol(new Symbol("printi", 0, true, "int", {"int"}));
    (scopes.back())->addSymbol(new Symbol("readi", 0, true, "int", {"int"}));
    scopesOffset.back() = 3;
}

StackTable::~StackTable() {
    for(SymbolTable* scope : scopes) {
        delete scope;
    }
}

void StackTable::pushScope(bool isLoop) {
     int currOffset = 0;
    if(scopesOffset.size() > 0) {
        currOffset = scopesOffset.back();
    }
    SymbolTable* symTable = new SymbolTable(currOffset, isLoop);
    scopes.push_back(symTable);
    if(scopes.size() > 0) {
        scopesOffset.push_back(currOffset); 
        scopes.back()->setBaseReg(scopes.back()->getBaseReg()); 
   }
    scopes.push_back(symTable);
    if(isLoop) 
        handleLoopScope(symTable);
}

string StackTable::setFunctionType(string funcName) {
    for(SymbolTable* scope : scopes) {
        if(scope->getIsLoop() && funcName == "print") {
            return scope->findSymbol(funcName)->getType();
        }
    }
    if(funcName == "print") {
        return output::makeFunctionType("string", "void");
    }
    else if(funcName == "printi") {
        return output::makeFunctionType("int", "void");
    }
    return output::makeFunctionType("int", "int");
}

void StackTable::popScope() {
    //output::endScope();
    // for (Symbol* symbol : scopes.back()->symbols) {
    //     string name = symbol->getName();
    //     int offs = symbol->getOffset();
    //     string type = upperCase(symbol->getType());
    //     if(symbol->getIsFunction()) {
    //         //type = setFunctionType(name);
    //         buffer.emit(scopes.back()->getNextLabel() + ":");
    //     }
    //    // output::printID(name, offs, type);
    // }
    if (scopes.back()->getIsLoop()) {
        codeGenerator.defineLable(scopes.back()->getNextLabel());
    }
    scopes.pop_back();
    scopesOffset.pop_back();
}

bool StackTable::isDefinedInProgram(const string& name) {
    for (SymbolTable* scope : scopes) {
        if (scope->isDefinedInTable(name)) 
            return true;
    }
    return false;
}

Symbol* StackTable::findSymbol(const string& name) {
    for (SymbolTable* scope : scopes) {
        Symbol* symbol = scope->findSymbol(name);
        if (symbol != nullptr) 
            return symbol;
    }
    return nullptr;
}

void StackTable::addSymbolToProgram(const string& name, bool isFunc, const string& type, vector<string> names) {
    //SymbolTable* toRecover = scopes.back();
    if(!isFunc) {
        //newOffset = offsets.back();
        //offsets.push_back(newOffset + 1); 
    }
    Symbol * newSymbol = new Symbol(name, scopesOffset.back(), isFunc, type, names);
    scopes.back()->addSymbol(newSymbol);    
    scopesOffset.back() += 1;
}

SymbolTable* StackTable::getScope(){
    return scopes.back();
}

SymbolTable* StackTable::findInnermostLoopScope() {
    SymbolTable* currScope;;
    // Start from the most recent (innermost) scope and work backwards
    for (int i = scopes.size() - 1; i >= 0; i--) {
        if (currScope && currScope->getIsLoop()) 
            break;  // Return the first loop scope found, which is the innermost
    }
    return currScope;  // Return nullptr if no loop scope is found
}

void StackTable::handleLoopScope(SymbolTable* loopScope) {
    loopScope->setEntryLable(loopScope->getNextLabel());
    loopScope->setNextLabel(loopScope->getNextLabel());
    buffer.emit("br label %" + loopScope->getEntryLabel());
    codeGenerator.defineLable(loopScope->getEntryLabel());
}