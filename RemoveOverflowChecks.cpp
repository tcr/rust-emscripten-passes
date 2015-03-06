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
  struct RemoveOverflowChecks : public BasicBlockPass {
    static char ID; // Pass identification, replacement for typeid
    RemoveOverflowChecks() : BasicBlockPass(ID) { }

    virtual bool runOnBasicBlock(BasicBlock &BB) {
      SmallVector<Instruction*, 8> Killed;

      for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E; ++I) {
        if (CallInst* Call = dyn_cast<CallInst>(&*I)) {
          Function* Fn = Call->getCalledFunction();
          if (!Fn || !Fn->isIntrinsic()) {
            continue;
          }

          Instruction::BinaryOps Op;
          switch (Fn->getIntrinsicID()) {
            case Intrinsic::sadd_with_overflow:
              Op = Instruction::Add;
              break;
            case Intrinsic::ssub_with_overflow:
              Op = Instruction::Sub;
              break;
            case Intrinsic::smul_with_overflow:
              Op = Instruction::Mul;
              break;
            case Intrinsic::uadd_with_overflow:
              Op = Instruction::Add;
              break;
            case Intrinsic::usub_with_overflow:
              Op = Instruction::Sub;
              break;
            case Intrinsic::umul_with_overflow:
              Op = Instruction::Mul;
              break;
            default:
              continue;
          }

          Value* S1 = Call->getArgOperand(0);
          Value* S2 = Call->getArgOperand(1);

          Type* IntTy = Fn->getFunctionType()->getParamType(0);
          Type* BoolTy = IntegerType::get(IntTy->getContext(), 1);
          StructType* StructTy = dyn_cast<StructType>(Fn->getReturnType());
          Constant* ZeroInt = ConstantInt::get(IntTy, 0);
          Constant* ZeroBool = ConstantInt::get(BoolTy, 0);
          Constant* Parts[2] = { ZeroInt, ZeroBool };
          Constant* ZeroStruct = ConstantStruct::get(
              StructTy, ArrayRef<Constant*>(Parts, Parts + 2));

          BinaryOperator* BinOp = BinaryOperator::Create(Op, S1, S2, "", Call);

          unsigned Indexes[1] = { 0 };
          Value* ResultStruct = InsertValueInst::Create(
              ZeroStruct, BinOp, ArrayRef<unsigned>(Indexes, Indexes + 1), "", Call);

          Call->replaceAllUsesWith(ResultStruct);
          Killed.push_back(Call);
        }
      }

      for (SmallVector<Instruction*, 8>::iterator I = Killed.begin(), E = Killed.end(); I != E; ++I) {
        (*I)->eraseFromParent();
      }
      return Killed.size() > 0;
    }
  };
}

char RemoveOverflowChecks::ID = 0;
static RegisterPass<RemoveOverflowChecks> X("remove-overflow-checks",
        "remove calls to llvm.*.with.overflow intrinsics");
