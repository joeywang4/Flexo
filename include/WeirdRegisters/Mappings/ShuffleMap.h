//=============================================================================
// FILE:
//    ShuffleMap.h
//
// DESCRIPTION:
//    Randomly shuffle the map everytime the circuit is executed.
//
// License: MIT
//=============================================================================
#ifndef SHUFFLE_MAP_H
#define SHUFFLE_MAP_H

#include "WeirdRegisters/Mappings/Mapping.h"

using namespace llvm;

class ShuffleMap: public Mapping
{
public:
  ShuffleMap(Flexo* flexo, IRBuilder<>& builder)
  : Mapping(flexo, builder) {
    compileTimeMap = false;
  };
  ~ShuffleMap() override = default;

  void init(int count) override;
  Value* getRuntimeRealId(int regId) override;

protected:
  Value* regIdMap;
};

#endif
