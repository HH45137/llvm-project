#ifndef LLVM_LIB_TARGET_REDDSP_REDDSPFRAMELOWERING_H
#define LLVM_LIB_TARGET_REDDSP_REDDSPFRAMELOWERING_H
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/MachineFunction.h"
namespace llvm {
class RedDSPFrameLowering : public TargetFrameLowering {
public:
  RedDSPFrameLowering() : TargetFrameLowering(StackGrowsDown, Align(16), 0) {}
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  bool hasFPImpl(const MachineFunction &) const override { return false; }
};
} // namespace llvm
#endif