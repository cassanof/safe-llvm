#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/MachineFunction.h"

// Skips empty machine functions.
void skipEmptyMachineFunctions(llvm::MachineFunction::iterator &funcs);

// produces a nop sled of 9 nop instructions.
void insertNopSled(llvm::MachineInstr *MI);

// returns true if the given instruction is a branch instruction that
// branches into the given machine function.
bool isBranchInto(llvm::MachineInstr *MI, llvm::MachineFunction *MF);

// returns true if the given byte encodes a free-branch instruction.
bool encodesFreeBranch(uint8_t byte);
