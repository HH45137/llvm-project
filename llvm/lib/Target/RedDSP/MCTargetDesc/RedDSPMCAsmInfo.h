#ifndef LLVM_LIB_TARGET_REDDSP_MCTARGETDESC_REDDSPMCASMINFO_H
#define LLVM_LIB_TARGET_REDDSP_MCTARGETDESC_REDDSPMCASMINFO_H
#include "llvm/MC/MCAsmInfoELF.h"
namespace llvm {
class RedDSPMCAsmInfo : public MCAsmInfoELF {
public:
  explicit RedDSPMCAsmInfo(const MCTargetOptions &Options);
};
}
#endif