//=============================================================================
// FILE:
//    NoBranchWR.h
//
// DESCRIPTION:
//    Assign an arch value to WR without using branch instructions
//
// License: MIT
//=============================================================================
#ifndef NO_BRANCH_WR_H
#define NO_BRANCH_WR_H

#include "WeirdRegisters/WeirdRegisters.h"

using namespace llvm;

class NoBranchWR: public WeirdRegisters
{
public:
  NoBranchWR(
    Flexo* flexo, IRBuilder<>& builder, int align
  ) : WeirdRegisters(flexo, builder, align) {};
  ~NoBranchWR() override = default;

  void assignArch(Value* archVal, int regId, bool invert = false) override;

protected:
  Value* getWRegGEP(Value* offset, StringRef name);
};

#endif
