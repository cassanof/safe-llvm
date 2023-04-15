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
#include <vector>

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
  case llvm::X86::SUB8ri:
  case llvm::X86::SUB8ri8:
    return llvm::X86::SUB8rr;
  case llvm::X86::SUB16ri:
  case llvm::X86::SUB16ri8:
    return llvm::X86::SUB16rr;
  case llvm::X86::SUB32ri:
  case llvm::X86::SUB32ri8:
    return llvm::X86::SUB32rr;
  case llvm::X86::SUB64ri32:
  case llvm::X86::SUB64ri8:
    return llvm::X86::SUB64rr;
  // ------------------ AND ------------------
  case llvm::X86::AND8ri:
  case llvm::X86::AND8ri8:
    return llvm::X86::AND8rr;
  case llvm::X86::AND16ri:
  case llvm::X86::AND16ri8:
    return llvm::X86::AND16rr;
  case llvm::X86::AND32ri:
  case llvm::X86::AND32ri8:
    return llvm::X86::AND32rr;
  case llvm::X86::AND64ri32:
  case llvm::X86::AND64ri8:
    return llvm::X86::AND64rr;
  // ------------------ OR ------------------
  case llvm::X86::OR8ri:
  case llvm::X86::OR8ri8:
    return llvm::X86::OR8rr;
  case llvm::X86::OR16ri:
  case llvm::X86::OR16ri8:
    return llvm::X86::OR16rr;
  case llvm::X86::OR32ri:
  case llvm::X86::OR32ri8:
    return llvm::X86::OR32rr;
  case llvm::X86::OR64ri32:
  case llvm::X86::OR64ri8:
    return llvm::X86::OR64rr;
  // ------------------ XOR ------------------
  case llvm::X86::XOR8ri:
  case llvm::X86::XOR8ri8:
    return llvm::X86::XOR8rr;
  case llvm::X86::XOR16ri:
  case llvm::X86::XOR16ri8:
    return llvm::X86::XOR16rr;
  case llvm::X86::XOR32ri:
  case llvm::X86::XOR32ri8:
    return llvm::X86::XOR32rr;
  case llvm::X86::XOR64ri32:
  case llvm::X86::XOR64ri8:
    return llvm::X86::XOR64rr;
  // ------------------ CMP ------------------
  case llvm::X86::CMP8ri:
  case llvm::X86::CMP8ri8:
    return llvm::X86::CMP8rr;
  case llvm::X86::CMP16ri:
  case llvm::X86::CMP16ri8:
    return llvm::X86::CMP16rr;
  case llvm::X86::CMP32ri:
  case llvm::X86::CMP32ri8:
    return llvm::X86::CMP32rr;
  case llvm::X86::CMP64ri32:
  case llvm::X86::CMP64ri8:
    return llvm::X86::CMP64rr;
  // ------------------ TEST ------------------
  case llvm::X86::TEST8ri:
    return llvm::X86::TEST8rr;
  case llvm::X86::TEST16ri:
    return llvm::X86::TEST16rr;
  case llvm::X86::TEST32ri:
    return llvm::X86::TEST32rr;
  case llvm::X86::TEST64ri32:
    return llvm::X86::TEST64rr;
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

// produces a MOVri instruction with the given size
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
    assert(false && "Invalid size for MOVri");
  }
}

// produces a MOVrr instruction with the given size
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
    assert(false && "Invalid size for ADDri");
  }
}

// returns the size of the register class of the destination register of the
// given instruction, if it has a destination register. if it doesn't,
// it defaults to 64 bits.
unsigned int getDefRegClassSize(llvm::MachineInstr &MI) {
  // get the register class of the destination register
  if (!MI.getOperand(0).isReg()) {
    llvm::errs() << "Operand 0 is not a register. Defaulting to 64 bits\n";
    return 64;
  }
  llvm::Register DefReg = MI.getOperand(0).getReg();
  const llvm::TargetRegisterClass *DefRC =
      MI.getParent()->getParent()->getRegInfo().getRegClassOrNull(DefReg);
  assert(DefRC && "No register class found for register");
  unsigned int DefSize = DefRC->MC->getSizeInBits();
  llvm::errs() << "Def register class size: " << DefSize << "\n";
  return DefSize;
}

