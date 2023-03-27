#include "ImmediateReencoding.h"
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
#include <cstdint>
#include <cstdio>

llvm::FunctionPass *llvm::createImmedateReencodingPass() {
  return new ImmediateReencodingMachinePass();
}

char ImmediateReencodingMachinePass::ID = 0;

static llvm::RegisterPass<ImmediateReencodingMachinePass>
    X("immreencode", "Immediate Reencoding Pass");

bool ImmediateReencodingMachinePass::runOnMachineFunction(
    llvm::MachineFunction &MF) {
  // quit immediately if it's empty
  if (MF.empty())
    return false;

  llvm::MachineFunction::iterator MBB = MF.begin();
  llvm::MachineFunction::iterator MBBEnd = MF.end();

  for (; MBB != MBBEnd; ++MBB) {
    llvm::MachineBasicBlock::iterator I = MBB->begin();
    llvm::MachineBasicBlock::iterator IEnd = MBB->end();
    for (; I != IEnd; ++I) {
      llvm::MachineInstr &MI = *I;
      uint8_t num_operands = MI.getNumOperands();
      auto kind = MI.getOpcode();

      for (int i = 0; i < num_operands; i += 1) {
        llvm::MachineOperand &MO = MI.getOperand(i);
        if (!MO.isImm())
          continue;

        int64_t imm = MO.getImm();
        if (!encodesFreeBranch(imm & 0xff))
          continue;

        llvm::errs() << "Found free-branch immediate at " << i << ": "
                     << llvm::format("0x%lx", imm) << "!\n";
      }
    }
  }

  return true;
}
