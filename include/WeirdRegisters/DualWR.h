//=============================================================================
// FILE:
//    DualWR.h
//
// DESCRIPTION:
//    Assign an arch value to WR without using branch instructions and
//    additional registers
//
// License: MIT
//=============================================================================
#ifndef DUAL_WR_H
#define DUAL_WR_H

#include "WeirdRegisters/NoBranchWR.h"

using namespace llvm;

class DualWR: public NoBranchWR
{
public:
  DualWR(Flexo* flexo, IRBuilder<>& builder, int align)
  : NoBranchWR(flexo, builder, align) {};
  ~DualWR() override = default;

  void assignArch(Value* archVal, int regId, bool invert = false) override;
};

#endif
