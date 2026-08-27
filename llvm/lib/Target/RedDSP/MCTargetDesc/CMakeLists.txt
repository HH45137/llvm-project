add_llvm_component_library(LLVMRedDSPDesc
  RedDSPMCTargetDesc.cpp
  RedDSPMCAsmInfo.cpp
  RedDSPInstPrinter.cpp
  LINK_COMPONENTS MC RedDSPInfo Support
  ADD_TO_COMPONENT RedDSP)