#include "BranchCookie.h"
#include "../X86.h"
#include "../X86InstrBuilder.h"
#include "../X86TargetMachine.h"
#include "Utils.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include <cstdio>

llvm::FunctionPass *llvm::createBranchCookieMachinePass() {
  return new BranchCookieMachinePass();
}

char BranchCookieMachinePass::ID = 0;

static llvm::RegisterPass<BranchCookieMachinePass> X("branchcookie",
                                                   "Branch Cookie Pass");


bool BranchCookieMachinePass::runOnMachineFunction(llvm::MachineFunction &MF) {
  // quit immediately if it's empty
  if (MF.empty())
    return false;

  return true;
}

