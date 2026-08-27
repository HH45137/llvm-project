#include "RedDSPSubtarget.h"
using namespace llvm;
#define DEBUG_TYPE "reddsp-subtarget"
#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "RedDSPGenSubtargetInfo.inc"

RedDSPSubtarget::RedDSPSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                                 const TargetMachine &TM)
    : RedDSPGenSubtargetInfo(TT, CPU.empty() ? "generic" : CPU,
                            CPU.empty() ? "generic" : CPU, FS),
      InstrInfo(*this), FrameLowering(), TLInfo(TM, *this) {
  ParseSubtargetFeatures(CPU.empty() ? "generic" : CPU,
                         CPU.empty() ? "generic" : CPU, FS);
}