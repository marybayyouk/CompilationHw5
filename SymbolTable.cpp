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
        if(symbol) {
         delete symbol; 
         symbol = nullptr; 
        }
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
    (scopes.back())->addSymbol(new Symbol("print", 0, true, "string", "void"));
    (scopes.back())->addSymbol(new Symbol("printi", 0, true, "int", "void"));
    (scopes.back())->addSymbol(new Symbol("readi", 0, true, "int", "int"));
    // (scopes.back()->addSymbol(new Symbol("true", 0, false, "bool", {""})));
    // (scopes.back()->addSymbol(new Symbol("false", 0, false, "bool", {""})));
    // for (const Symbol* symbol : getScope()->symbols) {
    //     cout<<"the name"<< endl;
    //     cout<<symbol->getName()<<endl;
    // }

    scopesOffset.back() = 3;
}

StackTable::~StackTable() {
    for(SymbolTable* scope : scopes) {
        if(scope) {
            cout<<scope->getIsLoop()<<endl;
            delete scope;
            scope = nullptr;
        }
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
    if(isLoop) {
        handleLoopScope(symTable);
        symTable->setEntryLable(buffer.freshLabel());
        symTable->setNextLabel(buffer.freshLabel());
    } 
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
    SymbolTable* temp = scopes.back();
    if (temp->getIsLoop()) {
        codeGenerator.defineLable(temp->getNextLabel());
    }
    scopes.pop_back();
    scopesOffset.pop_back();
    // delete temp;
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

void StackTable::addSymbolToProgram(const string& name, bool isFunc, const string& type, const string& names) {
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