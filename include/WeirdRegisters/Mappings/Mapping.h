//=============================================================================
// FILE:
//    Mapping.h
//
// DESCRIPTION:
//    Manage the mapping of weird registers to memory locations
//
// License: MIT
//=============================================================================
#ifndef MAPPING_H
#define MAPPING_H

#include "llvm/IR/IRBuilder.h"

class Flexo;

using namespace llvm;

class Mapping {
public:
  Mapping(Flexo* flexo, IRBuilder<>& builder)
  : flexo(flexo), builder(builder), count(-1) {}
  virtual ~Mapping() = default;

  virtual void init(int count) { this->count = count; };
  virtual int getRealId(int regId) { return regId; };
  virtual Value* getRuntimeRealId(int regId) { return nullptr; };

  // mapping constructed at compile time?
  bool compileTimeMap = true;

protected:
  Flexo* flexo;
  IRBuilder<>& builder;
  // number of weird registers
  int count;
};

#endif
