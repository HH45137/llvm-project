include "llvm/Target/Target.td"
include "RedDSPRegisterInfo.td"
include "RedDSPCallingConv.td"
include "RedDSPInstrInfo.td"

defm : RemapAllTargetPseudoPointerOperands<GR32>;

def : ProcessorModel<"generic", NoSchedModel, []>;
def RedDSPInstrInfo : InstrInfo;
def RedDSPAsmWriter : AsmWriter;

def RedDSP : Target {
  let InstructionSet = RedDSPInstrInfo;
  let AssemblyWriters = [RedDSPAsmWriter];
}