//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "break-struct-arguments"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <vector>

using namespace std;
using namespace llvm;

namespace {
  struct RemoveAssume : public BasicBlockPass {
    static char ID; // Pass identification, replacement for typeid
    RemoveAssume() : BasicBlockPass(ID) { }

    virtual bool runOnBasicBlock(BasicBlock &BB) {
      SmallVector<Instruction*, 8> Killed;

      for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E; ++I) {
        if (CallInst* Call = dyn_cast<CallInst>(&*I)) {
          Function* Fn = Call->getCalledFunction();
          if (!Fn) {
            continue;
          }

          if (Fn->getValueName()->getKey() == "llvm.assume") {
            Killed.push_back(Call);
          } 
        }
      }

      for (SmallVector<Instruction*, 8>::iterator I = Killed.begin(), E = Killed.end(); I != E; ++I) {
        (*I)->eraseFromParent();
      }
      return Killed.size() > 0;
    }
  };
}

char RemoveAssume::ID = 0;
static RegisterPass<RemoveAssume> X("remove-assume",
        "remove calls to llvm.assume");
