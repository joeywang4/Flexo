//==============================================================================
// FILE:
//    IRParser.h
//
// DESCRIPTION:
//    Parse LLVM IR and translate to a verilog module
//
// License: MIT
//==============================================================================
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Intrinsics.h"
#include "Circuits/IRParser.h"

string getOperator(unsigned opcode) {
  switch (opcode)
  {
  case Instruction::And:  return " & ";
  case Instruction::Or:   return " | ";
  case Instruction::Xor:  return " ^ ";
  case Instruction::Add:  return " + ";
  case Instruction::Sub:  return " - ";
  case Instruction::Mul:  return " * ";
  case Instruction::SDiv: return " / ";
  case Instruction::SRem: return " % ";
  case Instruction::URem: return " % ";
  case Instruction::Shl:  return " << ";
  case Instruction::LShr: return " >> ";
  case Instruction::AShr: return " >>> ";
  default:
    outs() << "Unsupported opcode: " << opcode << "\n";
    assert(false);
    return "";
  }
}

string valToName(Value* val) {
  string name;
  if (val->getName().empty()) {
    // use ptr address as value name
    name = "_" + to_string((uintptr_t)val);
  }
  else {
    name = val->getName().str();
  }

  name = "_" + name;
  return name;
}

// output verilog code
string IRParser::dump(string name) {
  string code = "module " + name + "(\n";
  bool first = true;
  for (auto arg: args) {
    if (!arg->isInput && !arg->isOutput) continue;
    if (!first) code += ",\n";
    code += "  ";
    code += arg->isInput ? "input " : "output ";
    if (arg->width > 1) {
      code += "[" + to_string(arg->width - 1) + ":0] ";
    }
    code += arg->name;
    first = false;
  }
  code += "\n);\n";

  for (auto wire: wires) {
    if (wire->isArg) continue;
    code += "  wire ";
    if (wire->width > 1) {
      code += "[" + to_string(wire->width - 1) + ":0] ";
    }
    code += wire->name + ";\n";
  }
  code += "\n";

  for (auto line: lines) {
    code += "  assign " + line + ";\n";
  }

  code += "endmodule\n";
  return code;
}

// parse a verilog module from LLVM IR
IRParser::IRParser(Function* F, bool verbose) : verbose(verbose) {
  // parse function arguments
  for (auto arg = F->arg_begin(), IE = F->arg_end(); arg != IE; ++arg) {
    string name = valToName(arg);
    if (name.rfind("_error_", 0) == 0) continue; // ignore error argument

    bus* bus = new struct bus;
    bus->name = name;
    bus->width = arg->getType()->isIntegerTy() ? arg->getType()->getIntegerBitWidth() : 0;
    bus->isArg = true;
    bus->isInput = false;
    bus->isOutput = false;
    busMap[bus->name] = bus;
    args.push_back(bus);
    wires.push_back(bus);
    if (verbose) {
      outs() << "Add arg: " << bus->name << "(" << bus->width << ")\n";
    }
  }

  // parse a block (assume there is only one block)
  BasicBlock& blk = F->getEntryBlock();
  for (auto &Inst : blk) {
    BinaryOperator *BinOp = dyn_cast<BinaryOperator>(&Inst);
    if (BinOp) {
      parseBinaryOp(BinOp);
      continue;
    }

    // parse a MUX gate
    if (SelectInst* selectInst = dyn_cast<SelectInst>(&Inst)) {
      parseSelect(selectInst);
      continue;
    }

    // parse cast instructions
    if (CastInst* castInst = dyn_cast<CastInst>(&Inst)) {
      parseCast(castInst);
      continue;
    }

    // parse GEP instructions
    if (GetElementPtrInst* gepInst = dyn_cast<GetElementPtrInst>(&Inst)) {
      parseGEP(gepInst);
      continue;
    }

    // parse load instructions
    if (LoadInst* loadInst = dyn_cast<LoadInst>(&Inst)) {
      parseLoad(loadInst);
      continue;
    }

    // parse store instructions
    if (StoreInst* storeInst = dyn_cast<StoreInst>(&Inst)) {
      parseStore(storeInst);
      continue;
    }

    // parse integer comparison instructions
    if (ICmpInst* icmpInst = dyn_cast<ICmpInst>(&Inst)) {
      parseIcmp(icmpInst);
      continue;
    }

    // parse call instructions
    if (CallInst* callInst = dyn_cast<CallInst>(&Inst)) {
      parseCall(callInst);
      continue;
    }

    // must be return if Inst is not handled by previos parsers
    ReturnInst *ret = dyn_cast<ReturnInst>(&Inst);
    if(!ret && verbose) {
      outs() << "Unsupported instruction: " << Inst << "\n";
    }
    assert(ret);
  }
}

