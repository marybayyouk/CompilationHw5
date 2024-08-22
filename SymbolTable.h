#ifndef __SYMBOLTABLE_H_
#define __SYMBOLTABLE_H_

#include <iostream>
#include <string>
#include <vector>
#include "hw3_output.hpp"

using std::string;
using std::vector;

struct NameTypeInfo {
    string type; ///int, string, void, etc.
    vector<string> names; ///for functions, the names of the parameters
};

class Symbol {  ///variable or function
private:
    string symName; 
    int symOffset;
    bool isFunc;
    NameTypeInfo NameType;
public:
    Symbol(const string& name, int offset, bool isfunc, const string& type, vector<string> names) : 
                                            symName(name), symOffset(offset), isFunc(isfunc), NameType{type, names} {}
    ~Symbol() = default;
    ///GETTERS
    NameTypeInfo getNameType() const { return NameType; }
    string getName() const { return symName; }
    int getOffset() const { return symOffset; }
    bool getIsFunction() const { return isFunc; }
    string getType() const { return NameType.type; }
};

class SymbolTable {  ///scope
private:
    string baseReg; //rbp
    int currentOffset;
    string nextLable; // next scope label
    string entryLabel; // current scope begin label
    bool isLoop;
public:
    vector<Symbol*> symbols;
    SymbolTable(int maxOff,bool isLoop);
    ~SymbolTable();
    bool isDefinedInTable(const string& name);
    Symbol* findSymbol(const string& symName);
    void addSymbol(Symbol* symbol);
    ///GETTERS 
    string getBaseReg() const { return baseReg; }
    string getEntryLabel() const { return entryLabel; }
    string getNextLabel() const { return nextLable; }
    int getOffset() const { return currentOffset; }
    bool getIsLoop() const { return isLoop; }
    ///SETTERS
    void setBaseReg(const string& reg) { baseReg = reg; }
    void setNextLabel(const string& label) { nextLable = label; }
    void setEntryLable(const string& label) { entryLabel = label; }
    void setOffset(int offset) { currentOffset = offset; }
    void setIsLoop(bool isloop) { isLoop = isloop; }
};

class StackTable {  ///stack of scopes

public:
    vector<SymbolTable*> scopes; 
    vector<int> scopesOffset; //   

    StackTable();
    ~StackTable();
    void pushScope(bool isLoop);
    void popScope();
    string setFunctionType(string funcName);
    bool isDefinedInProgram(const string& symName);
    void addSymbolToProgram(const string& name, bool isFunc, const string& type, vector<string> names);
    SymbolTable* getScope();
    Symbol* findSymbol(const string& symName);
    SymbolTable* findInnermostLoopScope();
    void handleLoopScope(SymbolTable* loopScope);
};

#endif // __SYMBOLTABLE_H_