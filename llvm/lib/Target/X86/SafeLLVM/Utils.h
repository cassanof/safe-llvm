#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/Register.h"

// Skips empty machine functions.
void skipEmptyMachineFunctions(llvm::MachineFunction::iterator &funcs);

// produces a nop sled of 9 nop instructions.
void insertNopSled(llvm::MachineInstr *MI);

// returns true if the given instruction is a branch instruction that
// branches into the given machine function.
bool isBranchInto(llvm::MachineInstr *MI, llvm::MachineFunction *MF);

// returns true if the given byte encodes a free-branch instruction.
bool encodesFreeBranch(uint8_t byte);

// produces an instruction that pushes the eflags register onto the stack.
// the given register is used for saving the eflags register.
void pushEFLAGS(llvm::MachineBasicBlock::iterator &I, llvm::Register R);

// produces an instruction that pops the eflags register from the stack.
// the given register is used for saving the eflags register.
void popEFLAGS(llvm::MachineBasicBlock::iterator &I, llvm::Register R);
