#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/raw_ostream.h"

class InstructionRewritePass : public llvm::MachineFunctionPass {
public:
  static char ID;
  InstructionRewritePass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(llvm::MachineFunction &MF) override;
  llvm::StringRef getPassName() const override {
    return "Instruction Rewrite Pass";
  }
};