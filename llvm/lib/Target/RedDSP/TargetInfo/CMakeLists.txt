add_llvm_component_library(LLVMRedDSPInfo
  RedDSPTargetInfo.cpp
  LINK_COMPONENTS MC Support
  ADD_TO_COMPONENT RedDSP)