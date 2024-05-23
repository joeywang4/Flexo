//=============================================================================
// FILE:
//    ShuffleMap.cpp
//
// DESCRIPTION:
//    Randomly shuffle the map everytime the circuit is executed.
//
// License: MIT
//=============================================================================
#include "Flexo.h"
#include "WeirdRegisters/Mappings/ShuffleMap.h"

#define MULT 1664525
#define ADD 1013904223

// The init code is based on the following snippet:
/*
int arr[count];
unsigned seed = rand();

for (int i = 0; i < count; ++i) {
  arr[i] = i;
}

for (int i = 0; i < count - 1; ++i) {
  int j = seed % (count - i) + i;
  seed = seed * MULT + ADD;
  int tmp = arr[i];
  arr[i] = arr[j];
  arr[j] = tmp;
}
*/
void ShuffleMap::init(int count) {
  Mapping::init(count);
  IntegerType *I32Ty = builder.getInt32Ty();
  ArrayType *regMapTy = ArrayType::get(I32Ty, count);
  regIdMap = builder.CreateAlloca(regMapTy, nullptr, "regMap");
  Value* seed = builder.CreateAlloca(I32Ty, nullptr, "seed");
  auto rand = builder.CreateCall(
    flexo->getFunc("rand"), std::nullopt, "rand"
  );
  builder.CreateStore(rand, seed);

  LLVMContext &CTX = builder.getContext();
  BasicBlock* entryBlk = builder.GetInsertBlock();
  Function* F = entryBlk->getParent();
  // init regIdMap
  BasicBlock* initBlk = BasicBlock::Create(CTX, "shuffle.init", F);
  // shuffle regIdMap
  BasicBlock* swapBlk = BasicBlock::Create(CTX, "shuffle.swap", F);
  // end shuffling
  BasicBlock* endBlk = BasicBlock::Create(CTX, "shuffle.end", F);
  builder.CreateBr(initBlk);
  builder.SetInsertPoint(initBlk);

  // init regIdMap to { 0, 1, 2, 3, ... }
  PHINode *initIdx = builder.CreatePHI(I32Ty, 2, "shuffle.init.i");
  initIdx->addIncoming(builder.getInt32(0), entryBlk);
  auto regIdMapIdx = builder.CreateGEP(
    I32Ty, regIdMap, {initIdx}, "regMap.init.idx", /*IsInBounds*/true
  );
  builder.CreateStore(initIdx, regIdMapIdx);
  auto nextIdx = builder.CreateAdd(
    initIdx, builder.getInt32(1), "shuffle.init.next.i"
  );
  initIdx->addIncoming(nextIdx, initBlk);
  auto endInitCond = builder.CreateICmpEQ(
    nextIdx, builder.getInt32(count), "shuffle.init.end"
  );
  builder.CreateCondBr(endInitCond, swapBlk, initBlk);
  builder.SetInsertPoint(swapBlk);

  // shuffle regIdMap (swap elements inside regIdMap)
  PHINode *swapIdx = builder.CreatePHI(I32Ty, 2, "shuffle.swap.i");
  swapIdx->addIncoming(builder.getInt32(0), initBlk);
  auto seedVal = builder.CreateLoad(I32Ty, seed, "seed.val");
  auto bound = builder.CreateSub(
    builder.getInt32(count), swapIdx, "shuffle.swap.bound"
  );
  auto bounded = builder.CreateURem(seedVal, bound);
  auto randIdx = builder.CreateAdd(swapIdx, bounded, "shuffle.swap.j");
  auto seedNext = builder.CreateAdd(
    builder.CreateMul(seedVal, builder.getInt32(MULT)),
    builder.getInt32(ADD), "seed.next"
  );
  builder.CreateStore(seedNext, seed);
  auto leftIdx = builder.CreateGEP(
    I32Ty, regIdMap, {swapIdx}, "shuffle.swap.left.idx", /*IsInBounds*/true
  );
  auto rightIdx = builder.CreateGEP(
    I32Ty, regIdMap, {randIdx}, "shuffle.swap.right.idx", /*IsInBounds*/true
  );
  auto leftVal = builder.CreateLoad(I32Ty, leftIdx, "shuffle.swap.left.val");
  auto rightVal = builder.CreateLoad(I32Ty, rightIdx, "shuffle.swap.right.val");
  builder.CreateStore(leftVal, rightIdx);
  builder.CreateStore(rightVal, leftIdx);
  auto nextSwapIdx = builder.CreateAdd(
    swapIdx, builder.getInt32(1), "shuffle.swap.next.i"
  );
  swapIdx->addIncoming(nextSwapIdx, swapBlk);
  auto endSwapCond = builder.CreateICmpEQ(
    nextSwapIdx, builder.getInt32(count - 1), "shuffle.swap.end"
  );
  builder.CreateCondBr(endSwapCond, endBlk, swapBlk);
  builder.SetInsertPoint(endBlk);
}

Value* ShuffleMap::getRuntimeRealId(int regId) {
  IntegerType *I32Ty = builder.getInt32Ty();

  auto regIdx = builder.CreateGEP(
    I32Ty, regIdMap, {builder.getInt64(regId)},
    "regMap." + std::to_string(regId), /*IsInBounds*/true
  );
  return builder.CreateLoad(I32Ty, regIdx, "rand.reg." + std::to_string(regId));
}
