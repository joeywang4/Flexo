//==============================================================================
// FILE:
//    Flexo.h
//
// DESCRIPTION:
//    Declares the Flexo pass for the new and the legacy pass managers.
//
// License: MIT
//==============================================================================
#ifndef LLVM_FLEXO_H
#define LLVM_FLEXO_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

class WeirdMachine;

using namespace llvm;

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
class Flexo : public PassInfoMixin<Flexo> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
  void runOnModule(Module &M);

  // Without isRequired returning true, this pass will be skipped for functions
  // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
  // all functions with optnone.
  static bool isRequired() { return true; }

  LLVMContext& getContext() { return this->module->getContext(); }
  FunctionCallee getFunc(StringRef funcName);
  FunctionCallee addFunc(StringRef funcName, FunctionType* funcTy);  
  void setModuleAsm(StringRef moduleAsm);

  // debug  
  bool verbose = false;

private:
  void translateFunc(Function* F, StringRef circuitFile);
  BasicBlock* removeExistingIR(Function* F);

  Module* module;
  StringMap<FunctionCallee> FuncMap;
  bool randomizeWR = true;
  WeirdMachine* wm;
};

//------------------------------------------------------------------------------
// Legacy PM interface
//------------------------------------------------------------------------------
class LegacyFlexo : public ModulePass {
public:
  static char ID;
  LegacyFlexo() : ModulePass(ID) {}
  bool runOnModule(Module &M) override;

  Flexo Impl;
};

#endif
