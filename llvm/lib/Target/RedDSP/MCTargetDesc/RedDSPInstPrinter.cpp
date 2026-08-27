#include "RedDSPInstPrinter.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
using namespace llvm;

void RedDSPInstPrinter::printInst(const MCInst *, uint64_t, StringRef Annot,
                                  const MCSubtargetInfo &, raw_ostream &OS) {
  llvm_unreachable("RED DSP MC instruction printing is not implemented");
}

void RedDSPInstPrinter::printRegName(raw_ostream &OS, MCRegister Reg) {
  OS << MRI.getName(Reg);
}