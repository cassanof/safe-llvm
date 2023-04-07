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

// returns true if the given qword could encode a free branch instruction.
bool encodesFreeBranch(uint64_t qword);

// splits a qword that encodes a free branch instruction into two qwords
// that do not encode a free branch instruction. May return an empty
// optional if the greedy algorithm fails to find a split.
std::optional<std::pair<uint64_t, uint64_t>> splitIntoNonFreeBranch(uint64_t qword);

// produces an instruction that pushes the eflags register onto the stack.
// the given register is used for saving the eflags register.
void pushEFLAGS(llvm::MachineBasicBlock::iterator &I, llvm::Register R);

// produces an instruction that pops the eflags register from the stack.
// the given register is used for saving the eflags register.
void popEFLAGS(llvm::MachineBasicBlock::iterator &I, llvm::Register R);
