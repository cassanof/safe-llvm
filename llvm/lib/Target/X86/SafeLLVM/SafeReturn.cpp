

#include "SafeReturn.h"
#include "../X86.h"
#include "../X86InstrBuilder.h"
#include "../X86TargetMachine.h"
#include "Utils.cpp"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include <cstdio>

llvm::FunctionPass *llvm::createSafeReturnMachinePass() {
  return new SafeReturnMachinePass();
}

char SafeReturnMachinePass::ID = 0;

static llvm::RegisterPass<SafeReturnMachinePass> X("safereturn",
                                                   "Safe Return Pass");

bool SafeReturnMachinePass::runOnMachineFunction(llvm::MachineFunction &MF) {
  // quit immediately if it's empty
  if (MF.empty())
    return false;

  const llvm::X86Subtarget &STI = MF.getSubtarget<llvm::X86Subtarget>();
  const llvm::X86InstrInfo &TII = *STI.getInstrInfo();

  // print the name of the function
  llvm::errs() << "SafeReturnMachinePass: " << MF.getName() << "\n";

  // get the first basic block in the function
  llvm::MachineBasicBlock &MBB = MF.front();

  // get the first instruction in the basic block
  llvm::MachineBasicBlock::iterator I = MBB.begin();

  // TODO: replace 1337 with the stack canary value

  llvm::errs() << "SafeReturnMachinePass: inserted instructions: ";
  // create the instruction
  // mov    $1337,%r11
  llvm::MachineInstrBuilder MIB = BuildMI(
      MBB, I, llvm::DebugLoc(), TII.get(llvm::X86::MOV64ri), llvm::X86::R11);
  MIB.addImm(1337);
  MIB.getInstr()->print(llvm::errs());

  // create the instruction
  // xor %r11, (%rsp)
  MIB = BuildMI(MBB, I, llvm::DebugLoc(), TII.get(llvm::X86::XOR64mr));
  llvm::addRegOffset(MIB, llvm::X86::RSP, false, 0);
  MIB->addOperand(llvm::MachineOperand::CreateReg(llvm::X86::R11, false));
  MIB.getInstr()->print(llvm::errs());

  llvm::errs() << "\n";

  return true;
}
