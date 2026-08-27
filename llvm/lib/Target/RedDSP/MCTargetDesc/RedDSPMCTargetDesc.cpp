#include "RedDSPMCAsmInfo.h"
#include "RedDSPInstPrinter.h"
#define GET_REGINFO_ENUM
#include "RedDSPGenRegisterInfo.inc"
#include "TargetInfo/RedDSPTargetInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "RedDSPGenInstrInfo.inc"
#define GET_REGINFO_MC_DESC
#include "RedDSPGenRegisterInfo.inc"
#define GET_SUBTARGETINFO_MC_DESC
#include "RedDSPGenSubtargetInfo.inc"

static MCInstrInfo *createInstrInfo() { auto *X = new MCInstrInfo(); InitRedDSPMCInstrInfo(X); return X; }
static MCRegisterInfo *createRegInfo(const Triple &) { auto *X = new MCRegisterInfo(); InitRedDSPMCRegisterInfo(X, RedDSP::R15); return X; }
static MCSubtargetInfo *createSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createRedDSPMCSubtargetInfoImpl(TT, CPU.empty() ? "generic" : CPU,
                                         CPU.empty() ? "generic" : CPU, FS);
}
static MCAsmInfo *createAsmInfo(const MCRegisterInfo &, const Triple &,
                                const MCTargetOptions &Options) {
  return new RedDSPMCAsmInfo(Options);
}
static MCInstPrinter *createInstPrinter(const Triple &, unsigned SyntaxVariant,
                                        const MCAsmInfo &MAI,
                                        const MCInstrInfo &MII,
                                        const MCRegisterInfo &MRI) {
  return SyntaxVariant == 0 ? new RedDSPInstPrinter(MAI, MII, MRI) : nullptr;
}
extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRedDSPTargetMC() {
  Target &T = getTheRedDSPTarget();
  TargetRegistry::RegisterMCAsmInfo(T, createAsmInfo);
  TargetRegistry::RegisterMCInstrInfo(T, createInstrInfo);
  TargetRegistry::RegisterMCRegInfo(T, createRegInfo);
  TargetRegistry::RegisterMCSubtargetInfo(T, createSubtargetInfo);
  TargetRegistry::RegisterMCInstPrinter(T, createInstPrinter);
}