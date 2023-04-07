#include "ImmediateReencoding.h"
#include "../X86.h"
#include "../X86InstrBuilder.h"
#include "../X86TargetMachine.h"
#include "MCTargetDesc/X86MCTargetDesc.h"
#include "Utils.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/Register.h"
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

bool isAddRI(llvm::MachineInstr &MI) {
  switch (MI.getOpcode()) {
  case llvm::X86::ADD32ri:
  case llvm::X86::ADD32ri8:
  case llvm::X86::ADD64ri32:
  case llvm::X86::ADD64ri8:
    return true;
  default:
    return false;
  }
}

bool ImmediateReencodingMachinePass::runOnMachineFunction(
    llvm::MachineFunction &MF) {
  // quit immediately if it's empty
  if (MF.empty())
    return false;

  llvm::MachineFunction::iterator MBB = MF.begin();
  llvm::MachineFunction::iterator MBBEnd = MF.end();
  const llvm::X86Subtarget &STI = MF.getSubtarget<llvm::X86Subtarget>();
  const llvm::X86InstrInfo &TII = *STI.getInstrInfo();

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
        if (!encodesFreeBranch(imm))
          continue;

        llvm::errs() << "Found free-branch immediate at " << i << ": "
                     << llvm::format("0x%lx", imm) << " (ins: " << kind
                     << ")!\n";

        // divide the imm into two.
        std::optional<std::pair<uint64_t, uint64_t>> maybeSplit =
            splitIntoNonFreeBranch(imm);
        if (!maybeSplit.has_value()) {
          llvm::errs() << "Could not split the immediate into two "
                          "non-free-branch immediates! WOW!\n";
          continue;
        }
        int64_t imm1 = maybeSplit.value().first;
        int64_t imm2 = maybeSplit.value().second;

        // BIG TODO: handle more instrs, this is just for ADDxxrixx testing
        if (!isAddRI(MI))
          continue;

        // TODO: delete prints
        // print MBB before insertion
        MBB->print(llvm::errs(), nullptr);

        llvm::MachineInstrBuilder MIBB;
        llvm::DebugLoc DL = MI.getDebugLoc();

        // we add up the two immediate values and store the result in R12
        // code:
        // push %r12
        // push %eflags # because ADD changes the CF and OF flags
        // mov $imm1, %r12
        // add $imm2, %r12
        // pop %eflags
        // [the imm we are patching] ?? %r12 ??
        // pop %r12
        llvm::Register SaveEFLAGS =
            MF.getRegInfo().createVirtualRegister(&llvm::X86::GR64RegClass);
        MIBB = llvm::BuildMI(*MBB, I, DL, TII.get(llvm::X86::PUSH64r));
        MIBB.addReg(llvm::X86::R12);
        pushEFLAGS(I, SaveEFLAGS);
        MIBB = llvm::BuildMI(*MBB, I, DL, TII.get(llvm::X86::MOV64ri));
        MIBB.addReg(llvm::X86::R12);
        MIBB.addImm(imm1);
        MIBB = llvm::BuildMI(*MBB, I, DL, TII.get(llvm::X86::ADD64ri32),
                             llvm::X86::R12);
        MIBB.addReg(llvm::X86::R12);
        MIBB.addImm(imm2);
        popEFLAGS(I, SaveEFLAGS);

        llvm::MachineBasicBlock::iterator IAfterIMM = I;
        IAfterIMM++;
        MIBB = llvm::BuildMI(*MBB, IAfterIMM, DL, TII.get(llvm::X86::POP64r));
        MIBB.addReg(llvm::X86::R12);

        // now, we change MO's value to R12
        MO.ChangeToRegister(llvm::X86::R12, false);
        // change MI's instruction to be a register operand

        // BIG TODO: we gotta generalize tis to many insts....
        MI.setDesc(TII.get(llvm::X86::ADD64rr));

        // add R12 to live registers
        MBB->addLiveIn(llvm::X86::R12);

        // print MBB after insertion
        MBB->print(llvm::errs(), nullptr);
      }
    }
  }

  return true;
}
