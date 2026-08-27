#include "RedDSPTargetMachine.h"
#include "RedDSP.h"
#include "TargetInfo/RedDSPTargetInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
using namespace llvm;

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRedDSPTarget() {
  RegisterTargetMachine<RedDSPTargetMachine> X(getTheRedDSPTarget());
}

namespace {
class RedDSPPassConfig : public TargetPassConfig {
public:
  RedDSPPassConfig(RedDSPTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}
  bool addInstSelector() override {
    addPass(createRedDSPISelDag(getTM<RedDSPTargetMachine>(), getOptLevel()));
    return false;
  }
};
}

RedDSPTargetMachine::RedDSPTargetMachine(
    const Target &T, const Triple &TT, StringRef CPU, StringRef FS,
    const TargetOptions &Options, std::optional<Reloc::Model> RM,
    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, "e-m:e-p:32:32-i64:64-n32-S128", TT, CPU, FS,
          Options, RM.value_or(Reloc::Static),
          getEffectiveCodeModel(CM, CodeModel::Small), OL),
      Subtarget(TT, CPU, FS, *this), TLOF(std::make_unique<TargetLoweringObjectFileELF>()) {
  initAsmInfo();
}

TargetPassConfig *RedDSPTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new RedDSPPassConfig(*this, PM);
}