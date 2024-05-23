//=============================================================================
// FILE:
//    DualWR.cpp
//
// DESCRIPTION:
//    Assign an arch value to WR without using branch instructions and
//    additional registers
//
// License: MIT
//=============================================================================
#include <algorithm>
#include <string>
#include <ctime>
#include <cstdlib>
#include "Flexo.h"
#include "Utils.h"
#include "WeirdRegisters/DualWR.h"
#include "WeirdRegisters/Mappings/Mapping.h"

using namespace llvm;

// assign an arch value to WR
// This version avoids branch instructions (prevents BP to do weird things)
void DualWR::assignArch(Value* archVal, int regId, bool invert) {
  // ignore assignment on odd regIds
  if (regId & 1) return;

  IntegerType *I64Ty = builder.getInt64Ty();
  std::string regName = "Wire" + std::to_string(regId >> 1);
  uint64_t reg0Offset = (mapping->getRealId(regId) + firstIdx) * regOffset;
  uint64_t reg1Offset = (mapping->getRealId(regId + 1) + firstIdx) * regOffset;
  getCreateReg(regId);
  getCreateReg(regId + 1);

  // calculate accessOffset = archVal * (reg1Offset - reg0Offset) + reg0Offset
  // access: archVal ? reg1Offset : reg0Offset
  Value* reg0OffsetVal = builder.getInt64(reg0Offset);
  Value* reg1OffsetVal = builder.getInt64(reg1Offset);
  Value* offsetDiff = builder.getInt64(reg1Offset - reg0Offset);
  if (!mapping->compileTimeMap) {
    reg0OffsetVal = WRegOffsets[regId];
    reg1OffsetVal = WRegOffsets[regId + 1];
    offsetDiff = builder.CreateSub(reg1OffsetVal, reg0OffsetVal);
  }
  auto archVal64 = builder.CreateZExt(archVal, I64Ty);
  auto archValOffset = builder.CreateMul(archVal64, offsetDiff);
  auto accessOffset = builder.CreateAdd(archValOffset, reg0OffsetVal);

  // calculate flushOffset = archVal * (reg0Offset - reg1Offset) + reg1Offset
  // flush: archVal ? reg0Offset : reg1Offset
  Value* offsetDiffInv = builder.getInt64(reg0Offset - reg1Offset);
  if (!mapping->compileTimeMap) {
    offsetDiffInv = builder.CreateSub(reg0OffsetVal, reg1OffsetVal);
  }
  const auto archValOffsetInv = builder.CreateMul(archVal64, offsetDiffInv);
  const auto flushOffset = builder.CreateAdd(archValOffsetInv, reg1OffsetVal);

  // access accessOffset and flush flushOffset
  const auto accessIdx = getWRegGEP(accessOffset, "access." + regName);
  auto store = builder.CreateStore(builder.getInt8(0), accessIdx);
  store->setAlignment(align);
  const auto flushIdx = getWRegGEP(flushOffset, "flush." + regName);
  builder.CreateCall(
    flexo->getFunc("llvm.x86.sse2.clflush"), {flushIdx}
  );
}