// parse binary operation such as AND, OR, ...
void IRParser::parseBinaryOp(BinaryOperator* BinOp) {
  Value* in1Val = BinOp->getOperand(0);
  Value* in2Val = BinOp->getOperand(1);

  unsigned Opcode = BinOp->getOpcode();
  string in1 = valToBusName(in1Val);
  string in2 = valToBusName(in2Val);
  string out = valToBusName(BinOp, true);

  lines.push_back(out + " = " + in1 + getOperator(Opcode) + in2);
}

// handle a select inst (MUX gate)
void IRParser::parseSelect(SelectInst* selectInst) {
  Value* condVal = selectInst->getCondition();
  Value* trueVal = selectInst->getTrueValue();
  Value* falseVal = selectInst->getFalseValue();

  string cond = valToBusName(condVal);
  string trueName = valToBusName(trueVal);
  string falseName = valToBusName(falseVal);
  string out = valToBusName(selectInst, true);

  lines.push_back(out + " = " + cond + " ? " + trueName + " : " + falseName);
}

// handle zext and trunc instructions
void IRParser::parseCast(CastInst* castInst) {
  Value* inVal = castInst->getOperand(0);
  bus* inBus = getBus(valToName(inVal), inVal->getType()->getIntegerBitWidth());
  string in = inBus->name;
  bus* outBus = getOrCreateBus(
    valToName(castInst), castInst->getType()->getIntegerBitWidth()
  );
  string out = outBus->name;
  string range = "[0]";

  if (castInst->getOpcode() == Instruction::ZExt) {
    if (inBus->width > 1) {
      range = "[" + to_string(inBus->width - 1) + ":0]";
    }
    lines.push_back(out + range + " = " + in);

    string zeroRange = "[" + to_string(outBus->width - 1) + ":";
    zeroRange += to_string(inBus->width) +"]";
    lines.push_back(out + zeroRange + " = 0");
  }
  else if (castInst->getOpcode() == Instruction::Trunc) {
    if (outBus->width > 1) {
      range = "[" + to_string(outBus->width - 1) + ":0]";
    }
    lines.push_back(out + " = " + in + range);
  }
  else {
    if (verbose) {
      outs() << "Unsupported cast instruction: " << *castInst << "\n";
    }
    assert(false);
  }
}

// handle ICmp instructions
void IRParser::parseIcmp(ICmpInst* icmpInst) {
  Value* in1Val = icmpInst->getOperand(0);
  Value* in2Val = icmpInst->getOperand(1);

  string in1Name = valToBusName(in1Val);
  string in2Name = valToBusName(in2Val);
  string outName = valToBusName(icmpInst, true);

  string comparison = "";
  auto predicate = icmpInst->getPredicate();
  if (icmpInst->isEquality(predicate)) {
    if (icmpInst->isTrueWhenEqual()) {
      comparison = " == ";
    }
    else {
      comparison = " != ";
    }
  }
  else if (icmpInst->isGE(predicate)) {
    comparison = " >= ";
  }
  else if (icmpInst->isGT(predicate)) {
    comparison = " > ";
  }
  else if (icmpInst->isLE(predicate)) {
    comparison = " <= ";
  }
  else if (icmpInst->isLT(predicate)) {
    comparison = " < ";
  }
  else {
    outs() << "Unsupported predicate: " << predicate << "\n";
    assert(false);
  }

  lines.push_back(outName + " = " + in1Name + comparison + in2Name);
}

// handle GEP instructions
void IRParser::parseGEP(GetElementPtrInst* gepInst) {
  assert(gepInst->hasAllConstantIndices());

  Value* ptr = gepInst->getPointerOperand();
  string ptrName = valToName(ptr);
  unsigned width = gepInst->getSourceElementType()->getIntegerBitWidth();
  Value* idxVal = gepInst->idx_begin()->get();
  ConstantInt *constInt = dyn_cast<ConstantInt>(idxVal);
  unsigned offset = width * constInt->getZExtValue();

  bus* inBus = busMap[ptrName];
  if (inBus->width < offset + width) inBus->width = offset + width;
  gepMap[valToName(gepInst)] = make_pair(inBus, offset);
  if (verbose) {
    outs() << "Add GEP: " << valToName(gepInst) << "(" << inBus->name << ", " << width << ", " << offset << ")\n";
  }
}

// handle load instructions
void IRParser::parseLoad(LoadInst* loadInst) {
  Value* ptr = loadInst->getPointerOperand();
  string ptrName = valToName(ptr);
  unsigned width = loadInst->getType()->getIntegerBitWidth();
  bus* inBus;
  unsigned offset = 0;
  if (gepMap.count(ptrName) > 0) {
    auto got = gepMap[ptrName];
    inBus = got.first;
    offset = got.second;
  }
  else {
    inBus = getBus(ptrName, width);
  }

  string in = inBus->name;
  bus* outBus = getOrCreateBus(valToName(loadInst), width);
  string out = outBus->name;
  string range = "[" + to_string(offset + width - 1) + ":" + to_string(offset) + "]";

  lines.push_back(out + " = " + in + range);
}

