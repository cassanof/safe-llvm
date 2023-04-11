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
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include <cstdint>
#include <cstdio>
#include <optional>

llvm::FunctionPass *llvm::createImmedateReencodingPass() {
  return new ImmediateReencodingMachinePass();
}

char ImmediateReencodingMachinePass::ID = 0;

static llvm::RegisterPass<ImmediateReencodingMachinePass>
    X("immreencode", "Immediate Reencoding Pass");

std::optional<const unsigned int>
getRegisterEquivalentOpcode(llvm::MachineInstr &MI) {
  switch (MI.getOpcode()) {
  // ------------------ ADD ------------------
  case llvm::X86::ADD8ri:
  case llvm::X86::ADD8ri8:
    return llvm::X86::ADD8rr;
  case llvm::X86::ADD16ri:
  case llvm::X86::ADD16ri8:
    return llvm::X86::ADD16rr;
  case llvm::X86::ADD32ri:
  case llvm::X86::ADD32ri8:
    return llvm::X86::ADD32rr;
  case llvm::X86::ADD64ri32:
  case llvm::X86::ADD64ri8:
    return llvm::X86::ADD64rr;
  // ------------------ SUB ------------------
  // TODO: SUB is not implemented yet
  // ------------------ AND ------------------
  // TODO: AND is not implemented yet
  // ------------------ OR ------------------
  // TODO: OR is not implemented yet
  // ------------------ XOR ------------------
  // TODO: XOR is not implemented yet
  // ------------------ CMP ------------------
  // TODO: CMP is not implemented yet
  // ------------------ TEST ------------------
  // TODO: TEST is not implemented yet
  // ------------------ MOV ------------------
  case llvm::X86::MOV8ri:
    return llvm::X86::MOV8rr;
  case llvm::X86::MOV8mi:
    return llvm::X86::MOV8mr;
  case llvm::X86::MOV16ri:
    return llvm::X86::MOV16rr;
  case llvm::X86::MOV16mi:
    return llvm::X86::MOV16mr;
  case llvm::X86::MOV32ri:
  case llvm::X86::MOV32ri64:
    return llvm::X86::MOV32rr;
  case llvm::X86::MOV32mi:
    return llvm::X86::MOV32mr;
  case llvm::X86::MOV64ri32:
  case llvm::X86::MOV64ri:
    return llvm::X86::MOV64rr;
  case llvm::X86::MOV64mi32:
    return llvm::X86::MOV64mr;
  // ---------------- DEFAULT ----------------
  default:
    return std::nullopt;
  }
}

unsigned int getSizedPUSHr(unsigned int Size) {
  switch (Size) {
  case 8:
  case 16:
    return llvm::X86::PUSH16r;
  case 32:
    return llvm::X86::PUSH32r;
  case 64:
    return llvm::X86::PUSH64r;
  default:
    llvm::errs() << "Invalid size for PUSHr: " << Size << "\n";
    return 0;
  }
}

unsigned int getSizedPOPr(unsigned int Size) {
  switch (Size) {
  case 8:
  case 16:
    return llvm::X86::POP16r;
  case 32:
    return llvm::X86::POP32r;
  case 64:
    return llvm::X86::POP64r;
  default:
    llvm::errs() << "Invalid size for POPr: " << Size << "\n";
    return 0;
  }
}

unsigned int getSizedMOVri(unsigned int Size) {
  switch (Size) {
  case 8:
    return llvm::X86::MOV8ri;
  case 16:
    return llvm::X86::MOV16ri;
  case 32:
    return llvm::X86::MOV32ri;
  case 64:
    return llvm::X86::MOV64ri;
  default:
    llvm::errs() << "Invalid size for MOVri: " << Size << "\n";
    return 0;
  }
}

unsigned int getSizedADDri(unsigned int Size) {
  switch (Size) {
  case 8:
    return llvm::X86::ADD8ri;
  case 16:
    return llvm::X86::ADD16ri;
  case 32:
    return llvm::X86::ADD32ri;
  case 64:
    return llvm::X86::ADD64ri32;
  default:
    llvm::errs() << "Invalid size for ADDri: " << Size << "\n";
    return 0;
  }
}

unsigned int getDefRegClassSize(llvm::MachineInstr &MI) {
  // get the register class of the destination register
  if (!MI.getOperand(0).isReg()) {
    llvm::errs() << "Operand 0 is not a register. Defaulting to 64 bits\n";
    return 64;
  }
  llvm::Register DefReg = MI.getOperand(0).getReg();
  const llvm::TargetRegisterClass *DefRC =
      MI.getParent()->getParent()->getRegInfo().getRegClassOrNull(DefReg);
  if (!DefRC) {
    llvm::errs() << "No register class found for register " << DefReg << "\n";
    return 0;
  }
  unsigned int DefSize = DefRC->MC->getSizeInBits();
  llvm::errs() << "Def register class size: " << DefSize << "\n";
  return DefSize;
}

