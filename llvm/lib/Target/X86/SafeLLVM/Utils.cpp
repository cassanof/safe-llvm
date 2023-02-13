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

llvm::MachineBasicBlock *splitAt(llvm::MachineBasicBlock *MBB,
                                 llvm::MachineInstr &MI, bool UpdateLiveIns,
                                 llvm::LiveIntervals *LIS) {
  llvm::MachineBasicBlock::iterator SplitPoint(&MI);
  ++SplitPoint;

  if (SplitPoint == MBB->end()) {
    // Don't bother with a new block.
    return MBB;
  }

  llvm::MachineFunction *MF = MBB->getParent();

  llvm::LivePhysRegs LiveRegs;
  if (UpdateLiveIns) {
    // Make sure we add any physregs we define in the block as liveins to the
    // new block.
    llvm::MachineBasicBlock::iterator Prev(&MI);
    LiveRegs.init(*MF->getSubtarget().getRegisterInfo());
    LiveRegs.addLiveOuts(*MBB);
    for (auto I = MBB->rbegin(), E = Prev.getReverse(); I != E; ++I)
      LiveRegs.stepBackward(*I);
  }

  llvm::MachineBasicBlock *SplitBB =
      MF->CreateMachineBasicBlock(MBB->getBasicBlock());

  MF->insert(++llvm::MachineFunction::iterator(*MBB), SplitBB);
  SplitBB->splice(SplitBB->begin(), MBB, SplitPoint, MBB->end());

  SplitBB->transferSuccessorsAndUpdatePHIs(MBB);
  MBB->addSuccessor(SplitBB);

  if (UpdateLiveIns)
    addLiveIns(*SplitBB, LiveRegs);

  if (LIS)
    LIS->insertMBBInMaps(SplitBB);

  return SplitBB;
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
