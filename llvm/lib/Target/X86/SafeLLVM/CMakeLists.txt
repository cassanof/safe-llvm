add_llvm_component_library(LLVMSafeLLVM
  SafeReturn.cpp
  BranchCookie.cpp
  InstructionRewrite.cpp
  ImmediateReencoding.cpp
  Utils.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  Core
  MC
  SelectionDAG
  Support
  Target
  X86Desc
  X86Info
  GlobalISel
  ProfileData
  CFGuard

  ADD_TO_COMPONENT
  X86
  )