llvm::Register getTempPhysRegister(llvm::MachineInstr &MI) {
  // check which kind of register we are dealing with if any
  // print operand 0
  llvm::errs() << "Operand 0: " << MI.getOperand(0) << "\n";
  if (MI.getOperand(0).isReg()) {
    unsigned int Size = getDefRegClassSize(MI);
    llvm::errs() << "Register class size: " << Size << "\n";
    if (Size == 8) {
      llvm::errs() << "r12b is perfect (byte register)\n";
      return llvm::X86::R12B;
    }
    if (Size == 16) {
      llvm::errs() << "r12w is perfect (word register)\n";
      return llvm::X86::R12W;
    }
    if (Size == 32) {
      llvm::errs() << "r12d is perfect (dword register)\n";
      return llvm::X86::R12D;
    }
    if (Size == 64) {
      llvm::errs() << "r12 is perfect (qword register)\n";
      return llvm::X86::R12;
    }
  }
  printf("No reg, r12 is fine, will work for most cases\n");
  return llvm::X86::R12;
}

llvm::Register getTempVirtRegister(llvm::MachineInstr &MI) {
  // check which kind of register we are dealing with if any
  // print operand 0
  llvm::errs() << "Operand 0: " << MI.getOperand(0) << "\n";
  llvm::MachineRegisterInfo &MRI = MI.getParent()->getParent()->getRegInfo();

  if (MI.getOperand(0).isReg()) {
    unsigned int Size = getDefRegClassSize(MI);
    llvm::errs() << "Register class size: " << Size << "\n";
    if (Size == 8) {
      llvm::errs() << "GR8 is perfect (byte register)\n";
      return MRI.createVirtualRegister(&llvm::X86::GR8RegClass);
    }
    if (Size == 16) {
      llvm::errs() << "GR16 is perfect (word register)\n";
      return MRI.createVirtualRegister(&llvm::X86::GR16RegClass);
    }
    if (Size == 32) {
      llvm::errs() << "GR32 is perfect (dword register)\n";
      return MRI.createVirtualRegister(&llvm::X86::GR32RegClass);
    }
    if (Size == 64) {
      llvm::errs() << "GR64 is perfect (qword register)\n";
      return MRI.createVirtualRegister(&llvm::X86::GR64RegClass);
    }
  }
  printf("No reg, GR64 is fine, will work for most cases\n");
  return MRI.createVirtualRegister(&llvm::X86::GR64RegClass);
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
    llvm::errs() << "Processing block " << MBB->getName()
                 << ", number: " << MBB->getNumber() << "\n";

    for (; I != IEnd; ++I) {
      llvm::MachineInstr &MI = *I;
      uint8_t num_operands = MI.getNumOperands();
      // get string representation of the instruction
      llvm::StringRef kind_str = TII.getName(MI.getOpcode());

      for (int op_idx = 0; op_idx < num_operands; op_idx += 1) {
        llvm::MachineOperand &MO = MI.getOperand(op_idx);
        if (!MO.isImm())
          continue;

        int64_t imm = MO.getImm();
        if (!encodesFreeBranch(imm))
          continue;

        llvm::errs() << "Found free-branch immediate at operand " << op_idx
                     << ": " << llvm::format("0x%lx", imm)
                     << " (ins: " << kind_str << ")!\n";

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

        std::optional<const unsigned int> maybeOpcode =
            getRegisterEquivalentOpcode(MI);
        if (!maybeOpcode.has_value()) {
          llvm::errs() << "Could not find a register-equivalent opcode for "
                          "instruction "
                       << kind_str << "!\n";
          continue;
        }
        unsigned int NewOpcode = maybeOpcode.value();

        // TODO: delete prints
        // print MBB before insertion
        MBB->print(llvm::errs(), nullptr);

        llvm::MachineInstrBuilder MIBB;
        llvm::DebugLoc DL = MI.getDebugLoc();

        // get a temporary register, r12 but different sizes depending on
        // the instruction's register class
        llvm::Register TempReg = getTempVirtRegister(MI);

        // get size of the register class of the destination register
        unsigned int DefSize = getDefRegClassSize(MI);

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
        // MIBB = llvm::BuildMI(*MBB, I, DL, TII.get(getSizedPUSHr(DefSize)));
        // MIBB.addReg(TempReg);
        pushEFLAGS(I, SaveEFLAGS);
        MIBB = llvm::BuildMI(*MBB, I, DL, TII.get(getSizedMOVri(DefSize)),
                             TempReg);
        MIBB.addImm(imm1);
        MIBB = llvm::BuildMI(*MBB, I, DL, TII.get(getSizedADDri(DefSize)),
                             TempReg);
        MIBB.addReg(TempReg);
        MIBB.addImm(imm2);
        popEFLAGS(I, SaveEFLAGS);

        llvm::MachineBasicBlock::iterator IAfterIMM = I;
        IAfterIMM++;
        // pop and kill the temporary register
        // MIBB =
            // llvm::BuildMI(*MBB, IAfterIMM, DL, TII.get(getSizedPOPr(DefSize)));

        // now, we change MO's value to R12
        MO.ChangeToRegister(TempReg, false, false, true);
        // change MI's instruction to be a register operand
        MI.setDesc(TII.get(NewOpcode));

        // add R12 to live registers if it is not already there
        if (!MBB->isLiveIn(TempReg)) {
          MBB->addLiveIn(TempReg);
          MBB->sortUniqueLiveIns();
        }

        // FIX FOR this:
        // The register $r12d needs to be live in to %bb.5, but is missing
        // from the live-in list.

        // print MBB after insertion
        MBB->print(llvm::errs(), nullptr);
      }
    }
  }

  return true;
}
