#ifndef LLVM_LIB_TARGET_REDDSP_REDDSPSUBTARGET_H
#define LLVM_LIB_TARGET_REDDSP_REDDSPSUBTARGET_H
#include "RedDSPFrameLowering.h"
#include "RedDSPISelLowering.h"
#include "RedDSPInstrInfo.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#define GET_SUBTARGETINFO_HEADER
#include "RedDSPGenSubtargetInfo.inc"

namespace llvm {
class RedDSPSubtarget : public RedDSPGenSubtargetInfo {
  RedDSPInstrInfo InstrInfo;
  RedDSPFrameLowering FrameLowering;
  RedDSPTargetLowering TLInfo;
  SelectionDAGTargetInfo TSInfo;
public:
  RedDSPSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                  const TargetMachine &TM);
  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);
  const RedDSPInstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const RedDSPRegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }
  const RedDSPFrameLowering *getFrameLowering() const override { return &FrameLowering; }
  const RedDSPTargetLowering *getTargetLowering() const override { return &TLInfo; }
  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override { return &TSInfo; }
};
} // namespace llvm
#endif