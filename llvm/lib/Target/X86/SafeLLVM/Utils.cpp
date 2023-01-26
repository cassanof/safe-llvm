
/// Skips empty machine blocks in the machine functions.
#include "llvm/CodeGen/MachineFunction.h"
void skipEmptyMachineFunctions(llvm::MachineFunction::iterator &funcs) {
  while (funcs->empty()) {
    ++funcs;
  }
}
