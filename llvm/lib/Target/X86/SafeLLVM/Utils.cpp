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

void pushEFLAGS(llvm::MachineBasicBlock::iterator &I, llvm::Register R) {
  // get parent stuff
  llvm::MachineInstr *MI = &*I;
  llvm::MachineBasicBlock *MBB = MI->getParent();
  llvm::MachineFunction *MF = MBB->getParent();
  llvm::DebugLoc DL = MI->getDebugLoc();
  const llvm::X86Subtarget &STI = MF->getSubtarget<llvm::X86Subtarget>();
  const llvm::X86InstrInfo &TII = *STI.getInstrInfo();
  const llvm::TargetRegisterInfo *TRI = STI.getRegisterInfo();

  // building the push
  llvm::MachineInstrBuilder MIBB;

  // if eflags is dead, we need to define it, otherwise we can't push it.
  // this is a hack, but it works. however, we are searching the neighborhood
  // of only 1000 instructions. this is a problem if the eflags register is
  // not used for a long time, but whatever, we can make a smarter algorithm
  // later.
  llvm::MachineBasicBlock::LivenessQueryResult EflagLive =
      MBB->computeRegisterLiveness(TRI, llvm::X86::EFLAGS, MI, 5000);

  if (llvm::MachineBasicBlock::LQR_Dead == EflagLive) {
    MIBB = BuildMI(*MBB, MI, DL, TII.get(llvm::TargetOpcode::IMPLICIT_DEF),
                   llvm::X86::EFLAGS);
  }

  // this is ugly, but the only way to do it.
  // llvm has no idea of what eflags is, so we have to use the
  // INLINEASM instruction to push the eflags register,
  // using the external pushfq instruction.
  MIBB = BuildMI(*MBB, MI, DL, TII.get(llvm::X86::INLINEASM))
             .addExternalSymbol("pushfq")
             .addImm(0)
             .addReg(llvm::X86::RSP, llvm::RegState::ImplicitDefine)
             .addReg(llvm::X86::RSP, llvm::RegState::ImplicitKill)
             .addReg(llvm::X86::EFLAGS, llvm::RegState::ImplicitKill);

  MIBB = BuildMI(*MBB, MI, DL, TII.get(llvm::X86::POP64r))
             .addReg(R, llvm::RegState::Define);
}

void popEFLAGS(llvm::MachineBasicBlock::iterator &I, llvm::Register R) {
  // get parent stuff
  llvm::MachineInstr *MI = &*I;
  llvm::MachineBasicBlock *MBB = MI->getParent();
  llvm::MachineFunction *MF = MBB->getParent();
  llvm::DebugLoc DL = MI->getDebugLoc();
  const llvm::X86Subtarget &STI = MF->getSubtarget<llvm::X86Subtarget>();
  const llvm::X86InstrInfo &TII = *STI.getInstrInfo();

  // building the pop
  llvm::MachineInstrBuilder MIBB;

  // same idea as pushEFLAGS, but popfq instead of pushfq.
  MIBB = BuildMI(*MBB, MI, DL, TII.get(llvm::X86::PUSH64r))
             .addReg(R, llvm::RegState::Kill);

  MIBB = BuildMI(*MBB, MI, DL, TII.get(llvm::X86::INLINEASM))
             .addExternalSymbol("popfq")
             .addImm(0)
             .addReg(llvm::X86::RSP, llvm::RegState::ImplicitDefine)
             .addReg(llvm::X86::RSP, llvm::RegState::ImplicitKill)
             .addReg(llvm::X86::EFLAGS, llvm::RegState::ImplicitDefine);
}
