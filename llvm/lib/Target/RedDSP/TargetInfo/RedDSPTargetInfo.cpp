#include "RedDSPTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
using namespace llvm;

Target &llvm::getTheRedDSPTarget() {
  static Target T;
  return T;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRedDSPTargetInfo() {
  RegisterTarget<Triple::reddsp> X(getTheRedDSPTarget(), "reddsp", "RED DSP",
                                   "RedDSP");
}