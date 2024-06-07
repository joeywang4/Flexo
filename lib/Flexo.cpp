//========================================================================
// FILE:
//    Flexo.cpp
//
// DESCRIPTION:
//    For each function that contains `_gate` in its name, Flexo 
//    converts the function to a weird machine.
//
// USAGE:
//    1. Legacy pass manager:
//      $ opt -load <BUILD_DIR>/lib/libFlexo.so `\`
//        --legacy-inject-func-call <bitcode-file>
//    2. New pass maanger:
//      $ opt -load-pass-plugin <BUILD_DIR>/lib/libFlexo.so `\`
//        -passes=-"inject-func-call" <bitcode-file>
//
// License: MIT
//========================================================================
#include <vector>
#include <filesystem>
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "kernel/yosys.h"
#include "Flexo.h"
#include "WeirdRegisters/NoBranchWR.h"
#include "WeirdRegisters/DualWR.h"
#include "WeirdMachines/DualRetWM.h"
#include "WeirdMachines/WeirdMachine.h"
#include "Circuits/Circuit.h"
#include "Circuits/IRParser.h"
#include "Utils.h"

using namespace llvm;

#define DEBUG_TYPE "create-WMs"
const char name[] = DEBUG_TYPE;

//-----------------------------------------------------------------------------
// Flexo implementation
//-----------------------------------------------------------------------------

// public interface - get a function from function map
FunctionCallee Flexo::getFunc(StringRef funcName) {
  assert(this->FuncMap.contains(funcName));
  return this->FuncMap[funcName];
}

// public interface - add a function to module and function map
FunctionCallee Flexo::addFunc(StringRef funcName, FunctionType* funcTy){
  FunctionCallee func = this->module->getOrInsertFunction(
    funcName, funcTy
  );
  this->FuncMap[funcName] = func;
  return func;
}

// public interface - define module asm
void Flexo::setModuleAsm(StringRef moduleAsm) {
  this->module->setModuleInlineAsm(moduleAsm);
}

// remove existing IR code in a function and create a new entry block
BasicBlock* Flexo::removeExistingIR(Function* F) {
  std::vector<BasicBlock*> BasicBlocks;

  // get all basic blocks before erasing them
  for (auto Blk = F->begin(), End = F->end(); Blk != End; ++Blk) {
    for (auto &Inst : *Blk) {
      // remove output variable usages
      Inst.replaceAllUsesWith(UndefValue::get(Inst.getType()));
    }
    BasicBlocks.push_back(&*Blk);
  }

  // erase basic blocks
  for (auto Blk: BasicBlocks) Blk->eraseFromParent();
  BasicBlocks.clear();

  // create a new entry block
  LLVMContext &CTX = this->module->getContext();
  BasicBlock *EntryBlk = BasicBlock::Create(CTX, "entry", F);
  return EntryBlk;
}

// Translate a function to WM
void Flexo::translateFunc(Function* F, StringRef circuitFile) {
  // delete the original IR code
  IRBuilder<> builder(removeExistingIR(F));

  // Use stack memory to alloc weird registers
  WeirdRegisters* WR;
  std::string WR_TYPE = get_str_env("WR_TYPE", "Dual");

  if (this->verbose) {
    outs() << " WR_TYPE is " << WR_TYPE << "\n";
  }

  if (WR_TYPE == "Baseline") {
    WR = new WeirdRegisters(this, builder, 16);
  } 
  else if (WR_TYPE == "NoBranch") {
    WR = new NoBranchWR(this, builder, 16);
  }
  else if (WR_TYPE == "Dual") {
    WR = new DualWR(this, builder, 16);
  }
  else {
    outs() << "Unsupported weird register type: " << WR_TYPE << "\n";
    assert(false);
  }

  Value* errorDetected = this->wm->createCircuit(F, builder, circuitFile, WR);

  // clear weird registers
  WR->cleanup();
  delete WR;

  // return error detection result if not returning void
  if (F->getReturnType()->isVoidTy()) {
    builder.CreateRetVoid();
  }
  else {
    builder.CreateRet(errorDetected);
  }
}

// module pass main
void Flexo::runOnModule(Module &M) {
  this->module = &M;

  this->verbose = get_str_env("WM_VERBOSE", "") == "true";

  // define the weird machine
  int RET_WM_DIV_ROUNDS = get_int_env("RET_WM_DIV_ROUNDS", 4);
  int RET_WM_DIV_SIZE = get_int_env("RET_WM_DIV_SIZE", 16);
  int RET_WM_JMP_SIZE = get_int_env("RET_WM_JMP_SIZE", 512);
  int DUAL_WM_MAX_INPUT = get_int_env("DUAL_WM_MAX_INPUT", 4);

  WeirdMachine *wm = new DualRetWM(
    this, RET_WM_DIV_ROUNDS, RET_WM_DIV_SIZE, RET_WM_JMP_SIZE, DUAL_WM_MAX_INPUT
  );

  wm->create();
  wm->verbose = this->verbose;
  this->wm = wm;

  // use yosys to synthesize and optimize circuits
  string tmp_path = get_str_env("TMP_PATH", "/tmp");
  ofstream yosys_log;
  open_file(yosys_log, tmp_path + "/yosys.log");
  Yosys::log_streams.push_back(&yosys_log);
  Yosys::log_error_stderr = true;
  Yosys::yosys_setup();

  string keyword = get_str_env("WM_KEYWORD", "__weird__");
  string circuitFile = get_str_env("WM_CIRCUIT_FILE", "");
  bool useCircuitFile = circuitFile != "";

  for (Function &F : M) {
    if (F.isDeclaration() || F.getName().find(keyword) == StringRef::npos) continue;

    outs() << "Found weird function: " << F.getName() << "\n";

    string filename = tmp_path + "/" + F.getName().str();
    if (useCircuitFile) {
      filesystem::copy_file(
        circuitFile, filename + ".v", filesystem::copy_options::overwrite_existing
      );
    }
    else {
      IRParser parser(&F, this->verbose);
      string parsedCirc = parser.dump(F.getName().str());
      write_file(filename + ".v", parsedCirc);
    }

    vector<string> yosysPasses;
    wm->setYosysPasses(yosysPasses, filename);
    yosys_log << "=== Circuit: " << F.getName().str() << " ===\n\n";
    Yosys::RTLIL::Design *newDesign = new Yosys::RTLIL::Design;
    for (string pass : yosysPasses) Yosys::run_pass(pass, newDesign);
    delete newDesign;
    yosys_log << "=== End circuit: " << F.getName().str() << " ===\n\n";

    // write the optimized circuit to the function body
    this->translateFunc(&F, filename);
  }

  // clean up
  Yosys::yosys_shutdown();
  yosys_log.close();
  this->module = nullptr;
  delete wm;
}

PreservedAnalyses Flexo::run(
  llvm::Module &M, llvm::ModuleAnalysisManager &
) {
  runOnModule(M);

  return llvm::PreservedAnalyses::none();
}

bool LegacyFlexo::runOnModule(llvm::Module &M) {
  Impl.runOnModule(M);

  return true;
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getFlexoPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION,
    name,
    LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](
          StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>
        ) {
          if (Name == name) {
            MPM.addPass(Flexo());
            return true;
          }
          return false;
        }
      );
    }
  };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getFlexoPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacyFlexo::ID = 0;

// Register the pass - required for (among others) opt
static RegisterPass<LegacyFlexo>
    X(/*PassArg=*/"legacy-inject-func-call", /*Name=*/"LegacyFlexo",
      /*CFGOnly=*/false, /*is_analysis=*/false);
