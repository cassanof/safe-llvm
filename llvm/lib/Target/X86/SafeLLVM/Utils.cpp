#include "../X86.h"
#include "../X86InstrBuilder.h"
#include "../X86TargetMachine.h"
#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include <cstdint>

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

bool encodesFreeBranch(uint64_t qword) {
  for (int i = 0; i < 8; i++) {
    // NOTE: for now, we may want to add multi-byte instructions
    uint8_t byte = (qword >> (i * 8)) & 0xff;
    switch (byte) {
    case 0xc3: // ret
    case 0xc2: // ret imm16
    case 0xcb: // retf
    case 0xca: // retf imm16
    case 0xcf: // iret
      return true;
    }
  }

  return false;
}

// greedy bad byte splitting algorithm. fuzzed in Rust at
// [root]/scripts/bad_byte_fuzz/ in millions of iterations, this algorithm was
// able to find a split for every qword that encodes a free branch instruction.
// however, it may not, so we return an optional.
std::optional<std::pair<uint64_t, uint64_t>>
splitIntoNonFreeBranch(uint64_t qword) {
  uint64_t left = qword / 2;
  uint64_t right = qword - left;

  // set the heuristic to the original value
  // NOTE: is this a heuristic or perfect pivot? Who knows. This looks related
  // to the collatz conjecture.
  uint64_t heuristic = qword;

  // now, we try extra hard to fix it B)
  bool leftBad = encodesFreeBranch(left);
  bool rightBad = encodesFreeBranch(right);
  int i = 0;
  while (leftBad || rightBad) {
    if (heuristic == 0) {
      return std::nullopt;
    }
    heuristic /= 2;

    if (i % 2 == 0) {
      uint64_t l1 = left / 2;
      uint64_t l2 = left - l1;
      left = l1;
      right += l2;
    } else {
      uint64_t r1 = right / 2;
      uint64_t r2 = right - r1;
      right = r1;
      left += r2;
    }

    leftBad = encodesFreeBranch(left);
    rightBad = encodesFreeBranch(right);
    i += 1;
  }

  return std::make_pair(left, right);
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
