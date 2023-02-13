#include "SafeReturn.h"
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

llvm::FunctionPass *llvm::createSafeReturnMachinePass() {
  return new SafeReturnMachinePass();
}

char SafeReturnMachinePass::ID = 0;

static llvm::RegisterPass<SafeReturnMachinePass> X("safereturn",
                                                   "Safe Return Pass");

void insertEncryptionInstrs(llvm::MachineBasicBlock &MBB,
                            llvm::MachineBasicBlock::iterator I,
                            llvm::X86Subtarget &STI) {
  const llvm::X86InstrInfo &TII = *STI.getInstrInfo();

  llvm::errs()
      << "--- SafeReturnMachinePass: inserted instructions START ---\n";
  // moves the stack canary value to r11
  // mov    %fs:0x28,%r11
  llvm::MachineInstrBuilder MIB =
      BuildMI(MBB, I, llvm::DebugLoc(), TII.get(llvm::X86::MOV64rm));
  // hack to get the correct register
  MIB->addOperand(llvm::MachineOperand::CreateReg(llvm::X86::R11, true));
  MIB.addReg(0);
  MIB.addImm(1);
  MIB.addReg(0);
  MIB.addImm(0x28);
  MIB.addReg(llvm::X86::FS);

  llvm::MachineInstr *FIRST = MIB.getInstr();
  FIRST->print(llvm::errs());

  // create the instruction
  // xor %r11, (%rsp)
  MIB = BuildMI(MBB, I, llvm::DebugLoc(), TII.get(llvm::X86::XOR64mr));
  llvm::addRegOffset(MIB, llvm::X86::RSP, false, 0);
  MIB->addOperand(llvm::MachineOperand::CreateReg(llvm::X86::R11, false));
  MIB.getInstr()->print(llvm::errs());

  llvm::errs() << "--- SafeReturnMachinePass: inserted instructions END ---\n";

  // tell the register allocator that R11 is used
  MBB.addLiveIn(llvm::X86::R11);
  MBB.sortUniqueLiveIns();
}

void insertEntryEncrypt(llvm::MachineFunction &MF, llvm::X86Subtarget &STI) {
  // get the first basic block in the function
  llvm::MachineBasicBlock &MBB = MF.front();

  // get the first instruction in the basic block
  llvm::MachineBasicBlock::iterator I = MBB.begin();

  insertEncryptionInstrs(MBB, I, STI);
}

bool SafeReturnMachinePass::runOnMachineFunction(llvm::MachineFunction &MF) {
  // quit immediately if it's empty
  if (MF.empty())
    return false;

  const llvm::X86Subtarget &STI = MF.getSubtarget<llvm::X86Subtarget>();

  // print the name of the function
  llvm::errs() << "SafeReturnMachinePass: " << MF.getName() << "\n";

  // firsly, we insert the ret code, then if we had any rets, we insert the
  // entry code
  llvm::MachineFunction::iterator MBB = MF.begin();
  llvm::MachineFunction::iterator MBBEnd = MF.end();
  bool hadRet = false;

  // TODO: worry about branching
  for (; MBB != MBBEnd; ++MBB) {
    llvm::MachineBasicBlock::iterator I = MBB->begin();
    llvm::MachineBasicBlock::iterator IEnd = MBB->end();

    for (; I != IEnd; ++I) {
      if (I->getOpcode() == llvm::X86::RET) {
        hadRet = true;
        printf("SafeReturnMachinePass: found ret\n");
        insertEncryptionInstrs(*MBB, I, const_cast<llvm::X86Subtarget &>(STI));
      }
    }
  }

  // only insert the entry code if we had a ret
  if (hadRet) {
    insertEntryEncrypt(MF, const_cast<llvm::X86Subtarget &>(STI));
  }

  return true;
}
