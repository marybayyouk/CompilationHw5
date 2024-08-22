#ifndef __SYMBOLTABLE_H_
#define __SYMBOLTABLE_H_

#include <iostream>
#include <string>
#include <vector>
#include "hw3_output.hpp"

using std::string;
using std::vector;

struct NameTypeInfo {
    string type;
    vector<string> names;
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
    int currentOffset;
    bool isLoop;
public:
    vector<Symbol*> symbols;
    SymbolTable(int maxOff,bool isLoop);
    ~SymbolTable();
    int getOffset() const { return currentOffset; }
    bool getIsLoop() const { return isLoop; }
    bool isDefinedInTable(const string& name);
    Symbol* findSymbol(const string& symName);
    void addSymbol(Symbol* symbol);
};

class StackTable {  ///stack of scopes
public:
    vector<SymbolTable*> scopes;
    vector<int> offsets;    
    StackTable();
    ~StackTable();
    void pushScope(bool isLoop);
    void popScope();
    string setFunctionType(string funcName);
    bool isDefinedInProgram(const string& symName);
    void addSymbolToProgram(const string& name, bool isFunc, const string& type, vector<string> names);
    SymbolTable* getScope();
    Symbol* findSymbol(const string& symName);
};

#endif // __SYMBOLTABLE_H_