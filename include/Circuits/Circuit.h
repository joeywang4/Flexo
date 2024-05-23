//==============================================================================
// FILE:
//    Circuit.h
//
// DESCRIPTION:
//    Manage a circuit of the weird machine
//
// License: MIT
//==============================================================================
#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <string>
#include <vector>
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "Gate.h"
#include "Wire.h"
#include "Utils.h"

using namespace llvm;

typedef std::vector<Gate*>::iterator GateIterator;

class Circuit {
public:
  Circuit(): wireCount(0) {
    // add special constant 0/1 wires
    getOrCreateWire("$false");
    getOrCreateWire("$true");
  }
  ~Circuit() {
    for (auto wire: this->wires) {
      delete wire;
    }
    this->wires.clear();
    this->namedWires.clear();
    this->inputs.clear();
    this->outputs.clear();
    for (auto gate: this->gates) {
      delete gate;
    }
    this->gates.clear();
  }

  // create a circuit from files
  void parseBlif(StringRef filename);

  // wires related
  Wire* createWire();
  size_t getWireCount() { return wires.size(); }

  // input wires related
  size_t getInputCount() { return inputs.size(); }
  Wire* getInput(size_t idx) { return inputs[idx]; }
  StringRef getInputName(size_t idx) { return inputNames[idx]; }
  unsigned getInputOffset(size_t idx) { return inputOffsets[idx]; }
  int getInputIdx(Wire* wire) { return findIdx(&inputs, wire); }
  bool isInputWire(Wire* wire) { return getInputIdx(wire) != -1; }

  // output wires related
  size_t getOutputCount() { return outputs.size(); }
  Wire* getOutput(size_t idx) { return outputs[idx]; }
  StringRef getOutputName(size_t idx) { return outputNames[idx]; }
  unsigned getOutputOffset(size_t idx) { return outputOffsets[idx]; }
  int getOutputIdx(Wire* wire) { return findIdx(&outputs, wire); }
  bool isOutputWire(Wire* wire) { return getOutputIdx(wire) != -1; }

  // constant wires related
  Wire* getFalseWire() { return getWire("$false"); }
  Wire* getTrueWire() { return getWire("$true"); }
  bool isConstantWire(Wire* wire) { 
    return wire == getFalseWire() || wire == getTrueWire();
  }

  // gates related
  Gate* newGate(
    GateType type,
    std::vector<Wire*> inputWires,
    std::vector<Wire*> outputWires,
    unsigned truthTbl = 0
  );
  GateIterator begin() { return gates.begin(); }
  GateIterator end() { return gates.end(); }
  GateIterator insertGate(GateIterator it, Gate *gate) { 
    return gates.insert(it, gate); 
  }
  GateIterator eraseGate(GateIterator it);

  // debug
  std::string dump();
  bool verbose = false;

private:
  // wires related
  Wire* getWire(StringRef name);
  Wire* getOrCreateWire(StringRef name);
  Wire* addInputWire(StringRef name);
  void addOutput(Wire* wire);
  void replaceWire(Wire* oldWire, Wire* newWire);
  void eraseWire(Wire* wire);

  // add a gate to the circuit
  void addGate(
    GateType type,
    ArrayRef<StringRef> inputs,
    ArrayRef<StringRef> outputs,
    unsigned truthTbl = 0
  );
  void addGate(
    GateType type,
    std::vector<Wire*> inputWires,
    std::vector<Wire*> outputWires,
    unsigned truthTbl = 0
  );

  void sortGates();
  void insertAssign();
  void removeRedundantAssign();
  void removeUnusedInputs();

  // data members
  unsigned wireCount;
  StringMap<Wire*> namedWires;
  std::vector<Wire*> wires;
  std::vector<Wire*> inputs;
  std::vector<std::string> inputNames;
  std::vector<unsigned> inputOffsets;
  std::vector<Wire*> outputs;
  std::vector<std::string> outputNames;
  std::vector<unsigned> outputOffsets;
  std::vector<Gate*> gates;
  bool useTable = false;
};

#endif
