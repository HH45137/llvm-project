#include "RedDSP.h"
#include "RedDSPTargetMachine.h"
#include "TargetInfo/RedDSPTargetInfo.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
class RedDSPAsmPrinter : public AsmPrinter {
public:
  static char ID;
  RedDSPAsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> S)
      : AsmPrinter(TM, std::move(S), ID) {}
  StringRef getPassName() const override { return "RED DSP Assembly Printer"; }
  void emitInstruction(const MachineInstr *MI) override;

private:
  void printOperand(const MachineOperand &MO, raw_ostream &OS);
};
} // namespace

void RedDSPAsmPrinter::printOperand(const MachineOperand &MO, raw_ostream &OS) {
  if (MO.isReg()) {
    OS << MF->getSubtarget<RedDSPSubtarget>().getRegisterInfo()->getName(
        MO.getReg());
  } else if (MO.isImm()) {
    OS << MO.getImm();
  } else if (MO.isMBB()) {
    MO.getMBB()->getSymbol()->print(OS, MAI);
  } else if (MO.isGlobal()) {
    getSymbol(MO.getGlobal())->print(OS, MAI);
  } else if (MO.isSymbol()) {
    OS << MO.getSymbolName();
  } else {
    llvm_unreachable("unsupported RED DSP assembly operand");
  }
}

void RedDSPAsmPrinter::emitInstruction(const MachineInstr *MI) {
  // Most meta instructions are normally consumed before AsmPrinter, but MIR
  // tests and reduced pipelines may still expose them here. They do not
  // represent RED DSP ISA operations and must not leak into optimizer input.
  if (MI->isMetaInstruction())
    return;

  std::string Text;
  raw_string_ostream OS(Text);
  unsigned Opc = MI->getOpcode();
  switch (Opc) {
#define PRINT(NAME, TEXT)                                                      \
  case RedDSP::NAME:                                                           \
    OS << TEXT;                                                                \
    break
    PRINT(ADDrr, "ADD.INT ");
    PRINT(SUBrr, "SUB.INT ");
    PRINT(MULrr, "MUL.INT ");
    PRINT(DIVrr, "DIV.INT ");
    PRINT(ANDrr, "AND ");
    PRINT(ORrr, "OR ");
    PRINT(XORrr, "XOR ");
    PRINT(SFLrr, "SFL ");
    PRINT(SFRrr, "SFR ");
    PRINT(ADDri, "ADD.INT.IMM ");
    PRINT(SUBri, "SUB.INT.IMM ");
    PRINT(MULri, "MUL.INT.IMM ");
    PRINT(DIVri, "DIV.INT.IMM ");
    PRINT(ANDri, "AND.IMM ");
    PRINT(ORri, "OR.IMM ");
    PRINT(XORri, "XOR.IMM ");
    PRINT(SFLri, "SFL.IMM ");
    PRINT(SFRri, "SFR.IMM ");
    PRINT(MACrr, "MAC.INT ");
    PRINT(MACri, "MAC.INT.IMM ");
    PRINT(CMPrr, "CMP ");
    PRINT(CMPri, "CMP.IMM ");
    PRINT(CBErr, "CBE ");
    PRINT(MOVrr, "ADD.INT ");
    PRINT(MOVi, "ADD.INT.IMM ");
    PRINT(LD, "LD ");
    PRINT(ST, "ST ");
    PRINT(BEQ, "BEQ ");
    PRINT(BNE, "BNE ");
    PRINT(BR, "BEQ ");
    PRINT(JMP, "JMP ");
    PRINT(CALL, "CALL ");
    PRINT(CALL_SYM, "CALL ");
  case RedDSP::NOP:
    OS << "NOP";
    break;
  case RedDSP::RET:
    OS << "RET R15 X X X";
    break;
  default:
    llvm_unreachable("unsupported RED DSP instruction");
#undef PRINT
  }
  if (Opc != RedDSP::RET && Opc != RedDSP::NOP) {
    const bool IsImm = Opc == RedDSP::ADDri || Opc == RedDSP::SUBri ||
                       Opc == RedDSP::MULri || Opc == RedDSP::DIVri ||
                       Opc == RedDSP::ANDri || Opc == RedDSP::ORri ||
                       Opc == RedDSP::XORri || Opc == RedDSP::SFLri ||
                       Opc == RedDSP::SFRri || Opc == RedDSP::CMPri;
    if (Opc == RedDSP::MOVrr) {
      printOperand(MI->getOperand(0), OS);
      OS << ' ';
      printOperand(MI->getOperand(1), OS);
      OS << " R0 X";
    } else if (Opc == RedDSP::MOVi) {
      printOperand(MI->getOperand(0), OS);
      OS << " R0 X ";
      printOperand(MI->getOperand(1), OS);
    } else if (IsImm) {
      printOperand(MI->getOperand(0), OS);
      OS << ' ';
      printOperand(MI->getOperand(1), OS);
      OS << " X ";
      printOperand(MI->getOperand(2), OS);
    } else if (Opc == RedDSP::MACri) {
      printOperand(MI->getOperand(0), OS);
      OS << ' ';
      printOperand(MI->getOperand(2), OS);
      OS << " X ";
      printOperand(MI->getOperand(3), OS);
    } else if (Opc == RedDSP::MACrr) {
      printOperand(MI->getOperand(0), OS);
      OS << ' ';
      printOperand(MI->getOperand(2), OS);
      OS << ' ';
      printOperand(MI->getOperand(3), OS);
      OS << " X";
    } else if (Opc == RedDSP::BEQ || Opc == RedDSP::BNE) {
      printOperand(MI->getOperand(0), OS);
      OS << ' ';
      printOperand(MI->getOperand(1), OS);
      OS << " X ";
      printOperand(MI->getOperand(2), OS);
    } else if (Opc == RedDSP::CBErr) {
      printOperand(MI->getOperand(0), OS);
      OS << ' ';
      printOperand(MI->getOperand(1), OS);
      OS << ' ';
      printOperand(MI->getOperand(2), OS);
      OS << " X";
    } else if (Opc == RedDSP::BR) {
      OS << "R0 R0 X ";
      printOperand(MI->getOperand(0), OS);
    } else if (Opc == RedDSP::JMP) {
      printOperand(MI->getOperand(0), OS);
      OS << " X X X";
    } else if (Opc == RedDSP::CALL || Opc == RedDSP::CALL_SYM) {
      OS << "R15 ";
      printOperand(MI->getOperand(0), OS);
      OS << " X X";
    } else if (Opc == RedDSP::LD || Opc == RedDSP::ST) {
      printOperand(MI->getOperand(0), OS);
      OS << ' ';
      printOperand(MI->getOperand(1), OS);
      OS << " X ";
      printOperand(MI->getOperand(2), OS);
    } else {
      for (unsigned I = 0; I < MI->getNumExplicitOperands(); ++I) {
        if (I)
          OS << ' ';
        printOperand(MI->getOperand(I), OS);
      }
      OS << " X";
    }
  }
  OutStreamer->emitRawText(OS.str());
}

char RedDSPAsmPrinter::ID = 0;
extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeRedDSPAsmPrinter() {
  RegisterAsmPrinter<RedDSPAsmPrinter> X(getTheRedDSPTarget());
}