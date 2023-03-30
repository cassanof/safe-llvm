#include "../X86.h"
#include "../X86InstrBuilder.h"
#include "../X86TargetMachine.h"
#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

void skipEmptyMachineFunctions(llvm::MachineFunction::iterator &funcs) {
  while (funcs->empty()) {
    ++funcs;
  }
}

void insertNopSled(llvm::MachineInstr *MI) {
  llvm::DebugLoc DL = MI->getDebugLoc();
  llvm::MachineBasicBlock *MBB = MI->getParent();
  llvm::MachineFunction *MF = MBB->getParent();

  const llvm::X86Subtarget &STI = MF->getSubtarget<llvm::X86Subtarget>();
  const llvm::X86InstrInfo &TII = *STI.getInstrInfo();

  // now, we need to create the sled
  llvm::MachineInstrBuilder MIBNop;

  for (int i = 0; i < 9; i++) {
    MIBNop = BuildMI(*MBB, MI, DL, TII.get(llvm::X86::NOOP));
  }
}

bool isBranchInto(llvm::MachineInstr *MI, llvm::MachineFunction *MF) {
  if (MI->isBranch()) {
    llvm::MachineOperand MO = MI->getOperand(0);
    if (!MO.isMBB()) {
      return false;
    }
    llvm::MachineFunction *Branching = MO.getMBB()->getParent();
    if (Branching->getFunctionNumber() == MF->getFunctionNumber()) {
      return true;
    }
  }
  return false;
}

bool encodesFreeBranch(uint8_t byte) {
  switch (byte) {
  case 0xc3: // ret
  case 0xc2: // ret imm16
  case 0xcb: // retf
  case 0xca: // retf imm16
  case 0xcf: // iret
    return true;
  }

  return false;
}
