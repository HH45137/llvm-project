#ifndef LLVM_LIB_TARGET_REDDSP_MCTARGETDESC_REDDSPINSTPRINTER_H
#define LLVM_LIB_TARGET_REDDSP_MCTARGETDESC_REDDSPINSTPRINTER_H
#include "llvm/MC/MCInstPrinter.h"
namespace llvm {
class RedDSPInstPrinter : public MCInstPrinter {
public:
  RedDSPInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                    const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}
  void printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                 const MCSubtargetInfo &STI, raw_ostream &OS) override;
  void printRegName(raw_ostream &OS, MCRegister Reg) override;
  std::pair<const char *, uint64_t> getMnemonic(const MCInst &) const override {
    return {nullptr, 0};
  }
};
}
#endif