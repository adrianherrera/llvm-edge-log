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

#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Instrumentation.h"

#include "edge-log.h"

using namespace llvm;

#define DEBUG_TYPE "edge-log"

namespace {

static const char *const kEdgeTypeName = "__edge_type";
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
  IntegerType *Int8Ty = Type::getInt8Ty(C);

  GlobalVariable *EdgeTy = new GlobalVariable(
      M, Int8Ty, false, GlobalValue::ExternalLinkage, 0, kEdgeTypeName, 0,
      GlobalVariable::GeneralDynamicTLSModel, 0, false);
  auto LogEdgeF = M.getOrInsertFunction(
      kEdgeLogFuncName,
      FunctionType::get(Type::getVoidTy(C), {Int8Ty->getPointerTo()},
                        /* isVarArg */ false));

  for (auto &F : M) {
    if (F.isDeclaration()) {
      continue;
    }

    DISubprogram *FuncDI = F.getSubprogram();
    StringRef FuncName = FuncDI ? FuncDI->getName() : F.getName();

    for (auto &BB : F) {
      // Cache instructions
      SmallVector<Instruction *, 16> Insts;
      for (auto &I : BB) {
        Insts.push_back(&I);
      }

      // Log the edge at the start of each basic block
      BasicBlock::iterator IP = BB.getFirstInsertionPt();
      IRBuilder<> IRB(&(*IP));
      IRB.CreateCall(LogEdgeF, {IRB.CreateGlobalStringPtr(FuncName)});

      // Before every possible edge transition, set the edge type
      for (auto *I : Insts) {
        if (isa<CallInst>(I) || isa<InvokeInst>(I)) {
          IRB.SetInsertPoint(I);
          IRB.CreateStore(ConstantInt::get(Int8Ty, EdgeType::EdgeCall), EdgeTy);
        } else if (I->isTerminator()) {
          EdgeType EdgeTyVal = EdgeType::EdgeUnknown;

          switch (I->getOpcode()) {
          case Instruction::Br:
            EdgeTyVal = cast<BranchInst>(I)->isConditional()
                            ? EdgeType::EdgeCondBr
                            : EdgeType::EdgeUncondBr;
            break;
          case Instruction::Switch:
            EdgeTyVal = EdgeType::EdgeSwitch;
            break;
          case Instruction::Ret:
            EdgeTyVal = EdgeType::EdgeRet;
            break;
          case Instruction::Unreachable:
            EdgeTyVal = EdgeType::EdgeUnreachable;
            break;
          }

          IRB.SetInsertPoint(I);
          IRB.CreateStore(ConstantInt::get(Int8Ty, EdgeTyVal), EdgeTy);
        }
      }
    }
  }

  return true;
}

static RegisterPass<EdgeLog> X("edge-log", "Executed edge statistics",
                                 false, false);

static void registerEdgeLog(const PassManagerBuilder &,
                              legacy::PassManagerBase &PM) {
  PM.add(new EdgeLog());
}

static RegisterStandardPasses
    RegisterEdgeLog(PassManagerBuilder::EP_OptimizerLast, registerEdgeLog);

static RegisterStandardPasses
    RegisterEdgeLog0(PassManagerBuilder::EP_EnabledOnOptLevel0,
                       registerEdgeLog);
