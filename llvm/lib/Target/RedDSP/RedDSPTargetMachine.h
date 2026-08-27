#ifndef LLVM_LIB_TARGET_REDDSP_REDDSPTARGETMACHINE_H
#define LLVM_LIB_TARGET_REDDSP_REDDSPTARGETMACHINE_H
#include "RedDSPSubtarget.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include <optional>
namespace llvm {
class RedDSPTargetMachine : public CodeGenTargetMachineImpl {
  RedDSPSubtarget Subtarget;
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
public:
  RedDSPTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
      StringRef FS, const TargetOptions &Options, std::optional<Reloc::Model> RM,
      std::optional<CodeModel::Model> CM, CodeGenOptLevel OL, bool JIT);
  const RedDSPSubtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  TargetLoweringObjectFile *getObjFileLowering() const override { return TLOF.get(); }
};
} // namespace llvm
#endif