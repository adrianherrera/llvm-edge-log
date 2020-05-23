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

#include "llvm/IR/CallSite.h"
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
  PointerType *Int8PtrTy = Int8Ty->getPointerTo();
  IntegerType *Int32Ty = Type::getInt32Ty(C);

  auto LogEdgeF = M.getOrInsertFunction(
      kEdgeLogFuncName,
      FunctionType::get(Type::getVoidTy(C),
                        {Int8PtrTy, Int8PtrTy, Int32Ty, Int8Ty},
                        /* isVarArg */ false));

  for (auto &F : M) {
    StringRef FileName = M.getSourceFileName();

    if (F.isDeclaration()) {
      continue;
    }

    DISubprogram *FuncDI = F.getSubprogram();
    StringRef FuncName = FuncDI ? FuncDI->getName() : "";
    if (FuncName.empty()) {
      FuncName = F.getName();
    }

    assert(!FuncName.empty());

    // Create pointers to the file and function name strings
    BasicBlock::iterator IP = F.getEntryBlock().getFirstInsertionPt();
    IRBuilder<> IRB(&(*IP));
    Constant *File = IRB.CreateGlobalStringPtr(FileName);
    Constant *Func = IRB.CreateGlobalStringPtr(FuncName);

    for (auto &BB : F) {
      // Cache instructions
      SmallVector<Instruction *, 16> Insts;
      for (auto &I : BB) {
        Insts.push_back(&I);
      }

      // Log all edge transitions
      for (auto *I : Insts) {
        const DebugLoc &DbgLoc = I->getDebugLoc();
        ConstantInt *LineNo = ConstantInt::get(
            Int32Ty, DbgLoc ? DbgLoc.getLine() : -1, /* isSigned */ true);
        ConstantInt *EdgeTy = nullptr;

        if (isa<CallInst>(I) || isa<InvokeInst>(I)) {
          // Don't instrument LLVM intrinsics or ASan functions
          CallSite CS(I);
          if (auto *CalledF = CS.getCalledFunction()) {
            if (CalledF->isIntrinsic() ||
                CalledF->getName().startswith("__asan_")) {
              continue;
            }

            EdgeTy = ConstantInt::get(Int8Ty, EdgeType::EdgeDirectCall);
          } else {
            EdgeTy = ConstantInt::get(Int8Ty, EdgeType::EdgeIndirectCall);
          }
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

          EdgeTy = ConstantInt::get(Int8Ty, EdgeTyVal);
        }

        if (!EdgeTy) {
          continue;
        }

        IRB.SetInsertPoint(I);
        IRB.CreateCall(LogEdgeF, {File, Func, LineNo, EdgeTy});
      }
    }
  }

  return true;
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