// handle store instructions
void IRParser::parseStore(StoreInst* storeInst) {
  Value* val = storeInst->getValueOperand();
  Value* ptr = storeInst->getPointerOperand();
  string valName = valToBusName(val);
  string ptrName = valToName(ptr);
  unsigned width = val->getType()->getIntegerBitWidth();
  bus* outBus;
  unsigned offset = 0;

  if (gepMap.count(ptrName) > 0) {
    auto got = gepMap[ptrName];
    outBus = got.first;
    offset = got.second;
  }
  else {
    outBus = getOrCreateBus(ptrName, width);
  }

  string range = "[" + to_string(offset + width - 1) + ":" + to_string(offset) + "]";

  lines.push_back(outBus->name + range + " = " + valName);
}

// handle call instructions (built-in intrinsics)
void IRParser::parseCall(CallInst* callInst) {
  Function* callee = callInst->getCalledFunction();
  assert(callee->isIntrinsic());
  auto id = callee->getIntrinsicID();

  switch(id) {
  case Intrinsic::bitreverse:
  {
    Value* inVal = callInst->getOperand(0);
    string in = valToBusName(inVal);
    string out = valToBusName(callInst, true);
    unsigned width = inVal->getType()->getIntegerBitWidth();

    out += " = {";
    for (unsigned i = 0; i < width; i++) {
      out += in + "[" + to_string(i) + "]";
      if (i < width - 1) out += ", ";
    }
    lines.push_back(out + "}");
    break;
  }
  case Intrinsic::fshl:
  case Intrinsic::fshr:
  {
    Value* inVal1 = callInst->getOperand(0);
    Value* inVal2 = callInst->getOperand(1);
    Value* inVal3 = callInst->getOperand(2);
    string in1 = valToBusName(inVal1);
    string in2 = valToBusName(inVal2);
    string in3 = valToBusName(inVal3);
    string out = valToBusName(callInst, true);
    unsigned width = inVal1->getType()->getIntegerBitWidth();

    out += " = ({ " + in1 + ", " + in2 + " }";
    out += (id == Intrinsic::fshl) ? " << " : " >> ";
    out += "(" + in3 + " % " + to_string(width) + "))";
    out += (id == Intrinsic::fshl) ? " >> " : "[";
    out += to_string(width);
    out += (id == Intrinsic::fshl) ? "" : "-1:0]";
    lines.push_back(out);
    break;
  }
  case Intrinsic::bswap:
  {
    Value* inVal = callInst->getOperand(0);
    string in = valToBusName(inVal);
    string out = valToBusName(callInst, true);
    unsigned width = inVal->getType()->getIntegerBitWidth();

    out += " = {";
    for (unsigned i = 0; i < width; i += 8) {
      out += in + "[" + to_string(i + 7) + ":" + to_string(i) + "]";
      if (i < width - 8) out += ", ";
    }
    lines.push_back(out + "}");
    break;
  }
  default:
    outs() << "Unsupported intrinsic: " << callee->getName() << "\n";
    assert(false);
  }
}

// get bus name from value or create a constant number
string IRParser::valToBusName(Value* val, bool isOutput) {
  if (!isOutput && dyn_cast<ConstantInt>(val)) {
    ConstantInt *constInt = dyn_cast<ConstantInt>(val);
    return to_string(constInt->getZExtValue());
  }
  else {
    string name = valToName(val);
    unsigned width = val->getType()->getIntegerBitWidth();
    if (isOutput) return getOrCreateBus(name, width)->name;
    else return getBus(name, width)->name;
  }
}

// ensure bus exist and adjust width and input type
bus* IRParser::getBus(string name, unsigned width) {
  bus* bus = busMap[name];
  assert(bus != nullptr);

  if (bus->isArg && !bus->isInput) bus->isInput = true;
  if (bus->width < width) bus->width = width;

  return bus;
}

// create a bus if not exist and adjust width and output type
bus* IRParser::getOrCreateBus(string name, unsigned width) {
  bus* bus = busMap[name];

  if (bus == nullptr) {
    bus = new struct bus;
    bus->name = name;
    bus->width = width;
    bus->isArg = false;
    bus->isInput = false;
    bus->isOutput = false;
    busMap[name] = bus;
    wires.push_back(bus);
    if (verbose) {
      outs() << "Add bus: " << bus->name << "(" << bus->width << ")\n";
    }
  }
  else {
    if (bus->width < width) bus->width = width;
    if (bus->isArg && !bus->isOutput) bus->isOutput = true;
  }

  return bus;
}
