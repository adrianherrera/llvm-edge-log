//===-- EdgeLog.cpp - Log statistics of executed edges ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Log statistics of executed edges in the control-flow graph.
///
/// Unlike AFL, these are not "lossy" statistics (e.g., counts): they are exact,
/// so should not be used where performance is a requirement.
///
//===----------------------------------------------------------------------===//

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Instrumentation.h"

using namespace llvm;

#define DEBUG_TYPE "edge-log"

namespace {

static const char *const kEdgeLogFuncName = "__edge_log";

class EdgeLog : public ModulePass {
public:
  static char ID;
  EdgeLog() : ModulePass(ID) {}

  bool runOnModule(Module &) override;
};

} // anonymous namespace

char EdgeLog::ID = 0;

bool EdgeLog::runOnModule(Module &M) {
  LLVMContext &C = M.getContext();
  bool Modifed = false;

  auto LogEdgeF = M.getOrInsertFunction(
      kEdgeLogFuncName,
      FunctionType::get(Type::getVoidTy(C), /* isVarArg */ false));

  for (auto &F : M) {
    for (auto &BB : F) {
      BasicBlock::iterator IP = BB.getFirstInsertionPt();
      IRBuilder<> IRB(&*IP);
      IRB.CreateCall(LogEdgeF);

      Modifed = true;
    }
  }

  return Modifed;
}

static RegisterPass<EdgeLog> X("edge-log", "Executed edge statistics", false,
                               false);

static void registerEdgeLog(const PassManagerBuilder &,
                            legacy::PassManagerBase &PM) {
  PM.add(new EdgeLog());
}

static RegisterStandardPasses
    RegisterEdgeLog(PassManagerBuilder::EP_OptimizerLast, registerEdgeLog);

static RegisterStandardPasses
    RegisterEdgeLog0(PassManagerBuilder::EP_EnabledOnOptLevel0,
                     registerEdgeLog);
