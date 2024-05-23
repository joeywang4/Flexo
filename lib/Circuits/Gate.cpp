//========================================================================
// FILE:
//    Gate.cpp
//
// DESCRIPTION:
//    Manage a gate of a circuit
//
// License: MIT
//========================================================================
#include <string>
#include "llvm/IR/Instructions.h"
#include "Circuits/Gate.h"

using namespace llvm;

GateType fromOpcode(unsigned opcode) {
  switch (opcode) {
    case Instruction::And: return GateType::AND;
    case Instruction::Or: return GateType::OR;
    case Instruction::Xor: return GateType::XOR;
    case Instruction::FNeg: return GateType::NOT;
    case Instruction::Load: return GateType::ASSIGN;
    case Instruction::Select: return GateType::MUX;
    default: 
      assert(false && "Unknown gate type");
      return GateType::AND;
  }
}

std::string toString(GateType type) {
  switch(type) {
    case GateType::AND: return "AND";
    case GateType::OR: return "OR";
    case GateType::NOT: return "NOT";
    case GateType::ASSIGN: return "ASSIGN";
    case GateType::XOR: return "XOR";
    case GateType::MUX: return "MUX";
    case GateType::NAND: return "NAND";
    case GateType::TABLE: return "TABLE";
    default: return "UNKNOWN";
  }
}
