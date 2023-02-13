#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/MachineFunction.h"

/// Skips empty machine functions.
void skipEmptyMachineFunctions(llvm::MachineFunction::iterator &funcs);

// produces a nop sled of 9 nop instructions.
void insertNopSled(llvm::MachineInstr *MI);
