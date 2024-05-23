//==============================================================================
// FILE:
//    Gate.h
//
// DESCRIPTION:
//    A gate in the circuit
//
// License: MIT
//==============================================================================
#ifndef GATE_H
#define GATE_H

#include <vector>
#include <string>
#include "llvm/ADT/ArrayRef.h"

class Wire;

enum class GateType { AND, OR, NOT, ASSIGN, XOR, MUX, NAND, TABLE };

GateType fromOpcode(unsigned opcode);
std::string toString(GateType type);

class Gate {
public:
  Gate(
    GateType type,
    llvm::ArrayRef<Wire*> inputs = std::nullopt,
    llvm::ArrayRef<Wire*> outputs = std::nullopt,
    unsigned truthTbl = 0
  )
  : type(type), truthTbl(truthTbl) {
    const size_t inputSize = inputs.size();
    this->inputs.reserve(inputSize);
    for (size_t i = 0; i < inputSize; ++i)
      this->inputs.push_back(inputs[i]);

    const size_t outputSize = outputs.size();
    this->outputs.reserve(outputSize);
    for (size_t i = 0; i < outputSize; ++i)
      this->outputs.push_back(outputs[i]);
  }

  ~Gate() {
    inputs.clear();
    outputs.clear();
  }

  GateType getType() { return type; }
  unsigned getTruthTbl() { return truthTbl; }
  
  std::vector<Wire*> inputs;
  std::vector<Wire*> outputs;

private:
  GateType type;
  unsigned truthTbl;
};

#endif
