//==============================================================================
// FILE:
//    IRParser.h
//
// DESCRIPTION:
//    Parse LLVM IR and translate to a verilog module
//
// License: MIT
//==============================================================================
#ifndef IR_PARSER_H
#define IR_PARSER_H

#include <string>
#include <utility>
#include <vector>
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

using namespace std;
using namespace llvm;

struct bus {
  string name;
  unsigned width;
  bool isArg;
  bool isInput;
  bool isOutput;
};

class IRParser {
public:
  IRParser(Function* F, bool verbose = false);
  ~IRParser() { for (auto bus: wires) delete bus; }

  bool verbose = false;

  string dump(string name);

private:
  // parsers  
  void parseBinaryOp(BinaryOperator* BinOp);
  void parseSelect(SelectInst* selectInst);
  void parseCast(CastInst* castInst);
  void parseIcmp(ICmpInst* icmpInst);
  void parseGEP(GetElementPtrInst* gepInst);
  void parseLoad(LoadInst* loadInst);
  void parseStore(StoreInst* storeInst);
  void parseCall(CallInst* callInst);

  // bus related
  bus* getBus(string name, unsigned width);
  bus* getOrCreateBus(string name, unsigned width);
  string valToBusName(Value* val, bool isOutput = false);

  // bus name -> bus
  StringMap<bus*> busMap;
  // list of arguments
  vector<bus*> args;
  // GEPs name -> bus and bus offset
  StringMap<pair<bus*,unsigned>> gepMap;
  // all wires (arguments + internal wires)
  vector<bus*> wires;
  // verilog code (without 'assign' and ';')
  vector<string> lines;
};

#endif
