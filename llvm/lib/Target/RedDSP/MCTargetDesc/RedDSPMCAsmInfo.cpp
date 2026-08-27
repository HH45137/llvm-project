#include "RedDSPMCAsmInfo.h"
using namespace llvm;
RedDSPMCAsmInfo::RedDSPMCAsmInfo(const MCTargetOptions &Options)
    : MCAsmInfoELF(Options) {
  CodePointerSize = 4;
  CalleeSaveStackSlotSize = 4;
  CommentString = "//";
  SupportsDebugInformation = false;
}