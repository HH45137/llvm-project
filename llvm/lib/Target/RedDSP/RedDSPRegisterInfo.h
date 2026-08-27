#ifndef LLVM_LIB_TARGET_REDDSP_REDDSPREGISTERINFO_H
#define LLVM_LIB_TARGET_REDDSP_REDDSPREGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "RedDSP.h"
#define GET_REGINFO_ENUM
#include "RedDSPGenRegisterInfo.inc"
#define GET_REGINFO_HEADER
#include "RedDSPGenRegisterInfo.inc"

namespace llvm {
class RedDSPRegisterInfo : public RedDSPGenRegisterInfo {
public:
  RedDSPRegisterInfo();
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;
  const TargetRegisterClass *getPointerRegClass(unsigned Kind = 0) const override;
  bool eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;
  Register getFrameRegister(const MachineFunction &MF) const override;
};
} // namespace llvm
#endif