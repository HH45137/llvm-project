add_llvm_component_group(RedDSP)

set(LLVM_TARGET_DEFINITIONS RedDSP.td)

tablegen(LLVM RedDSPGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM RedDSPGenCallingConv.inc -gen-callingconv)
tablegen(LLVM RedDSPGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM RedDSPGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM RedDSPGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM RedDSPGenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(RedDSPCommonTableGen)

add_llvm_target(RedDSPCodeGen
  RedDSPAsmPrinter.cpp
  RedDSPFrameLowering.cpp
  RedDSPInstrInfo.cpp
  RedDSPISelDAGToDAG.cpp
  RedDSPISelLowering.cpp
  RedDSPRegisterInfo.cpp
  RedDSPSubtarget.cpp
  RedDSPTargetMachine.cpp

  LINK_COMPONENTS
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  MC
  RedDSPDesc
  RedDSPInfo
  SelectionDAG
  Support
  Target
  TargetParser

  ADD_TO_COMPONENT
  RedDSP
)

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)