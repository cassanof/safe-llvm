

#include "SafeReturn.h"
#include "../X86.h"
#include "../X86InstrBuilder.h"
#include "../X86TargetMachine.h"
#include "Utils.cpp"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

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

  // auto funcs_start = MF.begin();
  // skipEmptyMachineFunctions(funcs_start);
  // auto first_instr = funcs_start->begin();
  // // TODO: only insert entry encryption if you also insert ret decryption

  // // add entry encryption block
  // llvm::MachineBasicBlock *entry_encryption_block =
      // MF.CreateMachineBasicBlock();
  // MF.insert(funcs_start, entry_encryption_block);
  // entry_encryption_block->addSuccessor(first_instr->getParent());
  // first_instr = entry_encryption_block->begin(); // set new first instr

  // llvm::MachineOperand def_R11 =
      // llvm::MachineOperand::CreateReg(llvm::X86::R11, true);
  // llvm::MachineOperand ref_R11 =
      // llvm::MachineOperand::CreateReg(llvm::X86::R11, false);

  // // mov    %fs:0x28,%r11
  // llvm::MachineInstrBuilder entry_encryption =
      // llvm::BuildMI(*entry_encryption_block, first_instr,
                    // first_instr->getDebugLoc(), TII.get(llvm::X86::MOV64rm))
          // .addReg(0)
          // .addImm(1)
          // .addReg(0)
          // .addImm(0x28)
          // .addReg(llvm::X86::FS);
  // entry_encryption->addOperand(def_R11);

  // entry_encryption_block->addLiveIn(llvm::X86::R11);
  // entry_encryption_block->sortUniqueLiveIns();

  // // add ret decryption block

  // auto funcs_end = MF.end();

  return true;
}