// produces a temporary virtual register with the given size
llvm::Register getTempVirtRegister(llvm::MachineRegisterInfo &MRI,
                                   unsigned int Size) {
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
  assert(false && "No register class found for size");
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
  llvm::MachineRegisterInfo &MRI = MF.getRegInfo();

  // this is a list of registers that encode free branches.
  // we don't want the register allocator to use them when
  // resolving virtual registers.
  // example:
  // mov r10, 0x1 ---> 49 c7 c3 01
  //                         ^^
  //                         this is r11, it encodes `ret`
  std::vector<llvm::Register> EncodesFreeBranches = {llvm::X86::R10,
                                                     llvm::X86::R11};

  for (; MBB != MBBEnd; ++MBB) {
    llvm::MachineBasicBlock::iterator I = MBB->begin();
    llvm::MachineBasicBlock::iterator IEnd = MBB->end();
    for (; I != IEnd; ++I) {
      llvm::MachineInstr &MI = *I;
      uint8_t NumOperands = MI.getNumOperands();
      // get string representation of the instruction
      llvm::StringRef KindStr = TII.getName(MI.getOpcode());

      for (int OpIdx = 0; OpIdx < NumOperands; OpIdx += 1) {
        llvm::MachineOperand &MO = MI.getOperand(OpIdx);
        if (!MO.isImm())
          continue;

        int64_t Imm = MO.getImm();
        if (!encodesFreeBranch(Imm))
          continue;

        llvm::errs() << "Found free-branch immediate at operand " << OpIdx
                     << ": " << llvm::format("0x%lx", Imm)
                     << " (ins: " << KindStr << ")!\n";

        std::optional<const unsigned int> MaybeOpcode =
            getRegisterEquivalentOpcode(MI);
        if (!MaybeOpcode.has_value()) {
          llvm::errs() << "Could not find a register-equivalent opcode for "
                          "instruction "
                       << KindStr << "!\n";
          continue;
        }
        unsigned int NewOpcode = MaybeOpcode.value();

        // divide the imm into two.
        std::optional<std::pair<uint64_t, uint64_t>> MaybeSplit =
            splitIntoNonFreeBranch(Imm);
        if (!MaybeSplit.has_value()) {
          // the probability of this happening is very very very very low
          llvm::errs() << "Could not split the immediate into two "
                          "non-free-branch immediates! WOW!\n";
          continue;
        }

        int64_t Imm1 = MaybeSplit.value().first;
        int64_t Imm2 = MaybeSplit.value().second;

        llvm::MachineInstrBuilder MIBB;
        llvm::DebugLoc DL = MI.getDebugLoc();

        // get size of the register class of the destination register.
        // defaults to 64 bits if no register is found
        unsigned int DefSize = getDefRegClassSize(MI);

        // get two temporary registers.
        // we need two because we are in SSA form and we cannot have two
        // instructions writing to the same register.
        llvm::Register TempReg1 = getTempVirtRegister(MRI, DefSize);
        llvm::Register TempReg2 = getTempVirtRegister(MRI, DefSize);

        // we add up the two immediate values and store the result in temp
        // code:
        // push %eflags # because ADD changes the CF and OF flags
        // mov $imm1, %temp
        // add $imm2, %temp
        // pop %eflags
        // [the imm we are patching] ?? %temp ??
        llvm::Register SaveEFLAGS =
            MF.getRegInfo().createVirtualRegister(&llvm::X86::GR64RegClass);
        pushEFLAGS(I, SaveEFLAGS);
        MIBB = llvm::BuildMI(*MBB, I, DL, TII.get(getSizedMOVri(DefSize)),
                             TempReg1);
        MIBB.addImm(Imm1);
        MIBB = llvm::BuildMI(*MBB, I, DL, TII.get(getSizedADDri(DefSize)),
                             TempReg2);
        // we kill temp1 because it is not used anymore after this instruction
        MIBB.addReg(TempReg1, llvm::RegState::Kill);
        MIBB.addImm(Imm2);
        popEFLAGS(I, SaveEFLAGS);

        llvm::MachineBasicBlock::iterator IAfterIMM = I;
        IAfterIMM++;

        // now, we change MO's value to the temp register.
        MO.ChangeToRegister(
            // register to change to
            TempReg2,
            // isDef. we are not changing the definition of the register
            false,
            // isImp. no idea what this is for
            false,
            // isKill. we are killing the register because it is not used after
            // this
            true);

        // change MI's instruction to be a register operand
        MI.setDesc(TII.get(NewOpcode));

        // adding live-ins of bad registers to the MBB
        for (auto Reg : EncodesFreeBranches) {
          if (!MBB->isLiveIn(Reg)) {
            MBB->addLiveIn(Reg);
          }
        }
      }
    }
  }

  return true;
}
